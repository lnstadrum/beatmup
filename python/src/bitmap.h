/*
    Beatmup image and signal processing library
    Copyright (C) 2020, lnstadrum

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

#include <pybind11/numpy.h>

#include "bitmap/abstract_bitmap.h"
#include "context.h"

namespace Beatmup {
    /**
        Classes and utilities for Python binding
    */
    namespace Python {

        /**
            Wrapper of Android.Graphics.Bitmap object
        */
        class Bitmap : public AbstractBitmap {
        private:
            pybind11::buffer_info data;
            PixelFormat format;
            int strideBytes;

            void lockPixelData() {}
            void unlockPixelData() {}

        public:
            /**
                Creates the bitmap from Android Bitmap java object
            */
            Bitmap(Beatmup::Context& context, pybind11::buffer& buffer);

            inline const PixelFormat getPixelFormat() const { return format; };

            inline const int getWidth() const { return (int)data.shape[data.ndim - 2]; }

            inline const int getHeight() const  { return (int)data.shape[data.ndim - 3]; }

            const pixbyte* getData(int x, int y) const;
            pixbyte* getData(int x, int y);

            const msize getMemorySize() const;

            /**
                Returns a mutable python buffer containing bitmap data.
            */
            inline pybind11::buffer_info getPythonBuffer() {
#ifdef BEATMUP_DEBUG
                DebugAssertion::check(isUpToDate(ProcessingTarget::CPU), "Returning a buffer for outdated Python bitmap");
#endif
                return pybind11::buffer_info(data.ptr, data.itemsize, data.format, data.ndim, data.shape, data.strides, false);
            }
        };

    }
}