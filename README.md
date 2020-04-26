# Intro

Beatmup is an extensible parallel image and signal processing and visualization framework.

* It makes easy the execution of image processing tasks in parallel.
  - Image operations are implemented as asynchronous tasks. The tasks are submitted to the execution engine to perform operations with images in application-asynchronous fashion.
  - Images are processed by multiple workers in parallel on CPU for maximum speed.
  - The API allows for an easy synchronization between workers for multi-stage tasks.
* Beatmup uses hardware acceleration through **OpenGL** and **OpenGL ES** in a transparent platform-independent way.
  - Provides a realtime rendering toolset
  - Allows for user-defined shaders for rendering and image processing
* Beatmup natively supports 1, 2, 4, 8, 24 and 32-bit integer bitmaps and 32, 96 and 128-bit floating point bitmaps.
  - Enables 24-bit RGB bitmaps support in Android
  - Provides pixel arithmetics allowing to implement processing operations in a format-independent way
* It implements a number of predefined tasks providing a basic image processing toolset.
  - Hardware-accelerated layered rendering toolkit
  - Basic color filters and color space conversion
  - Image resampling and pixel format conversion
    * _x2_, a neural network-based upscaling technique using GPU on Raspberry Pi, Android and desktop environments
  - Flood fill and contours extraction
* Beatmup is tested in **linux**, **Windows** and **Android**. It works on **Raspberry Pi** and uses its GPU as well.
  - Android Java API is available.
    * Provides access to the camera. Enables realtime camera preview processing with custom shaders
  - Few to no dependencies; git and CMake is all you need
* Beatmup evolves as much as it can!
  - Basic OpenSL ES and AAudio support to play audio (Android)
  - Efficient audio signals graph plotting
  - Video decoding support (Android)
  - Neural networks inference using OpenGL compute shaders (in progress)
  
# Quick start

There is a bunch of test apps showcasing the use of Beatmup. To get things running, make sure you have *Git* and *CMake* installed, then open your favourite terminal and pick one of the copy-paste recipes that follow according to your OS.

## Linux

To build **X2** app upscaling an image using a neural net inferred with OpenGL you may proceed as follows:

    git clone https://github.com/lnstadrum/beatmup.git
    git submodule update
    cd beatmup
    mkdir build
    cd build
    cmake -DUSE_OPENGL=1 ../apps
    make X2

Use *-DUSE_GLX=1* instead *-DUSE_OPENGL=1* if there are issues. On **Raspberry Pi** replace it with *-DUSE_EGL=1*.

You can then feed the app with an image of your choice and get the upscaled result as follows:

    ./X2 <your-image>.bmp x2-result.bmp

## Windows

To build the **X2** app in Windows you may install *Visual Studio* (tested on 2013 Express and 2019 Community free editions) and generate the VS solution as follows:

    git clone https://github.com/lnstadrum/beatmup.git
    git submodule update
    cd beatmup
    mkdir build
    cd build
    cmake -DUSE_OPENGL=1 ../apps

Then open the *.sln* solution file in Visual Studio and build the executable.

## Android

There is Android project containing a library and a test app modules (*lib* and *app*). To build them install Android Studio and Android NDK bundle, clone the repository and open the project situated in *android* folder in Android studio.

If for some reason you do not want to bother you with Android studio but you still want to try out the test app, you can build an apk in a docker container. If you have docker installed, the following script will build *beatmupApp.apk* installation package which you may then copy to your Android phone and open to install the app:

    git clone https://github.com/lnstadrum/beatmup.git
    git submodule update
    cd beatmup
    cp .github/workflows/make-apps/* .
    docker build . -t android-build
    docker run -v $(pwd):/android-build android-build

