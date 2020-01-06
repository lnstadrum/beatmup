#include "../environment.h"
#include "../bitmap/converter.h"
#include "../bitmap/bitmap_access.h"
#include "../bitmap/mask_bitmap_access.h"
#include "../exception.h"
#include <algorithm>
#include <cstring>

using namespace Beatmup;


template<class in_t, class out_t> inline void convertBlock(AbstractBitmap& input, AbstractBitmap& output, int startx, int starty, msize nPix) {
	in_t in(input, startx, starty);
	out_t out(output, startx, starty);
	for (msize n = 0; n < nPix; ++n, in++, out++)
		out = in();
}


BitmapConverter::BitmapConverter() :
	input(NULL), output(NULL)
{}


void BitmapConverter::setBitmaps(AbstractBitmap* input, AbstractBitmap* output) {
	this->input = input;
	this->output = output;
}


ThreadIndex BitmapConverter::maxAllowedThreads() const {
	return AbstractTask::validThreadCount((int)(output->getSize().numPixels() / MIN_PIXEL_COUNT_PER_THREAD));
}


AbstractTask::ExecutionTarget BitmapConverter::getExecutionTarget() const {
	// it does not make sense to convert bitmaps on GPU
	return doNotUseGPU;
}


void BitmapConverter::beforeProcessing(ThreadIndex threadCount, GraphicPipeline* gpu) {
	NullTaskInput::check(input, "input bitmap");
	NullTaskInput::check(output, "output bitmap");
	RuntimeError::check(input->getSize() == output->getSize(),
		"Input and output bitmap must be of the same size.");
	input->lockPixels(ProcessingTarget::CPU);
	output->lockPixels(ProcessingTarget::CPU);
}


void BitmapConverter::afterProcessing(ThreadIndex threadCount, bool aborted) {
	input->unlockPixels();
	output->unlockPixels();
}


bool BitmapConverter::process(TaskThread& thread) {
	// if the bitmaps are equal, say done
	if (input == output)
		return true;

	// if pixel formats are identical, just copy
	if (input->getPixelFormat() == output->getPixelFormat()) {
		if (thread.currentThread() == 0)
			memcpy(
				output->getData(0,0),
				input->getData(0,0),
				input->getMemorySize()
			);
		return true;
	}

	// compute splitting accordingly to the current thread index
	int w = output->getWidth();
	msize
		npix = w * output->getHeight(),
		start = npix * thread.currentThread() / thread.totalThreads(),
		stop = npix * (1 + thread.currentThread()) / thread.totalThreads();

	// computing initial position
	int outx = (int)(start % w), outy = (int)(start / w);

	const int LOOK_AROUND_INTERVAL = 123456;
	while (start < stop && !thread.isTaskAborted()) {
		doConvert(outx, outy, stop-start);
		start += LOOK_AROUND_INTERVAL;
	}

	return true;
}


void BitmapConverter::doConvert(int outX, int outY, msize nPix) {
#define CALL_CONVERT_AND_RETURN(IN_T, OUT_T) \
	convertBlock < IN_T, OUT_T >(*input, *output, outX, outY, nPix); return;

	// for all (left) possible pairs of pixel formats, do the conversion
	switch (input->getPixelFormat()) {
	case SingleByte:
		switch (output->getPixelFormat()) {
			case TripleByte:	CALL_CONVERT_AND_RETURN(SingleByteBitmapReader, TripleByteBitmapWriter)
			case QuadByte:		CALL_CONVERT_AND_RETURN(SingleByteBitmapReader, QuadByteBitmapWriter)
			case SingleFloat:	CALL_CONVERT_AND_RETURN(SingleByteBitmapReader, SingleFloatBitmapWriter)
			case TripleFloat:	CALL_CONVERT_AND_RETURN(SingleByteBitmapReader, TripleFloatBitmapWriter)
			case QuadFloat:		CALL_CONVERT_AND_RETURN(SingleByteBitmapReader, QuadFloatBitmapWriter)
			default: throw ImplementationUnsupported("Cannot convert given formats");
		}

	case TripleByte:
		switch (output->getPixelFormat()) {
			case SingleByte:	CALL_CONVERT_AND_RETURN(TripleByteBitmapReader, SingleByteBitmapWriter)
			case QuadByte:		CALL_CONVERT_AND_RETURN(TripleByteBitmapReader, QuadByteBitmapWriter)
			case SingleFloat:	CALL_CONVERT_AND_RETURN(TripleByteBitmapReader, SingleFloatBitmapWriter)
			case TripleFloat:	CALL_CONVERT_AND_RETURN(TripleByteBitmapReader, TripleFloatBitmapWriter)
			case QuadFloat:		CALL_CONVERT_AND_RETURN(TripleByteBitmapReader, QuadFloatBitmapWriter)
			default: throw ImplementationUnsupported("Cannot convert given formats");
		}

	case QuadByte:
		switch (output->getPixelFormat()) {
			case SingleByte:	CALL_CONVERT_AND_RETURN(QuadByteBitmapReader, SingleByteBitmapWriter)
			case TripleByte:	CALL_CONVERT_AND_RETURN(QuadByteBitmapReader, TripleByteBitmapWriter)
			case SingleFloat:	CALL_CONVERT_AND_RETURN(QuadByteBitmapReader, SingleFloatBitmapWriter)
			case TripleFloat:	CALL_CONVERT_AND_RETURN(QuadByteBitmapReader, TripleFloatBitmapWriter)
			case QuadFloat:		CALL_CONVERT_AND_RETURN(QuadByteBitmapReader, QuadFloatBitmapWriter)
			default: throw ImplementationUnsupported("Cannot convert given formats");
		}

	case SingleFloat:
		switch (output->getPixelFormat()) {
			case SingleByte:	CALL_CONVERT_AND_RETURN(SingleFloatBitmapReader, SingleByteBitmapWriter)
			case TripleByte:	CALL_CONVERT_AND_RETURN(SingleFloatBitmapReader, TripleByteBitmapWriter)
			case QuadByte:		CALL_CONVERT_AND_RETURN(SingleFloatBitmapReader, QuadByteBitmapWriter)
			case TripleFloat:	CALL_CONVERT_AND_RETURN(SingleFloatBitmapReader, TripleFloatBitmapWriter)
			case QuadFloat:		CALL_CONVERT_AND_RETURN(SingleFloatBitmapReader, QuadFloatBitmapWriter)
			default: throw ImplementationUnsupported("Cannot convert given formats");
		}

	case TripleFloat:
		switch (output->getPixelFormat()) {
			case SingleByte:	CALL_CONVERT_AND_RETURN(TripleFloatBitmapReader, SingleByteBitmapWriter)
			case TripleByte:	CALL_CONVERT_AND_RETURN(TripleFloatBitmapReader, TripleByteBitmapWriter)
			case QuadByte:		CALL_CONVERT_AND_RETURN(TripleFloatBitmapReader, QuadByteBitmapWriter)
			case SingleFloat:	CALL_CONVERT_AND_RETURN(TripleFloatBitmapReader, SingleFloatBitmapWriter)
			case QuadFloat:		CALL_CONVERT_AND_RETURN(TripleFloatBitmapReader, QuadFloatBitmapWriter)
			default: throw ImplementationUnsupported("Cannot convert given formats");
		}

	case QuadFloat:
		switch (output->getPixelFormat()) {
			case SingleByte:	CALL_CONVERT_AND_RETURN(QuadFloatBitmapReader, SingleByteBitmapWriter)
			case TripleByte:	CALL_CONVERT_AND_RETURN(QuadFloatBitmapReader, TripleByteBitmapWriter)
			case QuadByte:		CALL_CONVERT_AND_RETURN(QuadFloatBitmapReader, QuadByteBitmapWriter)
			case SingleFloat:	CALL_CONVERT_AND_RETURN(QuadFloatBitmapReader, SingleFloatBitmapWriter)
			case TripleFloat:	CALL_CONVERT_AND_RETURN(QuadFloatBitmapReader, TripleFloatBitmapWriter)
			default: throw ImplementationUnsupported("Cannot convert given formats");
		}

	case BinaryMask:
		switch (output->getPixelFormat()) {
			case SingleByte:	CALL_CONVERT_AND_RETURN(BinaryMaskReader, SingleByteBitmapWriter)
			case TripleByte:	CALL_CONVERT_AND_RETURN(BinaryMaskReader, TripleByteBitmapWriter)
			case QuadByte:		CALL_CONVERT_AND_RETURN(BinaryMaskReader, QuadByteBitmapWriter)
			case SingleFloat:	CALL_CONVERT_AND_RETURN(BinaryMaskReader, SingleFloatBitmapWriter)
			case TripleFloat:	CALL_CONVERT_AND_RETURN(BinaryMaskReader, TripleFloatBitmapWriter)
			default: throw ImplementationUnsupported("Cannot convert given formats");
		}

	case QuaternaryMask:
		switch (output->getPixelFormat()) {
			case SingleByte:	CALL_CONVERT_AND_RETURN(QuaternaryMaskReader, SingleByteBitmapWriter)
			case TripleByte:	CALL_CONVERT_AND_RETURN(QuaternaryMaskReader, TripleByteBitmapWriter)
			case QuadByte:		CALL_CONVERT_AND_RETURN(QuaternaryMaskReader, QuadByteBitmapWriter)
			case SingleFloat:	CALL_CONVERT_AND_RETURN(QuaternaryMaskReader, SingleFloatBitmapWriter)
			case TripleFloat:	CALL_CONVERT_AND_RETURN(QuaternaryMaskReader, TripleFloatBitmapWriter)
			default: throw ImplementationUnsupported("Cannot convert given formats");
		}

	case HexMask:
		switch (output->getPixelFormat()) {
			case SingleByte:	CALL_CONVERT_AND_RETURN(HexMaskReader, SingleByteBitmapWriter)
			case TripleByte:	CALL_CONVERT_AND_RETURN(HexMaskReader, TripleByteBitmapWriter)
			case QuadByte:		CALL_CONVERT_AND_RETURN(HexMaskReader, QuadByteBitmapWriter)
			case SingleFloat:	CALL_CONVERT_AND_RETURN(HexMaskReader, SingleFloatBitmapWriter)
			case TripleFloat:	CALL_CONVERT_AND_RETURN(HexMaskReader, TripleFloatBitmapWriter)
			default: throw ImplementationUnsupported("Cannot convert given formats");
		}
	}
}


void BitmapConverter::convert(AbstractBitmap& input, AbstractBitmap& output) {
	BitmapConverter me;
	me.setBitmaps(&input, &output);

	if (input.getEnvironment().isManagingThread()) {
		RuntimeError::check(input.getSize() == output.getSize(),
			"Input and output bitmap must be of the same size.");

		// the bitmaps are assumed locked in this case
		me.doConvert(0, 0, input.getSize().numPixels());

	}
	else
		input.getEnvironment().performTask(me);
}
