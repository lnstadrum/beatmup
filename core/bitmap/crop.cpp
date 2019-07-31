#include "abstract_bitmap.h"
#include "internal_bitmap.h"
#include "bitmap_access.h"
#include "processing.h"
#include "crop.h"
#include <cstring>

using namespace Beatmup;


template<class in_t, class out_t> class Cropping {
public:
	static inline void process(in_t in, out_t out, const IntRectangle rect, const IntPoint outOrigin, const PixelFormat inputFormat, const PixelFormat outputFormat) {
		const unsigned char
			bpp = AbstractBitmap::BITS_PER_PIXEL[inputFormat],
			ppb = 8 / bpp;		// pixels per byte

		// test if output origin and clip rect horizontal borders are byte-aligned and the pixel formats are identical
		const bool mayCopy = (inputFormat == outputFormat) &&
			(bpp >= 8 || (outOrigin.x % ppb == 0 && rect.A.x % ppb == 0 && rect.B.x % ppb == 0));

		if (mayCopy) {
			// direct copying
			const msize lineSizeBytes = bpp >= 8 ? rect.width() * bpp / 8 : rect.width() / ppb;
			for (int y = rect.A.y; y < rect.B.y; ++y) {
				out.goTo(outOrigin.x, outOrigin.y + y - rect.A.y);
				in.goTo(rect.A.x, y);
				memcpy(*out, *in, lineSizeBytes);
			}
		}
		else
			// projecting
			for (int y = rect.A.y; y < rect.B.y; ++y) {
				out.goTo(outOrigin.x, outOrigin.y + y - rect.A.y);
				in.goTo(rect.A.x, y);
				for (int x = rect.A.x; x < rect.B.x; ++x, in++, out++)
					out = in();
			}
	}
};


Crop::Crop() : cropRect(0, 0, 0, 0), outOrigin(0, 0)
{}


bool Crop::process(TaskThread& thread) {
	BitmapProcessing::pipeline<Cropping>(*input, *output, outOrigin.x, outOrigin.y, cropRect, outOrigin, input->getPixelFormat(), output->getPixelFormat());
	return true;
}


void Crop::beforeProcessing(ThreadIndex threadCount, GraphicPipeline* gpu) {
	NullTaskInput::check(input, "input bitmap");
	NullTaskInput::check(output, "output bitmap");
	cropRect.normalize();
	if (!isFit())
		BEATMUP_ERROR("Crop rectangle does not fit bitmaps: ((%d,%d),(%d,%d)) from %d x %d to put at (%d,%d) in %d x %d.",
			cropRect.A.x, cropRect.B.x, cropRect.A.y, cropRect.B.y,
			input->getWidth(), input->getHeight(),
			outOrigin.x, outOrigin.y,
			output->getWidth(), output->getHeight()
		);
	input->lockPixels(ProcessingTarget::CPU);
	output->lockPixels(ProcessingTarget::CPU);
}


void Crop::afterProcessing(ThreadIndex threadCount, bool aborted) {
	input->unlockPixels();
	output->unlockPixels();
}


void Crop::setInput(AbstractBitmap* input) {
	this->input = input;
}


void Crop::setOutput(AbstractBitmap* output) {
	this->output = output;
}


void Crop::setCropRect(IntRectangle rect) {
	cropRect = rect;
}


void Crop::setOutputOrigin(IntPoint pos) {
	outOrigin = pos;
}


bool Crop::isFit() const {
	if (!input || !output)
		return false;
	if (!input->getSize().clientRect().isInside(cropRect.A))
		return false;
	IntPoint corner = cropRect.B - cropRect.A - 1 + outOrigin;
	if (!output->getSize().clientRect().isInside(corner))
		return false;
	return true;
}


AbstractBitmap* Crop::run(AbstractBitmap& bitmap, IntRectangle clipRect) {
	AbstractBitmap* out = new InternalBitmap(bitmap.getEnvironment(), bitmap.getPixelFormat(), clipRect.width(), clipRect.height());
	// after byte-aligning out must be zeroed (not very optimal...)
	if (out->getSize().numPixels() != clipRect.getArea())
		out->zero();
	Crop clip;
	clip.setInput(&bitmap);
	clip.setOutput(out);
	clip.setCropRect(clipRect);
	bitmap.getEnvironment().performTask(clip);
	return out;
}
