#include "abstract_bitmap.h"
#include "internal_bitmap.h"
#include "bitmap_access.h"
#include "processing.h"
#include "crop.h"
#include "../debug.h"
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
            (bpp >= 8 || (outOrigin.x % ppb == 0 && rect.a.x % ppb == 0 && rect.b.x % ppb == 0));

        if (mayCopy) {
            // direct copying
            const msize lineSizeBytes = bpp >= 8 ? rect.width() * bpp / 8 : rect.width() / ppb;
            for (int y = rect.a.y; y < rect.b.y; ++y) {
                out.goTo(outOrigin.x, outOrigin.y + y - rect.a.y);
                in.goTo(rect.a.x, y);
                memcpy(*out, *in, lineSizeBytes);
            }
        }
        else
            // projecting
            for (int y = rect.a.y; y < rect.b.y; ++y) {
                out.goTo(outOrigin.x, outOrigin.y + y - rect.a.y);
                in.goTo(rect.a.x, y);
                for (int x = rect.a.x; x < rect.b.x; ++x, in++, out++)
                    out = in();
            }
    }
};


Crop::Crop() : outOrigin(0, 0), cropRect(0, 0, 0, 0)
{}


bool Crop::process(TaskThread& thread) {
    BitmapProcessing::pipeline<Cropping>(*input, *output, outOrigin.x, outOrigin.y, cropRect, outOrigin, input->getPixelFormat(), output->getPixelFormat());
    return true;
}


void Crop::beforeProcessing(ThreadIndex threadCount, GraphicPipeline* gpu) {
    NullTaskInput::check(input, "input bitmap");
    NullTaskInput::check(output, "output bitmap");
    cropRect.normalize();
    if (!isFit()) {
        BEATMUP_DEBUG_E("Crop rectangle does not fit to bitmaps: ((%d,%d),(%d,%d)) from %d x %d to put at (%d,%d) in %d x %d.",
                cropRect.a.x, cropRect.b.x, cropRect.a.y, cropRect.b.y,
                input->getWidth(), input->getHeight(),
                outOrigin.x, outOrigin.y,
                output->getWidth(), output->getHeight());
        throw RuntimeError("Crop rectangle does not fit to bitmaps");
    }
    input->lockContent(PixelFlow::CpuRead);
    output->lockContent(PixelFlow::CpuWrite);
}


void Crop::afterProcessing(ThreadIndex threadCount, GraphicPipeline* gpu, bool aborted) {
    input->unlockContent(PixelFlow::CpuRead);
    output->unlockContent(PixelFlow::CpuWrite);
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
    if (!input->getSize().clientRect().isInside(cropRect.a))
        return false;
    IntPoint corner = cropRect.b - cropRect.a - 1 + outOrigin;
    if (!output->getSize().clientRect().isInside(corner))
        return false;
    return true;
}


AbstractBitmap* Crop::run(AbstractBitmap& bitmap, IntRectangle clipRect) {
    AbstractBitmap* out = new InternalBitmap(bitmap.getContext(), bitmap.getPixelFormat(), clipRect.width(), clipRect.height());
    // after byte-aligning out must be zeroed (not very optimal...)
    if (out->getSize().numPixels() != clipRect.getArea())
        out->zero();
    Crop clip;
    clip.setInput(&bitmap);
    clip.setOutput(out);
    clip.setCropRect(clipRect);
    bitmap.getContext().performTask(clip);
    return out;
}
