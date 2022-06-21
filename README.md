# MVerb VST3 version

This is a port of the MVerb plug-in to VST3. 
On the contrary to the original this port also supports processing with double precision.

## How to build

### You need:

    * cmake
    * VST SDK 3.7.5
    * compiler with c++17 support

### Build:

```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=RELEASE -Dvst3sdk_SOURCE_DIR=$PATH_TO_THE_VST3_SDK ../
cmake --build .
```

### Preset Installation

Copy the included vstpresets in the presets subfolder into the following folder. Create missing folders if necessary:

* macOS : ```/Library/Audio/Presets/Martin Eastwood/MVerb/```
* Windows: ```C:\ProgramData\VST3 Presets\Martin Eastwood\MVerb\```
* Linux : ```/usr/local/share/vst3/presets/Martin Eastwood/MVerb/```

About MVerb
=====

Studio quality, open-source reverb. Its release was intended to provide a practical demonstration of Dattorro’s figure-of-eight reverb structure and provide the open source community with a high quality reverb.

All code is self-contained within the mverb.h file and an example showing inclusion in a VST plugin is included.
