import 'dart:io';
import 'dart:typed_data';

import 'package:flutter/material.dart';
import 'package:permission_handler/permission_handler.dart';
import 'dart:async';

import 'package:streaming_sound/streaming_sound.dart' as streaming_sound;

void main() {
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  Stream<Float32List>? rec;
  static const int sampleRate = 44100;

  void startListening() async {
    if (Platform.isAndroid) {
      await Permission.microphone.request();
    }

    setState(() {
      rec = streaming_sound.recordSoundStream(sampleRate: sampleRate);
    });

    streaming_sound.playSoundStream(rec!, sampleRate: sampleRate);
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('Native Packages'),
        ),
        body: Center(
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: <Widget>[
              ElevatedButton(
                onPressed: startListening,
                child: const Text('Start Listening'),
              ),
            ],
          ),
        ),
      ),
    );
  }
}
