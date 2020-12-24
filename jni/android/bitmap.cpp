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

#include "bitmap.h"
#include "../../core/bitmap/abstract_bitmap.h"
#include <core/exception.h>
#include <core/gpu/pipeline.h>

using namespace Beatmup;
using namespace Android;


JNIEnv* Bitmap::getEnv() const {
    JNIEnv *env;
    jvm->AttachCurrentThread(&env, NULL);
    return env;
}


Bitmap::Bitmap(Context& ctx, JNIEnv * jenv, jobject bitmap) :
    AbstractBitmap(ctx), lockedPixels(NULL)
{
    jenv->GetJavaVM(&this->jvm);
    this->bitmap = jenv->NewGlobalRef(bitmap);

    // checking bitmap format
    AndroidBitmapFormat format = (AndroidBitmapFormat) getInfo().format;
    if (
        format != ANDROID_BITMAP_FORMAT_RGBA_8888 &&
        format != ANDROID_BITMAP_FORMAT_A_8
    )
        throw RuntimeError("Pixel format is not supported");
}


Bitmap::~Bitmap() {
    getEnv()->DeleteGlobalRef(bitmap);
}


AndroidBitmapInfo Bitmap::getInfo() const {
    AndroidBitmapInfo info;
    int result = AndroidBitmap_getInfo(getEnv(), bitmap, &info);
    if (result < 0)
        throw RuntimeError("AndroidBitmap_getInfo() failed");
    return info;
}


const PixelFormat Bitmap::getPixelFormat() const {
    if (lockedPixels)
        return lockedPixelFormat;

    AndroidBitmapFormat format = (AndroidBitmapFormat) getInfo().format;
    switch (format) {
        case ANDROID_BITMAP_FORMAT_RGBA_8888:
            return QuadByte;
        case ANDROID_BITMAP_FORMAT_A_8:
            return SingleByte;
        default:
            Insanity::insanity("AndroidBitmap_getInfo() returned unexpected format");
            return SingleByte;
    }
}


const int Bitmap::getWidth() const {
    if (lockedPixels)
        return lockedWidth;
    return getInfo().width;
}


const int Bitmap::getHeight() const {
    if (lockedPixels)
        return lockedHeight;
    return getInfo().height;
}


void Bitmap::lockPixelData() {
    BEATMUP_ASSERT_DEBUG(lockedPixels == nullptr);
    lockedWidth = getWidth();
    lockedHeight = getHeight();
    lockedPixelFormat = getPixelFormat();
    int result = AndroidBitmap_lockPixels(getEnv(), bitmap, &lockedPixels);
    if (result < 0)
        throw RuntimeError("AndroidBitmap_lockPixels() failed with error");
}


void Bitmap::unlockPixelData() {
    BEATMUP_ASSERT_DEBUG(lockedPixels != nullptr);
    AndroidBitmap_unlockPixels(getEnv(), bitmap);
    lockedPixels = nullptr;
}


const pixbyte* Bitmap::getData(int x, int y) const {
    RuntimeError::check(lockedPixels, "No pixel data available. Forget to lock the bitmap?");
    if (x < 0 || y < 0 || x >= lockedWidth || y >= lockedHeight)
        return nullptr;
    msize n = y * lockedWidth + x;
    return (pixbyte*)( (unsigned char*)lockedPixels + n * BITS_PER_PIXEL[lockedPixelFormat] / 8 );
}


pixbyte* Bitmap::getData(int x, int y) {
    RuntimeError::check(lockedPixels, "No pixel data available. Forget to lock the bitmap?");
    if (x < 0 || y < 0 || x >= lockedWidth || y >= lockedHeight)
        return nullptr;
    msize n = y * lockedWidth + x;
    return (pixbyte*)( (unsigned char*)lockedPixels + n * BITS_PER_PIXEL[lockedPixelFormat] / 8 );
}


const msize Bitmap::getMemorySize() const {
    return getWidth() * getHeight() * BITS_PER_PIXEL[ getPixelFormat() ] / 8;
}
