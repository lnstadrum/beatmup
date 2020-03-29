#include "tuning.h"
#include "../bitmap/bitmap_access.h"
#include "../bitmap/processing.h"
#include "../color/color_spaces.h"

using namespace Beatmup;


template<class in_t, class out_t> class ApplyTuning {
public:
    static inline void process(
        in_t in, out_t out,
        msize nPix, float hueOffset, float saturationFactor, float valueFactor, float contrast, float brightness
    ) {
        while (--nPix > 0) {
            // first apply brightness / contrast
            colorhsv hsv(in());

            // then HSV
            hsv.h += hueOffset;
            hsv.s *= saturationFactor;
            hsv.v *= valueFactor;

            out << ((pixfloat4)hsv) * contrast + brightness;
            in++;
            out++;
        }
    }
};


Filters::ImageTuning::ImageTuning() :
    hueOffset(0), saturationFactor(1), valueFactor(1),
    contrast(1), brightness(0)
{}


void Filters::ImageTuning::beforeProcessing(ThreadIndex threadCount, GraphicPipeline* gpu) {
    NullTaskInput::check(inputBitmap, "input bitmap");
    NullTaskInput::check(outputBitmap, "output bitmap");
    inputBitmap->lockPixels(ProcessingTarget::CPU);
    if (inputBitmap != outputBitmap)
        outputBitmap->lockPixels(ProcessingTarget::CPU);
}


void Filters::ImageTuning::afterProcessing(ThreadIndex threadCount, bool aborted) {
    inputBitmap->unlockPixels();
    if (inputBitmap != outputBitmap)
        outputBitmap->unlockPixels();
}


bool Filters::ImageTuning::process(TaskThread& thread) {
    // each thread receives a single part of bitmap to deal with
    int w = inputBitmap->getWidth();
    msize
        npix = w * inputBitmap->getHeight(),
        start = npix * thread.currentThread() / thread.totalThreads(),
        stop = npix * (1 + thread.currentThread()) / thread.totalThreads();
    // computing initial position
    int x = (int)(start % w), y = (int)(start / w);
    BitmapProcessing::pipeline<ApplyTuning>(
        *inputBitmap, *outputBitmap, x, y, stop-start,
        hueOffset, saturationFactor, valueFactor, contrast, brightness
    );
    return true;
}


void Filters::ImageTuning::setBitmaps(AbstractBitmap *input, AbstractBitmap *output) {
    inputBitmap = input;
    outputBitmap = output;
}



ThreadIndex Filters::ImageTuning::maxAllowedThreads() const {
    NullTaskInput::check(inputBitmap, "input bitmap");
    return AbstractTask::validThreadCount(inputBitmap->getWidth() * inputBitmap->getHeight() / MIN_PIXEL_COUNT_PER_THREAD);
}