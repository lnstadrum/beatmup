#include "resampler.h"
#include "resampler_tools.h"
#include "interpolation.h"
#include "processing.h"

using namespace Beatmup;


BitmapResampler::BitmapResampler() :
	input(NULL), output(NULL)
{}


void BitmapResampler::setBitmaps(AbstractBitmap* input, AbstractBitmap* output) {
	this->input = input;
	this->output = output;
	if (input)
		srcRect = IntRectangle(0, 0, input->getWidth() - 1, input->getHeight() - 1);
	if (output)
		destRect = IntRectangle(0, 0, output->getWidth() - 1, output->getHeight() - 1);
}


void BitmapResampler::setInputRect(const IntRectangle& rect) {
	srcRect = rect;
}


void BitmapResampler::setOutputRect(const IntRectangle& rect) {
	destRect = rect;
}


ThreadIndex BitmapResampler::maxAllowedThreads() const {
	return AbstractTask::validThreadCount(std::min(destRect.height() + 1, srcRect.getArea() / MIN_PIXEL_COUNT_PER_THREAD));
}


void BitmapResampler::beforeProcessing(ThreadIndex threadCount, GraphicPipeline* gpu) {
	NullTaskInput::check(input, "input bitmap");
	NullTaskInput::check(output, "output bitmap");
	RuntimeError::check(input != output, "input and output is the same bitmap");
	srcRect.normalize();
	srcRect.limit(IntRectangle(0, 0, input->getWidth() - 1, input->getHeight() - 1));
	destRect.normalize();
	destRect.limit(IntRectangle(0, 0, output->getWidth() - 1, output->getHeight() - 1));
	input->lockPixels(ProcessingTarget::CPU);
	output->lockPixels(ProcessingTarget::CPU);
}


void BitmapResampler::afterProcessing(ThreadIndex threadCount, bool aborted) {
	input->unlockPixels();
	output->unlockPixels();
}


bool BitmapResampler::process(TaskThread& thread) {
	BitmapProcessing::pipeline<BitmapResamplingTools::CumulatingResampler>(
		*input, *output, 0, 0,
		srcRect, destRect, thread
	);
	return true;
}