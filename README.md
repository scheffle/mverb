﻿#mverb VST3 version

## How to build

### You need:

    * cmake
    * VST SDK 3.7.5
    * compiler with c++17 support

### Build:

```
mkdir build
cd build
cmake -Dvst3sdk_SOURCE_DIR=$PATH_TO_THE_VST3_SDK ../
cmake --build .
```

About mverb
=====

Studio quality, open-source reverb. Its release was intended to provide a practical demonstration of Dattorro’s figure-of-eight reverb structure and provide the open source community with a high quality reverb.

All code is self-contained within the mverb.h file and an example showing inclusion in a VST plugin is included.
