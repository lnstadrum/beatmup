#include "../../filters/local/pixelwise_filter.h"
#include "../../exception.h"

using namespace Beatmup;


Filters::PixelwiseFilter::PixelwiseFilter() :
    inputBitmap(NULL), outputBitmap(NULL)
{}


void Filters::PixelwiseFilter::setBitmaps(AbstractBitmap *input, AbstractBitmap *output) {
    this->inputBitmap = input;
    this->outputBitmap = output;
}


ThreadIndex Filters::PixelwiseFilter::maxAllowedThreads() const {
    NullTaskInput::check(inputBitmap, "input bitmap");
    return AbstractTask::validThreadCount(inputBitmap->getWidth() * inputBitmap->getHeight() / MIN_PIXEL_COUNT_PER_THREAD);
}


void Filters::PixelwiseFilter::beforeProcessing(ThreadIndex threadCount, GraphicPipeline* gpu) {
    NullTaskInput::check(inputBitmap, "input bitmap");
    NullTaskInput::check(outputBitmap, "output bitmap");
    RuntimeError::check(
        inputBitmap->getWidth() == outputBitmap->getWidth() &&
        inputBitmap->getHeight() <= outputBitmap->getHeight(),
        "Incompatible input and output bitmaps sizes");
    inputBitmap->lockPixels(gpu ? ProcessingTarget::GPU : ProcessingTarget::CPU);
    if (inputBitmap != outputBitmap)
        outputBitmap->lockPixels(gpu ? ProcessingTarget::GPU : ProcessingTarget::CPU);
}


void Filters::PixelwiseFilter::afterProcessing(ThreadIndex threadCount, bool aborted) {
    inputBitmap->unlockPixels();
    if (inputBitmap != outputBitmap)
        outputBitmap->unlockPixels();
}


bool Filters::PixelwiseFilter::process(TaskThread& thread) {
    // each thread receives a single part of bitmap to deal with
    int w = inputBitmap->getWidth();
    msize
        npix = w * inputBitmap->getHeight(),
        start = npix * thread.currentThread() / thread.totalThreads(),
        stop  = npix * (1 + thread.currentThread()) / thread.totalThreads();
    // computing initial position
    int x = (int)(start % w), y = (int)(start / w);
    apply(x, y, stop - start, thread);
    return true;
}