#ifndef STREAMING_PLAYER_H
#define STREAMING_PLAYER_H

#include <stdint.h>

#include "export.h"

EXPORT typedef struct StreamPlayer StreamPlayer;

EXPORT StreamPlayer *stream_player_alloc(void);

EXPORT int stream_player_init(StreamPlayer *const self,
                              uint32_t const channel_count,
                              uint32_t const sample_rate,
                              uint32_t const period_size_in_frames,
                              const char *stream_name);

EXPORT void stream_player_uninit(StreamPlayer *const self);

EXPORT int stream_player_buffer_write(StreamPlayer *const self,
                                      const void *const data,
                                      const uint32_t framesToWrite);

EXPORT int stream_player_start(StreamPlayer *const self);

EXPORT int stream_player_stop(StreamPlayer *const self);

#endif
