#pragma once

#include <jni.h>

#include <android/bitmap.h>

#include <core/bitmap/abstract_bitmap.h>
#include <core/environment.h>

namespace Beatmup {
    namespace Android {

        /**
            Wrapper of Android.Graphics.Bitmap object
        */
        class Bitmap : public AbstractBitmap {
        private:
            jobject bitmap;             //!< java object representing the bitmap
            JavaVM *jvm;                //!< java environment

            void *lockedPixels;         //!< pixel buffer; available only after calling lockPixels()
            int lockedWidth, lockedHeight;
            PixelFormat lockedPixelFormat;

            AndroidBitmapInfo getInfo() const;

            JNIEnv *getEnv() const;

            void lockPixelData();

            void unlockPixelData();

        public:
            /**
                Creates the bitmap from Android Bitmap java object
            */
            Bitmap(Beatmup::Environment &, JNIEnv *, jobject);

            ~Bitmap();

            const PixelFormat getPixelFormat() const;

            const int getWidth() const;

            const int getHeight() const;

            void unlockPixels();

            const pixptr getData(int x, int y) const;

            const msize getMemorySize() const;
        };

    }
}