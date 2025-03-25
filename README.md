# ansi-escape-dart
This is a Process class implementation next to the official one with the addition that it also provides the ANSI sequences produced by the child process. Can be incoporated into a flutter app. Includes sample dart ffi file.

generating the dynamic library:

```
gcc -c run.c -o run.o
gcc -shared -o run.dylib run.o
```
