#include "bitmap.h"
#include <core/exception.h>
#include <core/gpu/pipeline.h>

using namespace Beatmup;
using namespace Android;


JNIEnv*Bitmap::getEnv() const {
    JNIEnv *env;
    jvm->AttachCurrentThread(&env, NULL);
    return env;
}


Bitmap::Bitmap(Environment& env, JNIEnv * jenv, jobject bitmap) :
    AbstractBitmap(env), lockedPixels(NULL)
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
    }
    Insanity::insanity("AndroidBitmap_getInfo() returned unexpected format");
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


void Bitmap::unlockPixels() {
    AbstractBitmap::unlockPixels();
}


pixbyte* Bitmap::getData(int x, int y) const {
    RuntimeError::check(lockedPixels, "No pixel data available. Forget to lock the bitmap?");
    if (x < 0 || y < 0 || x >= lockedWidth || y >= lockedHeight)
        return nullptr;
    msize n = y * lockedWidth + x;
    return (pixbyte*)( (unsigned char*)lockedPixels + n * BITS_PER_PIXEL[lockedPixelFormat] / 8 );
}


const msize Bitmap::getMemorySize() const {
    return getWidth() * getHeight() * BITS_PER_PIXEL[ getPixelFormat() ] / 8;
}