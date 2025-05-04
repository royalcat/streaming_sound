part of 'stream_sound_ffi.dart';

typedef _DataCallback = Void Function();

class StreamRecorder {
  final int sampleRate;
  final SampleFormat sampleFormat;

  const StreamRecorder({required this.sampleRate, required this.sampleFormat});

  Stream<Uint8List> record() {
    final self = _bindings.stream_recorder_alloc();

    final frameBuffer = framesPerBuffer(frameDuration, sampleRate);
    final buffer = malloc.allocate<Uint8>(frameBuffer * sampleFormat.bytesPerFrame);

    final out = StreamController<Uint8List>();

    out.onListen = () {
      final dataAvalibleCallback = NativeCallable<_DataCallback>.listener(() {
        final n = _bindings.stream_recorder_read_buffer(self, buffer.cast<Void>(), frameBuffer);
        if (n == -1) {
          throw PlatformException("Failed to read buffer from the recorder (code: $n).");
        }

        if (n == 0) {
          return;
        }

        out.add(Uint8List.fromList(buffer.asTypedList(n * sampleFormat.bytesPerFrame)));
      });

      var r = _bindings.stream_recorder_init(
          self, 1, sampleRate, frameBuffer, dataAvalibleCallback.nativeFunction);
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
