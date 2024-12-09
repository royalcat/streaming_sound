import 'dart:async';
import 'dart:ffi';
import 'dart:io';
import 'dart:math';
import 'dart:typed_data';

import 'package:ffi/ffi.dart';
import 'package:streaming_sound/src/streaming_sound_bindings_generated.dart';

part 'stream_player.dart';
part 'stream_recorder.dart';

const String _libName = "streaming_sound";
final _bindings = StreamingSoundBindings(() {
  if (Platform.isMacOS || Platform.isIOS) {
    return DynamicLibrary.open("$_libName.framework/$_libName");
  } else if (Platform.isAndroid || Platform.isLinux) {
    return DynamicLibrary.open("lib$_libName.so");
  } else if (Platform.isWindows) {
    return DynamicLibrary.open("$_libName.dll");
  }
  throw UnsupportedError("Unsupported platform: ${Platform.operatingSystem}");
}());

extension PointerCopy on Pointer {
  void copy(TypedData typedData) {
    final data = typedData.buffer.asUint8List();
    cast<Uint8>().asTypedList(data.length).setAll(0, data);
  }
}

class PlatformException implements Exception {
  final String message;

  PlatformException(this.message);

  @override
  String toString() => message;
}
