#include "bitmap_access.h"
#include "mask_bitmap_access.h"

const int
	Beatmup::MASK_LUT_1_BIT[2] = { 0, 255 },
	Beatmup::MASK_LUT_2_BITS[4] = { 0, 85, 170, 255 },
	Beatmup::MASK_LUT_4_BITS[16] = { 0, 17, 34, 51, 68, 85, 102, 119, 136, 153, 170, 187, 204, 221, 238, 255 };


Beatmup::BitmapContentModifier::BitmapContentModifier(AbstractBitmap& bitmap) {
	bitmap.invalidate(ProcessingTarget::GPU);
		// say to bitmap that it is gonna be processed on CPU, its GPU version is not up to date any more
}


void Beatmup::UnsupportedPixelFormat::check(AbstractBitmap& bitmap, PixelFormat pf) {
	if (bitmap.getPixelFormat() != pf)
		throw UnsupportedPixelFormat(bitmap.getPixelFormat());
}


void Beatmup::UnsupportedPixelFormat::assertMask(AbstractBitmap& bitmap) {
	if (!bitmap.isMask())
		throw UnsupportedPixelFormat(bitmap.getPixelFormat());
}


void Beatmup::UnsupportedPixelFormat::assertFloat(AbstractBitmap& bitmap) {
	if (!bitmap.isFloat())
		throw UnsupportedPixelFormat(bitmap.getPixelFormat());
}


void Beatmup::UnsupportedPixelFormat::assertInt(AbstractBitmap& bitmap) {
	if (!bitmap.isInteger())
		throw UnsupportedPixelFormat(bitmap.getPixelFormat());
}
