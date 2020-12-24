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

#include "abstract_bitmap.h"
#include "../context.h"
#include "../gpu/pipeline.h"
#include "internal_bitmap.h"
#include "converter.h"
#include "../gpu/bgl.h"
#include "../exception.h"
#include "../gpu/swapper.h"
#include "../utils/bmp_file.h"
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
    const bool handleValid = hasValidHandle();

    if (!handleValid)
        TextureHandler::prepare(gpu);
    glBindTexture(GL_TEXTURE_2D, textureHandle);

    // setup alignment
    if (isMask()) {
        // masks are stored as horizontally-stretched bitmaps
        const int textureWidth = getWidth() / (8 / getBitsPerPixel());

#ifdef BEATMUP_OPENGLVERSION_GLES20
        if (!handleValid)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, textureWidth, getHeight(), 0, GL_ALPHA, GL_UNSIGNED_BYTE, nullptr);
#else
        if (!handleValid)
            glTexStorage2D(GL_TEXTURE_2D, 1, GL_R8, textureWidth, getHeight());
#endif
        GL::GLException::check("allocating texture image (mask)");
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }

    else {
#ifdef BEATMUP_OPENGLVERSION_GLES20
        if (!handleValid)
            glTexImage2D(GL_TEXTURE_2D,
                0,
                GL::BITMAP_INTERNALFORMATS[getPixelFormat()],
                getWidth(), getHeight(),
                0,
                GL::BITMAP_PIXELFORMATS[getPixelFormat()],
                GL::BITMAP_PIXELTYPES[getPixelFormat()],
                nullptr);

#else
        if (!handleValid)
            glTexStorage2D(GL_TEXTURE_2D, 1, GL::BITMAP_INTERNALFORMATS[getPixelFormat()], getWidth(), getHeight());
#endif
        GL::GLException::check("allocating texture image");
    }
}


const GL::TextureHandler::TextureFormat AbstractBitmap::getTextureFormat() const {
    if (isMask())
        return TextureFormat::Rx8;
    return (TextureFormat)getPixelFormat();
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
        desc += " stored on CPU+GPU";
    else
        if (isUpToDate(ProcessingTarget::CPU))
            desc += " stored on CPU";
        else
            if (isUpToDate(ProcessingTarget::GPU))
                desc += " stored on GPU";
            else
                desc += " dirty";
    return desc;
}


void AbstractBitmap::saveBmp(const char* filename) {
    if (!isUpToDate(ProcessingTarget::CPU)) {
        // Grab output bitmap from GPU memory to RAM
        Swapper::pullPixels(*this);
    }

    lockPixelData();

    BmpFile::save(
        getData(0, 0),
        getWidth(), getHeight(),
        getBitsPerPixel(),
        filename
    );

    unlockPixelData();
}


AbstractBitmap::AbstractBitmap(Context& ctx) : ctx(ctx) {
    upToDate[ProcessingTarget::CPU] = true;
    upToDate[ProcessingTarget::GPU] = false;
}


AbstractBitmap::~AbstractBitmap() {
    if (hasValidHandle()) {
        TextureHandler::invalidate(*ctx.getGpuRecycleBin());
    }
}


Context& AbstractBitmap::getContext() const {
    return ctx;
}


void AbstractBitmap::zero() {
    lockPixelData();
    memset(getData(0, 0), 0, getMemorySize());
    unlockPixelData();
    upToDate[ProcessingTarget::CPU] = true;
    upToDate[ProcessingTarget::GPU] = false;
}


AbstractBitmap::ReadLock::ReadLock(AbstractBitmap& bitmap): bitmap(bitmap) {
#ifdef BEATMUP_DEBUG
    DebugAssertion::check(!bitmap.isDirty(), "Reading a dirty bitmap");
#endif
    if (bitmap.isUpToDate(ProcessingTarget::GPU) && !bitmap.isUpToDate(ProcessingTarget::CPU))
        Swapper::pullPixels(bitmap);
    bitmap.lockPixelData();
}


AbstractBitmap::ReadLock::~ReadLock() {
    bitmap.unlockPixelData();
}