#include "internal_bitmap.h"
#include "../exception.h"
#include "../utils/bmp_file.h"
#include "../gpu/swapper.h"

using namespace Beatmup;


InternalBitmap::InternalBitmap(Context& ctx, PixelFormat pixelFormat, int width, int height, bool allocate) :
    AbstractBitmap(ctx),
    pixelFormat(pixelFormat), width(width), height(height),
    memory(0),
    data(nullptr)
{
    if (getBitsPerPixel() < 8) {
        int n = 8 / getBitsPerPixel();
        this->width = ceili(width, n) * n;
    }
    if (allocate)
        memory = ctx.allocateMemory(getMemorySize());
    upToDate[ProcessingTarget::CPU] = allocate;
}


InternalBitmap::InternalBitmap(Context& ctx, const char* filename) :
    AbstractBitmap(ctx),
    data(nullptr)
{
    // read header
    BmpFile bmp(filename);
    switch (bmp.getBitsPerPixel()) {
        case 1:
            this->pixelFormat = PixelFormat::BinaryMask;
            break;
        case 4:
            this->pixelFormat = PixelFormat::HexMask;
            break;
        case 8:
            this->pixelFormat = PixelFormat::SingleByte;
            break;
        case 24:
            this->pixelFormat = PixelFormat::TripleByte;
            break;
        case 32:
            this->pixelFormat = PixelFormat::QuadByte;
            break;
        default:
            throw IOError(filename, "Unsupported pixel format");
    }
    this->width = bmp.getWidth();
    this->height = bmp.getHeight();

    // allocate & read
    memory = ctx.allocateMemory(getMemorySize());
    Context::Mem mem(ctx, memory);
    bmp.load(mem(), getMemorySize());
}


InternalBitmap::~InternalBitmap() {
    if (memory)
        ctx.freeMemory(memory);
}


void Beatmup::InternalBitmap::reshape(int width, int height) {
    if (this->width * this->height != width * height && memory) {
        ctx.freeMemory(memory);
        this->width = width;
        this->height = height;
        memory = ctx.allocateMemory(getMemorySize());
    }
    else {
        this->width = width;
        this->height = height;
    }

    if (upToDate[ProcessingTarget::GPU])
        TextureHandler::invalidate(*ctx.getGpuRecycleBin());

    upToDate[ProcessingTarget::CPU] = false;
    upToDate[ProcessingTarget::GPU] = false;
}


const PixelFormat InternalBitmap::getPixelFormat() const {
    return pixelFormat;
}


const int InternalBitmap::getWidth() const {
    return width;
}


const int InternalBitmap::getHeight() const {
    return height;
}


const msize InternalBitmap::getMemorySize() const {
    // a proper way to compute required memory size (works for bpp < 8)
    return ceili(height * width * AbstractBitmap::BITS_PER_PIXEL[pixelFormat], 8);
}


pixbyte* InternalBitmap::getData(int x, int y) const {
    return data + (y * width + x) * AbstractBitmap::BITS_PER_PIXEL[pixelFormat] / 8;
}


void InternalBitmap::lockPixelData() {
    if (!memory)
        memory = ctx.allocateMemory(getMemorySize());
    if (!data)
        data = (pixbyte*)ctx.acquireMemory(memory);
}


void InternalBitmap::unlockPixelData() {
    if (data) {
        data = nullptr;
        ctx.releaseMemory(memory, !isUpToDate(ProcessingTarget::CPU));
    }
}


void InternalBitmap::saveBmp(const char* filename) {
    if (!isUpToDate(ProcessingTarget::CPU)) {
        // Grab output bitmap from GPU memory to RAM
        Beatmup::Swapper::pullPixels(*this);
    }

    if (!memory)
        memory = ctx.allocateMemory(getMemorySize());
    Context::Mem mem(ctx, memory);
    BmpFile::save(
        mem(),
        width, height,
        getBitsPerPixel(),
        filename
    );
}
