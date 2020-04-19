#!/bin/sh -l

cd /beatmup

# build a C++ app
mkdir build
cd build
cmake -DUSE_GLX=1 ../apps
make BasicRendering
cd ..

# build android lib
cd android/app
gradle clean assembleRelease
