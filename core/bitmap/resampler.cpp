#include "resampler.h"
#include "resampler_tools.h"
#include "processing.h"
#include "resampler_cnn_x2/cnn.h"

using namespace Beatmup;


BitmapResampler::BitmapResampler() :
    input(nullptr), output(nullptr), mode(Mode::NEAREST_NEIGHBOR), convnet(nullptr)
{}


BitmapResampler::~BitmapResampler() {
    if (convnet)
        delete convnet;
}


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


AbstractTask::ExecutionTarget BitmapResampler::getExecutionTarget() const {
    return mode == Mode::CONVNET ? ExecutionTarget::useGPU : ExecutionTarget::doNotUseGPU;
}


void BitmapResampler::beforeProcessing(ThreadIndex threadCount, GraphicPipeline* gpu) {
    NullTaskInput::check(input, "input bitmap");
    NullTaskInput::check(output, "output bitmap");
    RuntimeError::check(input != output, "input and output is the same bitmap");
    srcRect.normalize();
    srcRect.limit(IntRectangle(0, 0, input->getWidth(), input->getHeight()));
    destRect.normalize();
    destRect.limit(IntRectangle(0, 0, output->getWidth(), output->getHeight()));

    if (mode == Mode::CONVNET) {
        RuntimeError::check(gpu != nullptr, "convnet resampling requires GPU");
        RuntimeError::check(
            destRect.width() == 2 * srcRect.width() && destRect.height() == 2 * srcRect.height(),
            "convnet resampling is only applicable for 2x upsampling"
        );
        if (!convnet) {
            convnet = new X2UpsamplingNetwork(*input->getContext().getGpuRecycleBin(), *gpu);
        }
    }

    ProcessingTarget target = gpu ?  ProcessingTarget::GPU : ProcessingTarget::CPU;
    input->lockPixels(target);
    output->lockPixels(target);
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

        case Mode::LINEAR:
            BitmapProcessing::pipeline<BitmapResamplingTools::BilinearResampling>(
                *input, *output, 0, 0,
                srcRect, destRect, thread
            );
            break;

        case Mode::CONVNET:
            break;

        default:
            Insanity::insanity("Resampling mode not implemented");
    }
    return true;
}


bool BitmapResampler::processOnGPU(GraphicPipeline& gpu, TaskThread& thread) {
    switch (mode) {
        case Mode::CONVNET:
            convnet->process(gpu, *input, *output);
            break;

        default:
            Insanity::insanity("Resampling mode not implemented");
    }
    return true;
}
