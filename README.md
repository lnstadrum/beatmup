Beatmup is a parallel image processing and visualization framework.

* It makes easy the execution of image processing tasks in parallel.
  - Image operations are implemented as asynchronous tasks. The tasks are submitted to the engine by the user to perform operations over images in application-asynchronous fashion.
  - Images are processed by multiple workers in parallel on CPU for maximum speed.
  - The API allows for an easy synchronization between workers for multi-stage tasks.
* Beatmup uses hardware acceleration through **OpenGL** and **OpenGL ES** in a transparent platform-independent way.
  - Provides a realtime rendering toolset
  - Allows for user-defined shaders for rendering and image processing
* It enables support of 1, 2, 4, 8, 24 and 32-bit bitmaps.
* It implements a number of predefined tasks providing a basic image processing toolset.
  - Image resampling and pixel format conversion
    * _x2_, a neural network-based upscaling technique using GPU
  - Basic color filters and color space conversion
  - Flood fill and contours extraction
* Beatmup is tested in **linux**, **Windows** and **Android**. It works on **Raspberry Pi** and uses its GPU as well.
  - Android Java API is available
  - Few to no dependencies in linux and Windows; CMake is all you need
  
# Quick start

To build X2 app upscaling an image using a neural net 

    git clone https://github.com/lnstadrum/beatmup.git
    cd beatmup
    mkdir build
    cd build
    cmake ../apps
    make X2
    cd ..
    build/X2 <your-image>.bmp x2.bmp
