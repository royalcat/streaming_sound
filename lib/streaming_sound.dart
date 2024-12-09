import 'dart:typed_data';

import 'src/stream_sound_ffi.dart';

Future<void> playSoundStream(
  Stream<Float32List> soundStream, {
  required int sampleRate,
  String? outputName,
}) =>
    StreamPlayer().play(
      sampleRate: sampleRate,
      soundStream: soundStream,
      outputName: outputName,
    );

Stream<Float32List> recordSoundStream({
  required int sampleRate,
}) =>
    StreamRecorder().record(
      sampleRate: sampleRate,
    );
