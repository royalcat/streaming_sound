part of 'stream_sound_ffi.dart';

typedef _DataCallback = Void Function();

class StreamRecorder {
  StreamRecorder();

  Stream<Float32List> record({required int sampleRate}) {
    final self = _bindings.stream_recorder_alloc();

    const frameDuration = Duration(milliseconds: 20);
    const bytesPerFrame = 4;
    final framesPerBuffer = frameDuration.inMilliseconds * sampleRate ~/ 1000;

    final buffer = malloc.allocate<Float>(framesPerBuffer * bytesPerFrame);

    final out = StreamController<Float32List>();

    out.onListen = () {
      final dataAvalibleCallback = NativeCallable<_DataCallback>.listener(() {
        final n = _bindings.stream_recorder_read_buffer(self, buffer.cast<Void>(), framesPerBuffer);
        if (n == -1) {
          throw PlatformException("Failed to read buffer from the recorder (code: $n).");
        }

        if (n == 0) {
          return;
        }

        out.add(Float32List.fromList(buffer.asTypedList(n)));
      });

      var r = _bindings.stream_recorder_init(
          self, 1, sampleRate, framesPerBuffer, dataAvalibleCallback.nativeFunction);
      if (r != 0) {
        throw PlatformException("Failed to init the recorder (code: $r).");
      }

      r = _bindings.stream_recorder_start(self);
      if (r != 0) {
        throw PlatformException("Failed to start the recorder (code: $r).");
      }

      print("Recording started.");
    };

    out.onCancel = () {
      out.close();
      _bindings.stream_recorder_stop(self);
      _bindings.stream_recorder_uninit(self);
      malloc.free(buffer);
    };

    return out.stream;
  }
}
