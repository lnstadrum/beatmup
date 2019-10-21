#include "internal_bitmap.h"
#include "../exception.h"
#include "../utils/bmp_file.h"

using namespace Beatmup;


InternalBitmap::InternalBitmap(Environment& env, PixelFormat pixelFormat, int width, int height, bool allocate) :
	AbstractBitmap(env),
	pixelFormat(pixelFormat), width(width), height(height),
	memory(0),
	data(nullptr)
{
	if (getBitsPerPixel() < 8) {
		int n = 8 / getBitsPerPixel();
		this->width = ceili(width, n) * n;
	}
    if (allocate)
		memory = env.allocateMemory(getMemorySize());
}


InternalBitmap::InternalBitmap(Environment& env, const char* filename) :
	AbstractBitmap(env),
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
	memory = env.allocateMemory(getMemorySize());
	Environment::Mem mem(env, memory);
	bmp.load(mem(), getMemorySize());
}


InternalBitmap::~InternalBitmap() {
	if (memory)
		env.freeMemory(memory);
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
		memory = env.allocateMemory(getMemorySize());
	if (!data)
		data = (pixbyte*)env.acquireMemory(memory);
}


void InternalBitmap::unlockPixelData() {
	if (data) {
		data = nullptr;
		env.releaseMemory(memory, !isUpToDate(ProcessingTarget::CPU));
	}
}


void InternalBitmap::unlockPixels() {
	AbstractBitmap::unlockPixels();
	/*if (isUpToDate(ProcessingTarget::GPU))
		// if it is not a mask, lets free CPU
		if (!isMask()) {
			invalidate(ProcessingTarget::CPU);
			// this will cause freeing the memory when releasing the chunk
		}*/
}


void InternalBitmap::saveBmp(const char* filename) {
	if (!memory)
		memory = env.allocateMemory(getMemorySize());
	Environment::Mem mem(env, memory);
	BmpFile::save(
		mem(),
		width, height,
		getBitsPerPixel(),
		filename
	);
}
