#include "../include/recorder.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../miniaudio/extras/miniaudio_split/miniaudio.h"

#define MILO_LVL MILO_DEFAULT_LVL
#include "../milo/milo.h"

/*************
 ** private **
 *************/

struct StreamRecorder {
  ma_device device;
  ma_uint32 bytes_per_frame;
  ma_pcm_rb rb;
  ma_mutex mutex;

  DataAvalibleCallback data_avalible_callback;
};

static void data_callback(ma_device *pDevice, void *_, const void *pInput,
                          ma_uint32 frameCount) {
  (void)_;
  ma_result r;
  StreamRecorder *const self = pDevice->pUserData;

  trace("frame count: %d", frameCount);

  ma_uint32 framesToWrite = frameCount;
  void *pWriteBuffer;

  r = ma_pcm_rb_acquire_write(&self->rb, &framesToWrite, &pWriteBuffer);
  if (r != MA_SUCCESS) {
    error("Failed to acquire write buffer! Error code: %d", r);
    return;
  }

  trace("write buffer framesToWrite: %d", framesToWrite);

  memcpy(pWriteBuffer, pInput, framesToWrite * self->bytes_per_frame);

  r = ma_pcm_rb_commit_write(&self->rb, framesToWrite);
  if (r != MA_SUCCESS) {
    error("Failed to commit write buffer! Error code: %d", r);
    return;
  }

  trace("successfully written %d bytes", framesToWrite * self->bytes_per_frame);

  self->data_avalible_callback();

  // void *pReadBuffer = malloc(frameCount * self->bytes_per_frame);
  // memcpy(pReadBuffer, pInput, frameCount * self->bytes_per_frame);

  // self->dart_data_callback(pInput, frameCount * self->bytes_per_frame);
}

/************
 ** public **
 ************/

int stream_recorder_read_buffer(StreamRecorder *const self, void *const data,
                                uint32_t framesToRead) {
  // ma_uint32 availableReadFrames = ma_pcm_rb_available_read(&self->rb);
  // if (availableReadFrames < framesToRead) {
  //   warn("buffer underflow; availableReadFrames: %d need: %d",
  //        availableReadFrames, framesToRead);
  //   return RangeErr;
  // }

  if (ma_pcm_rb_available_read(&self->rb) < framesToRead) {
    return 0;
  }

  ma_mutex_lock(&self->mutex);

  ma_uint32 avalibleFrames = framesToRead;
  void *pReadBuffer;
  ma_result r;
  r = ma_pcm_rb_acquire_read(&self->rb, &avalibleFrames, &pReadBuffer);
  if (r != MA_SUCCESS) {
    ma_mutex_unlock(&self->mutex);
    return error("Failed to acquire read buffer! Error code: %d", r), -1;
  }

  if (avalibleFrames == 0) {
    ma_mutex_unlock(&self->mutex);
    return warn("No data to read!"), 0;
  }

  trace("read buffer framesToRead: %d", avalibleFrames);

  memcpy(data, pReadBuffer, avalibleFrames * self->bytes_per_frame);

  r = ma_pcm_rb_commit_read(&self->rb, avalibleFrames);
  if (r != MA_SUCCESS && r != MA_AT_END) {
    ma_mutex_unlock(&self->mutex);
    return error("Failed to commit read buffer! Error code: %d", r), -1;
  }

  ma_mutex_unlock(&self->mutex);
  return avalibleFrames;
}

StreamRecorder *stream_recorder_alloc() {
  return malloc(sizeof(StreamRecorder));
}

int stream_recorder_init(StreamRecorder *const self,
                         uint32_t const channel_count,
                         uint32_t const sample_rate,
                         uint32_t const period_size_frames,
                         DataAvalibleCallback const data_avalible_callback) {
  ma_result r;

  self->data_avalible_callback = data_avalible_callback;

  r = ma_mutex_init(&self->mutex);
  if (r != MA_SUCCESS) {
    return error("Failed to initialize mutex! Error code: %d", r), r;
  }

  r = ma_pcm_rb_init(ma_format_f32, channel_count, period_size_frames * 32,
                     NULL, NULL, &self->rb);
  if (r != MA_SUCCESS) {
    return error("Failed to initialize ring buffer! Error code: %d", r), -1;
  }

  ma_device_config device_config =
      ma_device_config_init(ma_device_type_capture);
  device_config.capture.format = ma_format_f32;
  device_config.capture.channels = channel_count;
  device_config.periodSizeInFrames = period_size_frames;
  device_config.sampleRate = sample_rate;
  device_config.dataCallback = data_callback;
  device_config.pUserData = self;

  r = ma_device_init(NULL, &device_config, &self->device);
  if (r != MA_SUCCESS)
    return error("miniaudio device initialization error! Error code: %d", r),
           -1;

  self->bytes_per_frame = ma_get_bytes_per_frame(self->device.capture.format,
                                                 self->device.capture.channels);

  trace("recorder bytesPerFrame: %d", self->bytes_per_frame);

  return info("recorder initialized"), 0;
}

void stream_recorder_uninit(StreamRecorder *const self) {
  ma_device_uninit(&self->device);
}

int stream_recorder_start(StreamRecorder *const self) {
  ma_result r = ma_device_start(&self->device);
  if (r != MA_SUCCESS) {
    return error("Failed to start device! Error code: %d", r), -1;
  }
  return info("recorder started"), 0;
}

int stream_recorder_stop(StreamRecorder *const self) {
  ma_result r = ma_device_stop(&self->device);
  if (r != MA_SUCCESS) {
    return error("Failed to stop device! Error code: %d", r), -1;
  }
  return 0;
}
