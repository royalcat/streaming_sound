part of 'stream_sound_ffi.dart';

class StreamPlayer {
  StreamPlayer();

  Future<void> play({
    required Stream<Float32List> soundStream,
    required int sampleRate,
    String? outputName,
  }) async {
    final self = _bindings.stream_player_alloc();

    final r = _bindings.stream_player_init(
      self,
      1,
      sampleRate,
      outputName?.toNativeUtf8().cast() ?? nullptr.cast(),
    );
    if (r != 0) {
      throw PlatformException("Failed to init the player (code: $r).");
    }

    const needFirstFrames = 960 * 4;

    final firstFrameBufferCompleter = Completer<void>();

    var framesWritten = 0;

    var bufSize = 960;
    var buf = malloc.allocate<Void>(bufSize);

    soundStream.listen(
      (event) {
        final eventSize = event.lengthInBytes;
        if (eventSize > bufSize) {
          malloc.free(buf);
          buf = malloc.allocate<Void>(eventSize);
          bufSize = eventSize;
        }

        buf.cast<Float>().asTypedList(event.length).setAll(0, event);
        final res = _bindings.stream_player_buffer_write(self, buf, eventSize);
        if (res != 0) {
          throw PlatformException("Failed write to player buffer (code: $res).");
        }

        framesWritten += event.length;
        if (!firstFrameBufferCompleter.isCompleted && framesWritten >= needFirstFrames) {
          firstFrameBufferCompleter.complete();
        }
      },
      onDone: () {
        malloc.free(buf);
        final res = _bindings.stream_player_stop(self);
        if (res != 0) {
          throw PlatformException("Failed to stop the player (code: $res).");
        }
        _bindings.stream_player_uninit(self);
      },
    );

    await firstFrameBufferCompleter.future;

    final res = _bindings.stream_player_start(self);
    if (res != 0) {
      throw PlatformException("Failed to start the player (code: $res).");
    }

    return;
  }
}
