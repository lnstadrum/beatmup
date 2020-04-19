#include "resampler.h"
#include "resampler_tools.h"
#include "processing.h"
#include "resampler_cnn_x2/gles20/cnn.h"
#ifndef BEATMUP_OPENGLVERSION_GLES20
#include "resampler_cnn_x2/gles31/cnn.h"
#endif

using namespace Beatmup;

const float BitmapResampler::DEFAULT_CUBIC_PARAMETER = -0.5f;


BitmapResampler::BitmapResampler() :
    input(nullptr), output(nullptr), mode(Mode::CUBIC), cubicParameter(DEFAULT_CUBIC_PARAMETER), convnet(nullptr),
    isUsingEs31IfAvailable(false)
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


void BitmapResampler::setCubicParameter(float alpha) {
    cubicParameter = alpha;
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
    RuntimeError::check(
        srcRect == input->getSize().halfOpenedRectangle() && destRect == output->getSize().halfOpenedRectangle(),
        "input and output rectangular areas not matching the corresponding bitmaps are not supported when using GPU"
    );
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

#ifndef BEATMUP_OPENGLVERSION_GLES20
        // check if the ES backend needed to be upgraded to 3.1
        if (convnet && isUsingEs31IfAvailable && !convnet->usesEs31Backend()) {
            delete convnet;
            convnet = nullptr;
        }
#endif

        // init convnet instance if not yet
#ifndef BEATMUP_OPENGLVERSION_GLES20
        if (!convnet && isUsingEs31IfAvailable)
            convnet = new GLES31X2UpsamplingNetwork(input->getContext(), *gpu);
#endif
        if (!convnet)
            convnet = new GLES20X2UpsamplingNetwork(*input->getContext().getGpuRecycleBin(), *gpu);
    }

    input ->lockContent(gpu, false);
    output->lockContent(gpu, true);
}


void BitmapResampler::afterProcessing(ThreadIndex threadCount, GraphicPipeline* gpu, bool aborted) {
    input ->unlockContent(gpu, false);
    output->unlockContent(gpu, true);
}


bool BitmapResampler::process(TaskThread& thread) {
    switch (mode) {
        case Mode::NEAREST_NEIGHBOR:
            BitmapProcessing::pipeline<BitmapResamplingTools::NearestNeigborResampling>(
                *input, *output,
                srcRect, destRect, thread
            );
            break;

        case Mode::BOX:
            BitmapProcessing::pipeline<BitmapResamplingTools::BoxResampling>(
                *input, *output,
                srcRect, destRect, thread
            );
            break;

        case Mode::LINEAR:
            BitmapProcessing::pipeline<BitmapResamplingTools::BilinearResampling>(
                *input, *output,
                srcRect, destRect, thread
            );
            break;

        case Mode::CUBIC:
            BitmapProcessing::pipeline<BitmapResamplingTools::BicubicResampling>(
                *input, *output,
                srcRect, destRect, cubicParameter, thread
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
