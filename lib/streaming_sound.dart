import 'dart:async';
import 'dart:typed_data';

import 'src/stream_sound_ffi.dart';

enum SampleFormat {
  f32._(4),
  s32._(4),
  s24._(3),
  s16._(2),
  u8._(1);

  final int bytesPerFrame;

  const SampleFormat._(this.bytesPerFrame);
}

Future<void> playSoundStream<T extends TypedData>(
  Stream<Float32List> soundStream, {
  required int sampleRate,
  String? outputName,
}) =>
    StreamPlayer().play(
      sampleRate: sampleRate,
      soundStream: soundStream,
      outputName: outputName,
    );

Stream<T> recordSoundStream<T extends TypedData>({
  required int sampleRate,
  SampleFormat sampleFormat = SampleFormat.f32,
}) {
  assert(T == Uint8List || T == Int16List || T == Int32List || T == Float32List);
  switch (sampleFormat) {
    case SampleFormat.f32:
      assert(T == Float32List || T == Uint8List);
    case SampleFormat.s16:
      assert(T == Int16List || T == Uint8List);
    case SampleFormat.s24:
      assert(T == Int32List || T == Uint8List);
    case SampleFormat.s32:
      assert(T == Int32List || T == Uint8List);
    case SampleFormat.u8:
      assert(T == Uint8List);
  }

  Stream<Uint8List> out = StreamRecorder(
    sampleRate: sampleRate,
    sampleFormat: sampleFormat,
  ).record();

  if (T == Uint8List) {
    return out as Stream<T>;
  }
  if (T == Int16List) {
    return out.map<Int16List>((data) => Int16List.view(data.buffer)) as Stream<T>;
  }
  if (T == Int32List) {
    return out.map<Int32List>((data) => Int32List.view(data.buffer)) as Stream<T>;
  }
  if (T == Float32List) {
    return out.map<Float32List>((data) => Float32List.view(data.buffer)) as Stream<T>;
  }

  throw ArgumentError('Invalid type: $T');
}
