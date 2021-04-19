![android](https://github.com/lnstadrum/beatmup/workflows/android/badge.svg) ![linux](https://github.com/lnstadrum/beatmup/workflows/linux/badge.svg) ![windows](https://github.com/lnstadrum/beatmup/workflows/windows/badge.svg) ![Python packages](https://github.com/lnstadrum/beatmup/workflows/Build%20Python%20packages/badge.svg)


# Overview

Beatmup is an extensible asynchronous image and signal processing framework.

### Beatmup is for image processing, mainly.
  - It enables low-level user-defined processing tasks execution
    - in parallel and using hardware acceleration for the maximum speed,
    - asynchronously, in an application-friendly fashion.

  - It implements basic building blocks and predefined processing operations:
    - Image resampling and geometric operations
    - Basic color filtering
    - Flood fill and contour extraction
    - Realtime rendering

### Beatmup runs quite everywhere.
  - Used in **linux**, **Windows** and **Android**
  - Written in **C++11**, it has **Java** and **Python** bindings
  - **Prebuilt Python packages** are available for x64 desktop Windows and Ubuntu-based linux distributions
  - Few to no dependencies: Beatmup is extremely easy to build for a different platform
    - git, CMake and a C++ compiler is all you need
    - Beatmup compiles with gcc, clang and msvc

### Beatmup speeds up processing by using GPU regardless its vendor, price and release year.
  - It runs on
    - desktop GPUs from mainstream vendors,
    - integrated desktop GPUs,
    - Android GPUs,
    - Raspberry Pi GPU (all models including Pi Zero W),
    - NVIDIA Jetson Nano,
    - ...
  - It uses the GPU for
    - *x2*, neural network-based image upscaling technique,
    - running inference of small user-defined neural nets,
    - applying user-defined GLSL shaders to process images,
    - realtime camera image processing in Android,
    - scene rendering and predefined image processing tasks.


# Highlights

## *x2*: a neural net-based image upscaler

Beatmup is arguably the easiest way to get a fast hardware-accelerated neural network-based image superresolution running on any decent GPU. It does not provide a state-of-the-art PSNR score, but can run at a hundred of images per second on a desktop machine.

More details on the neural network design and the inference implementation: [Inferring a super-resolution neural network on Raspberry Pi GPU](https://medium.com/analytics-vidhya/inferring-a-super-resolution-neural-network-on-raspberry-pi-gpu-89b5456d21ef)

Update in 2.1 release: the fine-tuned version of the model (trained with multiple degradation kernels) achieves now 32.64 dB on DIV2K validation set compared to the initial score of 32.57 dB.

## PictureJam Collage Maker

An early version of Beatmup is used in [PictureJam Collage Maker Android app](https://play.google.com/store/apps/details?id=xyz.pichancer.picturejam.full).


# Available resources

## Docs
 * C++ documentation is available [here](https://lnstadrum.github.io/beatmup/cpp/html). This is the most extensive documentation, it also contains additional explanations that do not relate to a specific programming language:
   - [Programming model](https://lnstadrum.github.io/beatmup/cpp/html/ProgrammingModel.html)
   - [NNets module overview](https://lnstadrum.github.io/beatmup/cpp/html/NNetsModuleOverview.html)
 * Java package API documentation is [here](https://lnstadrum.github.io/beatmup/java/html).
 * Python documentation is [here](https://lnstadrum.github.io/beatmup/python/python.html).

## C++
There is a number of test apps showcasing the use of Beatmup with additional explanations in [apps](apps) folder. The code is in C++, but can be universally helpful. Few examples:
 * [apps/shaderer](apps/shaderer/app.cpp) processes images with a custom GLSL fragment shader read from the standard input.
 * [apps/x2](apps/x2/app.cpp) is a test app for the *x2* neural net.
 * [apps/classify](apps/classify/app.cpp) is a dog image classifier, a variant of ResNeXt trained on a subset of ImageNet containing 120 classes of dogs and cats images. The inference is implemented with OpenGL shaders. Top-1 validation accuracy achieved on Raspberry Pi Zero W is 72.08%. Classifying a 385*385 image takes ~595 ms there.

## Python
[python/examples](python/examples) folder contains detailed examples of scripts using Beatmup in Python.
 * [cifar100.py](python/examples/cifar100.py) explains how to train a neural net with TensorFlow/Keras, convert it into a Beatmup-compliant model and run its inference on CIFAR100 test set.
 * [x2_superresolution.py](python/examples/x2_superresolution.py) and [x2_superresolution_video.py](python/examples/x2_superresolution_video.py) showcase applying x2 upsampling neural net to an image and a video.

Unit tests in Python are available in [python/tests/test.py](python/tests/test.py) and [python/tests/test_nnets.py](python/tests/test_nnets.py).

## Java (Android)
An Android test app is available in [android/app](android/app) folder. It consists of independent examples showing how to use Beatmup in Android.
 * [BasicCameraUse.java](android/app/src/main/java/xyz/beatmup/androidapp/samples/BasicCameraUse.java) shows how to get the camera preview, apply a simple shader and display the result.
 * [UpsamplingConvnetOnCamera.java](android/app/src/main/java/xyz/beatmup/androidapp/samples/UpsamplingConvnetOnCamera.java) implements the application of x2 upsampling neural net to the camera preview.
 * [DogClassifier.java](android/app/src/main/java/xyz/beatmup/androidapp/samples/DogClassifier.java) implements the dog classifier taking the camera preview image as input.



# Quick start: compiling Beatmup

To get things running, make sure you have *git* and *CMake* installed, then pick one of the following copy-paste recipes.

## Linux

Building **X2** app upscaling an image using a neural net inferred with OpenGL:

    git clone https://github.com/lnstadrum/beatmup.git
    cd beatmup
    git submodule update --init --recursive
    mkdir -p build && cd build
    cmake -DUSE_GLX=ON ..
    make X2

 * Try to use `-DUSE_OPENGL=ON` instead `-DUSE_GLX=ON` starting from a clean build folder if you run into trouble.
 * On **Raspberry Pi prior to series 4** use the EGL backend with following CMake command:

    `cmake -DUSE_EGL=ON -DUSE_BRCM_LIBS=ON -DGLES_VERSION=20 ..`

More details on Raspberry Pi setup [here](https://github.com/lnstadrum/beatmup/wiki/Compiling-Beatmup-on-Raspberry-Pi-prior-to-4).

 * On **Raspberry Pi 4** rather use the following CMake command:

    `cmake -DUSE_EGL_DRM=ON -DGLES_VERSION=20 ..`

More details on Raspberry Pi 4 setup [here](https://github.com/lnstadrum/beatmup/wiki/Compiling-Beatmup-on-Raspberry-Pi-4).

Once the app is built, you can feed it with an image of your choice and get the upscaled result as follows:

    ./X2 <your-image>.bmp x2-result.bmp

## Windows

To build the **X2** app in Windows you may install *Visual Studio* and generate the VS solution as follows:

    git clone https://github.com/lnstadrum/beatmup.git
    cd beatmup
    git submodule update --init --recursive
    mkdir build && cd build
    cmake -DUSE_OPENGL=ON ..

Then open the *.sln* solution file in Visual Studio and build the executable.

## Android

There is Android project containing a library and a test app modules (*lib* and *app*). To build them install Android Studio and Android NDK bundle, clone the repository and open the project situated in *android* folder in Android studio.

You can build an apk in a docker container as well. Having docker installed, you can run the following script to build *beatmupApp.apk* installation package which you may then copy to your Android phone and open it there to install the test app:

    git clone https://github.com/lnstadrum/beatmup.git
    cd beatmup
    git submodule update --init --recursive
    docker build -f dockerfiles/android.Dockerfile -t beatmup-android .
    docker run -v $(pwd):/opt/android-build beatmup-android


# Quicker start with Python

## Windows

Prebuilt python packages are available in 64-bit Windows for Python **3.5**, **3.6**, **3.7**, **3.8** and **3.9**:

    python -m pip install --upgrade pip
    python -m pip install beatmup

## Ubuntu-based linux

Prebuilt packages for x64 desktop Ubuntu-based linux distributions are available for downloading on the [releases page](https://github.com/lnstadrum/beatmup/releases/).

## Compiling Python package

If there is no prebuilt package available for your platform/OS, you can easily build a Python wheel installable with `pip` on your own.

First, get the code and try to compile an app as explained above. Then in the repository root folder run

    cd build
    make beatmup
    cd ../python
    python3 setup.py bdist_wheel clean

The installation package is now available in `python/dist` folder. You can install it and make sure it works:

    python3 -m pip install --no-index --find-links=dist beatmup
    python3 -c "import beatmup; beatmup.say_hi()"


# Licence and contributions

The project is licensed under the GNU GPL v3 licence.

The contributions are governed by the [Contribution Licence Agreement (CLA)](CLA.md). In short, under this Agreement you remain the copyright owner of your contribution, but you allow us to use and distribute your contribution under the terms of a possibly different licence.

If you want to contribute:
 * make sure you own the copyright of your contribution,
 * raise a pull request,
 * send a signed CLA on beatmup.imaging@gmail.com.

Otherwise:
 * raise an issue if you have a question or a feature request,
 * contact us by email.