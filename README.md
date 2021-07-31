# TJpgDec
Tiny JPEG Decompressor

![Status](https://github.com/BigMumf/display-keyboard/actions/workflows/push_audit.yml/badge.svg)

This project is a mirror of the [TJpgDec project](http://elm-chan.org/fsw/tjpgd/00index.html).

The "upstream" branch is a clean copy of the source and sample files
from the official project page listed above with **no modifications**
whatsoever.

The default branch ("master") has the following changes:
1. Fuzzer test to detect memory access errors.
2. Changes to fix all detected memory access errors.
3. build (cmake) files.
4. GitHub workflows for continuous integration.

## Building

To build this library and sample programs:

```sh
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

## Fuzzing

To build and run the fuzzing test:

```sh
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DJPEG_FUZZING=1 ..
cmake --build .
./fuzzer
```

More info on fuzzing at [libFuzzer](https://llvm.org/docs/LibFuzzer.html).
