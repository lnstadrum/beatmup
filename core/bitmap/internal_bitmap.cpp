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

#include "internal_bitmap.h"
#include "../exception.h"
#include "../utils/bmp_file.h"
#include "../utils/utils.hpp"

using namespace Beatmup;


InternalBitmap::InternalBitmap(Context& ctx, PixelFormat pixelFormat, int width, int height, bool allocate) :
    AbstractBitmap(ctx),
    pixelFormat(pixelFormat), width(width), height(height)
{
    if (getBitsPerPixel() < 8) {
        int n = 8 / getBitsPerPixel();
        this->width = ceili(width, n) * n;
    }
    if (allocate)
        memory = AlignedMemory(getMemorySize());
    upToDate[ProcessingTarget::CPU] = allocate;
}


InternalBitmap::InternalBitmap(Context& ctx, const char* bmpFilename) :
    AbstractBitmap(ctx)
{
    // read header
    BmpFile bmp(bmpFilename);
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
            throw IOError(bmpFilename, "Unsupported pixel format");
    }
    this->width = bmp.getWidth();
    this->height = bmp.getHeight();

    // allocate & read
    memory = AlignedMemory(getMemorySize());
    bmp.load(memory(), getMemorySize());
}


void Beatmup::InternalBitmap::reshape(int width, int height) {
    if (this->width * this->height != width * height && memory) {
        this->width = width;
        this->height = height;
        memory = AlignedMemory(getMemorySize());
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


const pixbyte* InternalBitmap::getData(int x, int y) const {
    return memory.ptr<pixbyte>((y * width + x) * AbstractBitmap::BITS_PER_PIXEL[pixelFormat] / 8);
}

pixbyte* InternalBitmap::getData(int x, int y) {
    return memory.ptr<pixbyte>((y * width + x) * AbstractBitmap::BITS_PER_PIXEL[pixelFormat] / 8);
}


void InternalBitmap::lockPixelData() {
    if (!memory)
        memory = AlignedMemory(getMemorySize());
}