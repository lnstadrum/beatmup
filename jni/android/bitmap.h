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

#pragma once

#include <jni.h>

#include <android/bitmap.h>

#include <core/bitmap/abstract_bitmap.h>
#include <core/context.h>

namespace Beatmup {
    namespace Android {

        /**
            Wrapper of Android.Graphics.Bitmap object
        */
        class Bitmap : public AbstractBitmap {
        private:
            jobject bitmap;             //!< java object representing the bitmap
            JavaVM *jvm;                //!< java environment

            void *lockedPixels;         //!< pixel buffer; available only after calling lockPixelData()
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
            Bitmap(Beatmup::Context &, JNIEnv *, jobject);

            ~Bitmap();

            const PixelFormat getPixelFormat() const;

            const int getWidth() const;

            const int getHeight() const;

            const pixbyte* getData(int x, int y) const;
            pixbyte* getData(int x, int y);

            const msize getMemorySize() const;
        };

    }
}
