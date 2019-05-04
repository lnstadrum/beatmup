#include "internal_bitmap.h"

using namespace Beatmup;

InternalBitmap::InternalBitmap(Environment& env, PixelFormat pixelFormat, int width, int height) :
	AbstractBitmap(env),
	pixelFormat(pixelFormat), width(width), height(height),
	data(NULL)
{
	if (getBitsPerPixel() < 8) {
		int n = 8 / getBitsPerPixel();
		this->width = ceili(width, n) * n;
	}
	memory = env.allocateMemory(getMemorySize());
}


InternalBitmap::~InternalBitmap() {
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


const pixptr InternalBitmap::getData(int x, int y) const {
	return data + (y * width + x) * AbstractBitmap::BITS_PER_PIXEL[pixelFormat] / 8;
}


void InternalBitmap::lockPixelData() {
	if (!data)
		data = env.acquireMemory(memory);
}


void InternalBitmap::unlockPixelData() {
	if (data) {
		data = NULL;
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