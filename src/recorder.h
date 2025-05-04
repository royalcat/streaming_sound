#ifndef STREAMING_RECORDER_H
#define STREAMING_RECORDER_H

#include <stdint.h>

#include "export.h"

EXPORT typedef struct StreamRecorder StreamRecorder;
EXPORT typedef void (*DataAvalibleCallback)();

EXPORT StreamRecorder *stream_recorder_alloc(void);

EXPORT int stream_recorder_init(StreamRecorder *const self,
                         uint32_t const channel_count,
                         uint32_t const sample_rate,
                         uint32_t const period_size_in_frames,
                         DataAvalibleCallback const data_avalible_callback);

EXPORT int stream_recorder_read_buffer(StreamRecorder *const self,
                                       void *const data, uint32_t framesToRead);

EXPORT void stream_recorder_uninit(StreamRecorder *const self);

EXPORT int stream_recorder_start(StreamRecorder *const self);

EXPORT int stream_recorder_stop(StreamRecorder *const self);

#endif