#include "resampler.h"
#include "resampler_tools.h"
#include "processing.h"

using namespace Beatmup;


BitmapResampler::BitmapResampler() :
    input(nullptr), output(nullptr), mode(Mode::NEAREST_NEIGHBOR)
{}


void BitmapResampler::setBitmaps(AbstractBitmap* input, AbstractBitmap* output) {
    this->input = input;
    this->output = output;
    if (input)
        srcRect = IntRectangle(0, 0, input->getWidth(), input->getHeight());
    if (output)
        destRect = IntRectangle(0, 0, output->getWidth(), output->getHeight());
}


void BitmapResampler::setMode(Mode mode) {
    this->mode = mode;
}


void BitmapResampler::setInputRect(const IntRectangle& rect) {
    srcRect = rect;
}


void BitmapResampler::setOutputRect(const IntRectangle& rect) {
    destRect = rect;
}


ThreadIndex BitmapResampler::maxAllowedThreads() const {
    static const int MIN_PIXELS_PER_THREAD = 1000; //!< minimum number of pixels per worker
    return AbstractTask::validThreadCount(std::min(destRect.height() + 1, srcRect.getArea() / MIN_PIXELS_PER_THREAD));
}


void BitmapResampler::beforeProcessing(ThreadIndex threadCount, GraphicPipeline* gpu) {
    NullTaskInput::check(input, "input bitmap");
    NullTaskInput::check(output, "output bitmap");
    RuntimeError::check(input != output, "input and output is the same bitmap");
    srcRect.normalize();
    srcRect.limit(IntRectangle(0, 0, input->getWidth(), input->getHeight()));
    destRect.normalize();
    destRect.limit(IntRectangle(0, 0, output->getWidth(), output->getHeight()));
    input->lockPixels(ProcessingTarget::CPU);
    output->lockPixels(ProcessingTarget::CPU);
}


void BitmapResampler::afterProcessing(ThreadIndex threadCount, bool aborted) {
    input->unlockPixels();
    output->unlockPixels();
}


bool BitmapResampler::process(TaskThread& thread) {
    switch (mode) {
        case Mode::NEAREST_NEIGHBOR:
            BitmapProcessing::pipeline<BitmapResamplingTools::NearestNeigborResampling>(
                *input, *output, 0, 0,
                srcRect, destRect, thread
            );
            break;

        case Mode::BOX:
            BitmapProcessing::pipeline<BitmapResamplingTools::BoxResampling>(
                *input, *output, 0, 0,
                srcRect, destRect, thread
            );
            break;
    }
    return true;
}