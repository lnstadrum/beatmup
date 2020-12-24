/*
    Beatmup image and signal processing library
    Copyright (C) 2019, lnstadrum

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "tools.h"
#include "bitmap_access.h"
#include "converter.h"
#include "processing.h"
#include <cstdlib>


using namespace Beatmup;


namespace Kernels {
    template<class in_t> class ScanlineSearch {
    public:
        static void process(AbstractBitmap& bitmap, const typename in_t::pixtype& target, const IntPoint& start, IntPoint& result) {
            in_t in(bitmap, start.x, start.y);

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


    template<class out_t> class ChessboardRendering {
    public:
        static void process(AbstractBitmap& bitmap, int width, int height, int cellSize) {
            out_t out(bitmap);
            for (int y = 0; y < height; y++)
            for (int x = 0; x < width; x++) {
                out = pixint1{ 255 * ((x / cellSize + y / cellSize) % 2) };
                out++;
            }
        }
    };
}


InternalBitmap* BitmapTools::makeCopy(AbstractBitmap& source) {
    return makeCopy(source, source.getPixelFormat());
}


InternalBitmap* BitmapTools::makeCopy(AbstractBitmap& source, PixelFormat newPixelFormat) {
    return makeCopy(source, source.getContext(), newPixelFormat);
}


InternalBitmap* BitmapTools::makeCopy(AbstractBitmap& source, Context& context, PixelFormat newPixelFormat) {
    InternalBitmap* copy = new Beatmup::InternalBitmap(context, newPixelFormat, source.getWidth(), source.getHeight());
    FormatConverter converter;
    converter.setBitmaps(&source, copy);
    source.getContext().performTask(converter);
    return copy;
}


InternalBitmap* BitmapTools::chessboard(Context& ctx, int width, int height, int cellSize, PixelFormat pixelFormat) {
    RuntimeError::check(cellSize > 0, "Chessboard cell size must be positive");
    InternalBitmap* chess = new Beatmup::InternalBitmap(ctx, pixelFormat, width, height);
    AbstractBitmap::WriteLock<ProcessingTarget::CPU> lock(*chess);
    BitmapProcessing::write<Kernels::ChessboardRendering>(*chess, width, height, cellSize);
    return chess;
}


void BitmapTools::noise(AbstractBitmap& bitmap, IntRectangle area) {
    AbstractBitmap::WriteLock<ProcessingTarget::CPU> lock(bitmap);
    const int n = bitmap.getNumberOfChannels();

    if (bitmap.isMask())
        throw ImplementationUnsupported("Noise is not implemented for mask bitmaps");
    // floating-point bitmap
    else if (bitmap.isFloat())
        for (int y = area.a.y; y <= area.b.y; ++y) {
            pixfloat* p = (pixfloat*)bitmap.getData(area.a.x, y);
            for (int x = area.a.x; x <= area.b.x; ++x)
                for (int i = 0; i < n; ++i, ++p)
                    *p = (float)std::rand() / RAND_MAX;
        }
    // integer bitmap
    else if (bitmap.isInteger()) {
        for (int y = area.a.y; y <= area.b.y; ++y) {
            pixbyte* p = bitmap.getData(area.a.x, y);
            for (int x = area.a.x; x <= area.b.x; ++x)
                for (int i = 0; i < n; ++i, ++p)
                    *p = std::rand() % 256;
        }
    }
}


void BitmapTools::noise(AbstractBitmap& bitmap) {
    noise(bitmap, IntRectangle(0, 0, bitmap.getWidth() - 1, bitmap.getHeight() - 1));
}


void BitmapTools::makeOpaque(AbstractBitmap& bitmap, IntRectangle area) {
    AbstractBitmap::WriteLock<ProcessingTarget::CPU> lock(bitmap);

    // floating-point bitmap
    if (bitmap.getPixelFormat() == QuadFloat)
        for (int y = area.a.y; y <= area.b.y; ++y) {
            pixfloat* p = (pixfloat*)bitmap.getData(area.a.x, y);
            p += CHANNELS_4.A;
            *p = 1.0f;
            for (int x = area.a.x + 1; x <= area.b.x; ++x)
                *(p += 4) = 1.0f;
        }
    // integer bitmap
    else if (bitmap.getPixelFormat() == QuadByte)
        for (int y = area.a.y; y <= area.b.y; ++y) {
            pixbyte* p = bitmap.getData(area.a.x, y);
            p += CHANNELS_4.A;
            *p = 255;
            for (int x = area.a.x + 1; x <= area.b.x; ++x)
                *(p += 4) = 255;
        }
}


void BitmapTools::invert(AbstractBitmap& input, AbstractBitmap& output) {
    RuntimeError::check(input.getWidth() == output.getWidth() && input.getHeight() <= output.getHeight(),
        "Input size does not fit output size");
    RuntimeError::check(input.getPixelFormat() == output.getPixelFormat(),
        "Input/output pixel formats mismatch");

    AbstractBitmap::WriteLock<ProcessingTarget::CPU> outputLock(output);
    AbstractBitmap::ReadLock* readLock = (&input == &output) ? nullptr : new AbstractBitmap::ReadLock(input);


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

    delete readLock;
}


IntPoint BitmapTools::scanlineSearch(AbstractBitmap& source, pixint4 val, const IntPoint& startFrom) {
    IntPoint result;
    AbstractBitmap::ReadLock lock(source);
    BitmapProcessing::read<Kernels::ScanlineSearch>(source, val, startFrom, result);
    return result;
}


IntPoint BitmapTools::scanlineSearch(AbstractBitmap& source, pixfloat4 val, const IntPoint& startFrom) {
    IntPoint result;
    AbstractBitmap::ReadLock lock(source);
    BitmapProcessing::read<Kernels::ScanlineSearch>(source, val, startFrom, result);
    return result;
}
