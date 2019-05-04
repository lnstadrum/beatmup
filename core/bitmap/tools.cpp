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

BitmapTools::BitmapTools(Environment& env) : env(env)
{}

const Environment& BitmapTools::getEnvironment() const {
	return env;
}

template<class writer> inline void renderChessboard(writer out, int width, int height, int cellSize) {
	for (int y = 0; y < height; y++)
	for (int x = 0; x < width; x++) {
		out = pixint1{ 255 * ((x / cellSize + y / cellSize) % 2) };
		out++;
	}
}


BitmapPtr BitmapTools::makeCopy(AbstractBitmap& source, PixelFormat newPixelFormat) {
	BitmapPtr dest = new Beatmup::InternalBitmap(env, newPixelFormat, source.getWidth(), source.getHeight());
	BitmapConverter converter;
	converter.setBitmaps(&source, dest);
	env.performTask(0, converter);
	return dest;
}


BitmapPtr BitmapTools::makeCopy(AbstractBitmap& source) {
	return makeCopy(source, source.getPixelFormat());
}


BitmapPtr BitmapTools::chessboard(int width, int height, int cellSize, PixelFormat pixelFormat) {
	if (cellSize <= 0)
		BEATMUP_ERROR("Chessboard cell size must be positive");
	if (!AbstractBitmap::isMask(pixelFormat))
		BEATMUP_ERROR("Mask pixel formats are supported");
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
			p += CHANNELS.A;
			*p = 1.0f;
			for (int x = area.A.x; x <= area.B.x; ++x)
				*(p += 4) = 1.0f;
		}
	// integer bitmap
	else if (bitmap.getPixelFormat() == QuadByte)
		for (int y = area.A.y; y <= area.B.y; ++y) {
			pixbyte* p = bitmap.getData(area.A.x, y);
			p += CHANNELS.A;
			*p = 255;
			for (int x = area.A.x; x <= area.B.x; ++x)
				*(p += 4) = 255;
		}
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