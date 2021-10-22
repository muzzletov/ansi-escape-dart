import 'dart:async';
import 'dart:ffi';
import 'dart:ffi' as ffi;
import 'dart:isolate';

import 'package:example/models/terminal.dart';
import 'package:ffi/ffi.dart';

typedef _read_bytes_func = ffi.Pointer<Uint8> Function();
typedef _kill_func = ffi.Pointer<Uint8> Function();
typedef _start_func = ffi.Pointer<Int32> Function(Pointer<Pointer<Utf8>>);

typedef _ReadBytes = ffi.Pointer<Uint8> Function();
typedef _Kill = ffi.Pointer<Uint8> Function();
typedef _Start = ffi.Pointer<Int32> Function(Pointer<Pointer<Utf8>>);

ffi.DynamicLibrary _lib = ffi.DynamicLibrary.open('librun.dylib');

final _Kill _kill = _lib
    .lookup<ffi.NativeFunction<_kill_func>>('kill_child') // very barbaric!
    .asFunction();

final _ReadBytes _readBytes = _lib
    .lookup<ffi.NativeFunction<_read_bytes_func>>('read_bytes')
    .asFunction();
final _Start _start = _lib
    .lookup<ffi.NativeFunction<_start_func>>('start')
    .asFunction<_start_func>();


Future<int> init(List<String> args) async {
  final List<Pointer<Utf8>> _strPtrList = args.map((str) =>str.toNativeUtf8()).toList();
  final Pointer<Pointer<Utf8>> _pointerPointerList = calloc(_strPtrList.length+1);

  for (int i = 0; i < _strPtrList.length; i++) {
    _pointerPointerList[i] = _strPtrList[i];
  }

  _pointerPointerList[_strPtrList.length] = nullptr;

  final int state = _start(_pointerPointerList).address;

  calloc.free(_pointerPointerList);
  _strPtrList.forEach(calloc.free);

  return state;
}

Future<void> readBytes(SendPort sendPort) async {
  Pointer<Uint8> data = _readBytes();

  while (ffi.nullptr.address != data.address) {
    int m = data.elementAt(0).value-1;

    if(m > 0) {
      data = Pointer<Uint8>.fromAddress(data.address+1);
      await sendPort..send(data.asTypedList(m));
    }

    data = _readBytes();
  }

}

class ProcessListener {
  TerminalContainer term = TerminalContainer();
  late Function callback;
  late List<String> args = [];
  final StreamController _controller = StreamController();
  late Stream processStream;

  ProcessListener({required this.args, required this.callback}) {
    processStream = _controller.stream;
  }

  Future<bool> kill() async {
    return _kill().value == 0;
  }

  start() async {
    ReceivePort receiverPort = ReceivePort();
    receiverPort.listen((_data) async {
      callback(await Future.delayed(const Duration(milliseconds: 10), ()=>term.renderList(_data)));
    });
    await Future.delayed(const Duration(milliseconds: 10), () async =>await Isolate.spawn(init, args));
    await Isolate.spawn(readBytes, receiverPort.sendPort);
  }
}
