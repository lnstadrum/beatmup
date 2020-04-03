#include "abstract_bitmap.h"
#include "../context.h"
#include "../gpu/pipeline.h"
#include "internal_bitmap.h"
#include "converter.h"
#include "../gpu/bgl.h"
#include "../exception.h"
#include <cstring>


using namespace Beatmup;

const char* AbstractBitmap::PIXEL_FORMAT_NAMES[] = {
    "single byte", "triple byte", "quad byte",
    "single floating point", "triple floating point", "quad floating point",
    "binary mask", "quaternary mask", "hexadecimal mask"
};


const unsigned char AbstractBitmap::BITS_PER_PIXEL[] = {
     8,  8*3,  8*4,		// integer bitmaps
    32, 32*3, 32*4,		// floating point bitmaps
    1, 2, 4				// masks
};


const unsigned char AbstractBitmap::CHANNELS_PER_PIXEL[] = {
    1, 3, 4,			// integer bitmaps
    1, 3, 4,			// floating point bitmaps
    1, 1, 1				// masks
};


void AbstractBitmap::prepare(GraphicPipeline& gpu) {
    glBindTexture(GL_TEXTURE_2D, textureHandle);

    // check if there is data to transfer
    if (!upToDate[ProcessingTarget::CPU])
        return;

    // if the GPU version is up to date, return
    if (upToDate[ProcessingTarget::GPU])
        return;

    // setup alignment
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    if (isMask()) {
        // masks are stored as horizontally-stretched bitmaps
        int W = getWidth() / (8 / getBitsPerPixel());

#ifdef BEATMUP_OPENGLVERSION_GLES20
        glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, W, getHeight(), 0, GL_ALPHA, GL_UNSIGNED_BYTE, getData(0, 0));
#else
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_R8, W, getHeight());
        glTexSubImage2D(GL_TEXTURE_2D,
            0, 0, 0, W, getHeight(),
            GL_RED,
            GL_UNSIGNED_BYTE,
            getData(0, 0));
#endif
        GL::GLException::check("allocating texture image (mask)");
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }

    else {
#ifdef BEATMUP_OPENGLVERSION_GLES20
        glTexImage2D(GL_TEXTURE_2D,
                0,
                GL::BITMAP_INTERNALFORMATS[getPixelFormat()],
                getWidth(), getHeight(),
                0,
                GL::BITMAP_PIXELFORMATS[getPixelFormat()],
                GL::BITMAP_PIXELTYPES[getPixelFormat()],
                getData(0, 0));
#else
        glTexStorage2D(GL_TEXTURE_2D, 1, GL::BITMAP_INTERNALFORMATS[getPixelFormat()], getWidth(), getHeight());
        glTexSubImage2D(GL_TEXTURE_2D,
            0, 0, 0, getWidth(), getHeight(),
            GL::BITMAP_PIXELFORMATS[getPixelFormat()],
            GL::BITMAP_PIXELTYPES[getPixelFormat()],
            getData(0, 0));
#endif
        GL::GLException::check("allocating texture image");
    }

    upToDate[ProcessingTarget::GPU] = true;
}


const GL::TextureHandler::TextureFormat AbstractBitmap::getTextureFormat() const {
    if (isMask())
        return TextureFormat::Rx8;
    return (TextureFormat)getPixelFormat();
}


void AbstractBitmap::lockPixels(ProcessingTarget target) {
    class UnavailablePixelData : public Exception {
    public:
        UnavailablePixelData(const AbstractBitmap& bitmap):
            Exception("No pixel data available for bitmap %s", bitmap.toString().c_str())
        {}
    };

    if (target == ProcessingTarget::CPU) {
        // If pixel data in CPU memory is asked, GPU has it, cannot handle.
        if (!upToDate[ProcessingTarget::CPU]  && upToDate[ProcessingTarget::GPU]) {
            throw UnavailablePixelData(*this);
        }
    }

    // lock pixels only if working on CPU, or if GPU version is not up to date so that pixels will be queried from RAM
    if (target == ProcessingTarget::CPU || !upToDate[ProcessingTarget::GPU]) {
        if (!pixelDataLocked) {
            lockPixelData();
            pixelDataLocked = true;
        }
    }
}


void AbstractBitmap::unlockPixels() {
    if (pixelDataLocked) {
        unlockPixelData();
        pixelDataLocked = false;
    }
}


void AbstractBitmap::invalidate(ProcessingTarget target) {
    upToDate[target] = false;
    if (target == ProcessingTarget::GPU)
        GL::TextureHandler::invalidate(*ctx.getGpuRecycleBin());
}


bool AbstractBitmap::isUpToDate(ProcessingTarget target) const {
    return upToDate[target];
}


bool AbstractBitmap::isDirty() const {
    return !upToDate[ProcessingTarget::CPU] && !upToDate[ProcessingTarget::GPU];
}


int AbstractBitmap::getPixelInt(int x, int y, int cha) const {
    PixelFormat pf = getPixelFormat();
    if (isMask(pf)) {
        unsigned char
            pixPerByte = 8 / BITS_PER_PIXEL[pf],
            offset = (x + y*getWidth()) % pixPerByte;			// offset from byte-aligned bound in pixels
        const pixbyte* p = getData(x, y);
        return ((*p) >> (offset*BITS_PER_PIXEL[pf])) & ((1 << BITS_PER_PIXEL[pf]) - 1);
    }
    const pixbyte* p = getData(x, y) + cha * BITS_PER_PIXEL[pf] / 8 / CHANNELS_PER_PIXEL[pf];
    if (isInteger(pf))
        return *p;
    //if (isFloat(pf))
        return (int)(*((float*)p) * 255);
}


const unsigned char AbstractBitmap::getBitsPerPixel() const {
    return BITS_PER_PIXEL[getPixelFormat()];
}


const unsigned char AbstractBitmap::getNumberOfChannels() const {
    return CHANNELS_PER_PIXEL[getPixelFormat()];
}


bool AbstractBitmap::isInteger() const {
    return isInteger(getPixelFormat());
}


bool AbstractBitmap::isFloat() const {
    return isFloat(getPixelFormat());
}


bool AbstractBitmap::isMask() const {
    return isMask(getPixelFormat());
}


bool AbstractBitmap::isInteger(PixelFormat pixelFormat) {
    return pixelFormat == SingleByte|| pixelFormat == TripleByte || pixelFormat == QuadByte;
}


bool AbstractBitmap::isFloat(PixelFormat pixelFormat) {
    return pixelFormat == SingleFloat || pixelFormat == TripleFloat || pixelFormat == QuadFloat;
}


bool AbstractBitmap::isMask(PixelFormat pixelFormat) {
    return pixelFormat == BinaryMask || pixelFormat == QuaternaryMask || pixelFormat == HexMask;
}


std::string AbstractBitmap::toString() const {
    std::string desc = std::to_string(getWidth()) + "x" + std::to_string(getHeight()) + " " + PIXEL_FORMAT_NAMES[getPixelFormat()];
    if (isUpToDate(ProcessingTarget::CPU) && isUpToDate(ProcessingTarget::GPU))
        desc += " on CPU+GPU";
    else
        if (isUpToDate(ProcessingTarget::CPU))
            desc += " on CPU";
        else
            if (isUpToDate(ProcessingTarget::GPU))
                desc += " on GPU";
            else
                desc += " out of date";		// impossible
    return desc;
}


AbstractBitmap::AbstractBitmap(Context& ctx) : ctx(ctx) {
    upToDate[ProcessingTarget::CPU] = true;
    upToDate[ProcessingTarget::GPU] = false;
    pixelDataLocked = false;
}


AbstractBitmap::~AbstractBitmap() {
    if (hasValidHandle()) {
        TextureHandler::invalidate(*ctx.getGpuRecycleBin());
    }
}


const ImageResolution AbstractBitmap::getSize() const {
    return ImageResolution(getWidth(), getHeight());
}


Context& AbstractBitmap::getContext() const {
    return ctx;
}


void AbstractBitmap::zero() {
    WriteLock lock(*this);
    memset(getData(0, 0), 0, getMemorySize());
}


AbstractBitmap::ReadLock::ReadLock(AbstractBitmap& bitmap, ProcessingTarget unit):
    bitmap(bitmap)
{
    bitmap.lockPixels(unit);
}

AbstractBitmap::ReadLock::~ReadLock() {
    bitmap.unlockPixels();
}


AbstractBitmap::WriteLock::WriteLock(AbstractBitmap& bitmap):
    bitmap(bitmap)
{
    if (!bitmap.pixelDataLocked) {
        bitmap.lockPixelData();
        bitmap.pixelDataLocked = true;
    }
}

AbstractBitmap::WriteLock::~WriteLock() {
    bitmap.unlockPixels();
    bitmap.upToDate[ProcessingTarget::GPU] = false;
    bitmap.upToDate[ProcessingTarget::CPU] = true;
}
