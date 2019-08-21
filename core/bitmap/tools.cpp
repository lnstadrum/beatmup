#include "tools.h"
#include "bitmap_access.h"
#include "converter.h"
#include "processing.h"


using namespace Beatmup;


template<class in_t> class ScanlineSearch {
public:
	static void process(in_t in, const typename in_t::pixtype& target, const IntPoint& start, IntPoint& result) {
        typename in_t::pixtype convTarget;
        convTarget = target;
		int x = start.x, y = start.y;
		const int W = in.getWidth(), H = in.getHeight();
		in.goTo(x,y);
        do {
			if (in() == convTarget) {
				result.x = x;
				result.y = y;
				return;
			}
			x++;
			if (x >= W) {
				x = 0;
				y++;
			}
			in++;
		} while (y < H);
		result.x = result.y = -1;
	}
};


template<class writer> inline void renderChessboard(writer out, int width, int height, int cellSize) {
	for (int y = 0; y < height; y++)
	for (int x = 0; x < width; x++) {
		out = pixint1{ 255 * ((x / cellSize + y / cellSize) % 2) };
		out++;
	}
}


AbstractBitmap* BitmapTools::makeCopy(AbstractBitmap& source, PixelFormat newPixelFormat) {
	AbstractBitmap* copy = new Beatmup::InternalBitmap(source.getEnvironment(), newPixelFormat, source.getWidth(), source.getHeight());
	BitmapConverter converter;
	converter.setBitmaps(&source, copy);
	source.getEnvironment().performTask(converter);
	return copy;
}


AbstractBitmap* BitmapTools::makeCopy(AbstractBitmap& source) {
	return makeCopy(source, source.getPixelFormat());
}


AbstractBitmap* BitmapTools::chessboard(Environment& env, int width, int height, int cellSize, PixelFormat pixelFormat) {
	RuntimeError::check(cellSize > 0, "Chessboard cell size must be positive");
	RuntimeError::check(AbstractBitmap::isMask(pixelFormat), "Mask pixel formats are supported only");
	BitmapPtr chess = new Beatmup::InternalBitmap(env, pixelFormat, width, height);
	chess->lockPixels(ProcessingTarget::CPU);
	switch (pixelFormat) {
	case BinaryMask:
		renderChessboard<BinaryMaskWriter>(BinaryMaskWriter(*chess), width, height, cellSize);
		break;
	case QuaternaryMask:
		renderChessboard<QuaternaryMaskWriter>(QuaternaryMaskWriter(*chess), width, height, cellSize);
		break;
	case HexMask:
		renderChessboard<HexMaskWriter>(HexMaskWriter(*chess), width, height, cellSize);
		break;
	default:
		throw BitmapProcessing::ProcessingActionNotImplemented(pixelFormat);
	}
	chess->unlockPixels();
	return chess;
}


void BitmapTools::makeOpaque(AbstractBitmap& bitmap, IntRectangle area) {
	// floating-point bitmap
	if (bitmap.getPixelFormat() == QuadFloat)
		for (int y = area.A.y; y <= area.B.y; ++y) {
			pixfloat* p = (pixfloat*)bitmap.getData(area.A.x, y);
			p += CHANNELS_4.A;
			*p = 1.0f;
			for (int x = area.A.x; x <= area.B.x; ++x)
				*(p += 4) = 1.0f;
		}
	// integer bitmap
	else if (bitmap.getPixelFormat() == QuadByte)
		for (int y = area.A.y; y <= area.B.y; ++y) {
			pixbyte* p = bitmap.getData(area.A.x, y);
			p += CHANNELS_4.A;
			*p = 255;
			for (int x = area.A.x; x <= area.B.x; ++x)
				*(p += 4) = 255;
		}
}


void BitmapTools::invert(AbstractBitmap& input, AbstractBitmap& output) {
	RuntimeError::check(input.getWidth() == output.getWidth() && input.getHeight() <= output.getHeight(),
		"Input size does not fit output size");
	RuntimeError::check(input.getPixelFormat() == output.getPixelFormat(),
		"Input/output pixel formats mismatch");

	input.lockPixels(ProcessingTarget::CPU);
	output.lockPixels(ProcessingTarget::CPU);

	const size_t NPIX = input.getSize().numPixels();
	if (input.isFloat()) {
		pixfloat
			*pi = (pixfloat*)input.getData(0, 0),
			*po = (pixfloat*)output.getData(0, 0);
		const pixfloat* STOP = pi + NPIX * AbstractBitmap::CHANNELS_PER_PIXEL[input.getPixelFormat()];
		while (pi < STOP)
			*(po++) = 1 - *(pi++);
	}
	else {
		const size_t N = NPIX * AbstractBitmap::BITS_PER_PIXEL[input.getPixelFormat()] / 8;
		// fast integer inverse
		pixint_platform
			*pi = (pixint_platform*)input.getData(0, 0),
			*po = (pixint_platform*)output.getData(0, 0);
		const pixint_platform* STOP = pi + N / sizeof(pixint_platform);
		while (pi < STOP)
			*(po++) = ~*(pi++);
		// process remaining bytes
		pixbyte
			*ri = (pixbyte*)pi,
			*ro = (pixbyte*)po;
		for (int r = 0; r < N % sizeof(pixint_platform); ++r)
			*(ro++) = ~*(ri++);
	}
	input.unlockPixels();
	output.unlockPixels();
}


IntPoint BitmapTools::scanlineSearch(AbstractBitmap& source, pixint4 val, const IntPoint& startFrom) {
	IntPoint result;
	source.lockPixels(ProcessingTarget::CPU);
	BitmapProcessing::read<ScanlineSearch>(source, startFrom.x, startFrom.y, val, startFrom, result);
	source.unlockPixels();
    return result;
}


IntPoint BitmapTools::scanlineSearch(AbstractBitmap& source, pixfloat4 val, const IntPoint& startFrom) {
    IntPoint result;
    source.lockPixels(ProcessingTarget::CPU);
    BitmapProcessing::read<ScanlineSearch>(source, startFrom.x, startFrom.y, val, startFrom, result);
    source.unlockPixels();
    return result;
}
