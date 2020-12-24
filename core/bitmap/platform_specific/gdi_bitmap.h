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

#include "../abstract_bitmap.h"
#ifndef BEATMUP_PLATFORM_WINDOWS
    #error GDI bitmap is Windows-specific.
#endif

namespace Beatmup {

    /**
        A simple wrapper of GDI bitmap
    */
    class GDIBitmap : public AbstractBitmap {
    private:
        class Impl;
        Impl* impl;

    public:
        GDIBitmap(Context &ctx, const wchar_t* filename);

        GDIBitmap(Context &ctx, PixelFormat format, int width, int height);

        const PixelFormat getPixelFormat() const;

        const int getWidth() const;

        const int getHeight() const;

        int getStride() const;

        const msize getMemorySize() const;

        void lockPixelData();

        void unlockPixelData();

        const pixbyte* getData(int x, int y) const;

        void save(const wchar_t* filename);
    };

}
