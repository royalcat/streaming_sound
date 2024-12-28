import 'package:native_toolchain_c/native_toolchain_c.dart';
import 'package:logging/logging.dart';
import 'package:native_assets_cli/native_assets_cli.dart';

void main(List<String> args) async {
  await build(args, (config, output) async {
    final packageName = config.packageName;
    final cbuilder = CBuilder.library(
      name: packageName,
      assetName: 'src/${packageName}_bindings_generated.dart',
      // includes: [
      //   'src/milo',
      //   'src/miniaudio/extras/miniaudio_split',
      //   'src',
      // ],
      sources: [
        'src/milo.c',
        'src/player.c',
        'src/recorder.c',
        'src/miniaudio/extras/miniaudio_split/miniaudio.c',
      ],
    );
    await cbuilder.run(
      config: config,
      output: output,
      logger: Logger('')
        ..level = Level.ALL
        ..onRecord.listen((record) => print(record.message)),
    );
  });
}
