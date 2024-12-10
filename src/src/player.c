#include "../include/player.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../miniaudio/extras/miniaudio_split/miniaudio.h"

#define MILO_LVL MILO_DEFAULT_LVL
#include "../milo/milo.h"

/*************
 ** private **
 *************/

struct StreamPlayer {
  ma_device device;
  ma_pcm_rb rb;
  ma_pthread_mutex_t mutex;
  ma_uint32 bytes_per_frame;
};

static void data_callback(ma_device *pDevice, void *pOutput, const void *pInput,
                          ma_uint32 frameCount) {

  (void)pInput;
  StreamPlayer *const self = pDevice->pUserData;

  trace("callback self: %p", self);

  ma_uint32 framesToRead = frameCount;

  ma_uint32 availableReadFrames = ma_pcm_rb_available_read(&self->rb);
  if (availableReadFrames < framesToRead) {
    warn("buffer underflow; availableReadFrames: %d need: %d; filling with 0",
         availableReadFrames, framesToRead);
    memset(pOutput, 0, framesToRead * self->bytes_per_frame);
  }

  trace("framesToRead: %d", framesToRead);

  void *pReadBuffer;
  ma_result r;
  r = ma_pcm_rb_acquire_read(&self->rb, &framesToRead, &pReadBuffer);
  if (r != MA_SUCCESS) {
    return;
  }

  trace("read buffer framesToRead: %d", framesToRead);

  memcpy(pOutput, pReadBuffer, framesToRead * self->bytes_per_frame);

  r = ma_pcm_rb_commit_read(&self->rb, framesToRead);
  if (r != MA_SUCCESS) {
    return;
  }

  trace("successfully read %d bytes", framesToRead * self->bytes_per_frame);
}

/************
 ** public **
 ************/

StreamPlayer *stream_player_alloc() { return malloc(sizeof(StreamPlayer)); }

int stream_player_init(StreamPlayer *const self, uint32_t const channel_count,
                       uint32_t const sample_rate, const char *stream_name) {
  ma_mutex_init(&self->mutex);

  trace("init self: %p", self);

  ma_result r;
  r = ma_pcm_rb_init(ma_format_f32, channel_count, 8192, NULL, NULL, &self->rb);
  if (r != MA_SUCCESS) {
    return error("Failed to initialize ring buffer! Error code: %d", r), -1;
  }

  ma_device_config device_config =
      ma_device_config_init(ma_device_type_playback);
  device_config.playback.format = ma_format_f32;
  device_config.playback.channels = channel_count;
  device_config.noClip = MA_TRUE;
  device_config.performanceProfile = ma_performance_profile_low_latency;
  device_config.periodSizeInMilliseconds = 20;
  device_config.sampleRate = sample_rate;
  device_config.dataCallback = data_callback;
  device_config.pUserData = self;

#ifdef MA_ENABLE_PULSEAUDIO
  device_config.pulse.pStreamNamePlayback = stream_name;
#endif

  r = ma_device_init(NULL, &device_config, &self->device);
  if (r != MA_SUCCESS)
    return error("miniaudio device initialization error! Error code: %d", r),
           -1;

  if (self->device.playback.format != ma_format_f32) {
    return error(
               "miniaudio failed to initialize device with supported format!"),
           -1;
  }

  self->bytes_per_frame = ma_get_bytes_per_frame(
      self->device.playback.format, self->device.playback.channels);

  trace("player bytesPerFrame: %d", self->bytes_per_frame);

  return info("player initialized"), 0;
}

void stream_player_uninit(StreamPlayer *const self) {
  ma_device_uninit(&self->device);
}

int stream_player_buffer_write(StreamPlayer *const self, const void *const data,
                               const uint32_t dataSize) {
  trace("write self: %p", self);

  trace("write bytes_per_frame: %d", self->bytes_per_frame);

  ma_uint32 framesToWrite = dataSize / self->bytes_per_frame;

  if (self->device.playback.format != ma_format_f32) {
    return error("Unsupported format!"), -1;
  }
  if (self->device.playback.channels != 1) {
    return error("Unsupported channel count!"), -1;
  }

  trace("write framesToWrite: %d", framesToWrite);
  trace("write dataSize: %d", dataSize);

  if (framesToWrite == 0) {
    return warn("No data to write!"), 0;
  }

  ma_mutex_lock(&self->mutex);

  void *pWriteBuffer;
  ma_result r;
  r = ma_pcm_rb_acquire_write(&self->rb, &framesToWrite, &pWriteBuffer);
  if (r != MA_SUCCESS) {
    ma_mutex_unlock(&self->mutex);
    return error("Failed to acquire write buffer! Error code: %d", r), -1;
  }

  trace("write buffer framesToWrite: %d", framesToWrite);

  memcpy(pWriteBuffer, data, framesToWrite * self->bytes_per_frame);

  r = ma_pcm_rb_commit_write(&self->rb, framesToWrite);
  if (r != MA_SUCCESS) {
    ma_mutex_unlock(&self->mutex);
    return error("Failed to commit write buffer! Error code: %d", r), -1;
  }

  ma_mutex_unlock(&self->mutex);
  return trace("successfully written %d bytes",
               framesToWrite * self->bytes_per_frame),
         0;
}

int stream_player_start(StreamPlayer *const self) {
  trace("start self: %p", self);

  ma_result r = ma_device_start(&self->device);
  if (r != MA_SUCCESS) {
    return error("Failed to start device! Error code: %d", r), -1;
  }
  return 0;
}

int stream_player_stop(StreamPlayer *const self) {
  ma_result r = ma_device_stop(&self->device);
  if (r != MA_SUCCESS) {
    return error("Failed to stop device! Error code: %d", r), -1;
  }
  return 0;
}