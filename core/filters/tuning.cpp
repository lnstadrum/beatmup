#include "tuning.h"
#include "../bitmap/bitmap_access.h"
#include "../bitmap/processing.h"
#include "../color/color_spaces.h"

using namespace Beatmup;


template<class in_t, class out_t> class ApplyTuning {
public:
    static inline void process(
        AbstractBitmap& input, AbstractBitmap &output, int x, int y,
        msize nPix, float hueOffset, float saturationFactor, float valueFactor, float contrast, float brightness
    ) {
        in_t in(input, x, y);
        out_t out(output, x, y);
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
    outputBitmap->lockContent(nullptr, true);
    if (inputBitmap != outputBitmap)
        inputBitmap->lockContent(nullptr, false);
}


void Filters::ImageTuning::afterProcessing(ThreadIndex threadCount, GraphicPipeline* gpu, bool aborted) {
  outputBitmap->unlockContent(nullptr, true);
  if (inputBitmap != outputBitmap)
      inputBitmap->unlockContent(nullptr, false);
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
