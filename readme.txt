# Real-time reassignment JUCE app

## Dependencies

* [JUCE 7.x](https://github.com/juce-framework/juce/tree/develop)
* FFTW (libfftwf)
* LIBLTFAT (libltfatf)

## Installing

This project uses CMake, with a config based off the [Pamplejuce](https://github.com/sudara/pamplejuce) template.

### JUCE

The JUCE dependency is setup as a git submodule that tracks JUCE's develop branch. You can populate via:

```
git submodule update --init
```

### FFTW

Unfortunately this project does not handle building the FFTW or LIBLTFAT dependencies for you.

The CMakeLists.txt hard codes the location of the dependencies — you will likely need to change these locations t owhere your .a or .lib files are built.

For example the FFTW library is specified as `/usr/local/lib/libfftw3f.a`.

If you are compiling FFTW for the first time, please note you should build from their [downloads](https://www.fftw.org/download.html) and not their github, which for some reason is setup to be contributor-only.

Also: please ensure that you have the `--enable-float` flag set.

Also also: if you are on MacOS, you can't just use homebrew as it won't get you the headers you need. You have to compile from scratch ala:

```
./configure --enable-float  && make && make install
```

### LIBLTFAT

LIBLTFAT is specified as ``/libltfat/build/libltfatf.a` under the current directory.

Again, this location is arbitrary, you'll have to build that project yourself and then update the CMakeLists.txt to point to the correct location of the library.
