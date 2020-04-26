#!/bin/sh -l

cd /beatmup


# build a C++ app
if [ -d "/android-build" ]; then
    echo "Android build output folder given: skipping a C++ app build"
else
    mkdir build
    cd build
    cmake -DUSE_GLX=1 ../apps
    make BasicRendering
    
    if [ $? -ne 0 ]; then
      exit 1
    fi

    cd ..
fi


# build android app
cd android/app
gradle clean assembleDebug

if [ -d "/android-build" ]; then
    mv -v build/outputs/apk/debug/app-debug.apk /android-build/beatmupApp.apk
fi
