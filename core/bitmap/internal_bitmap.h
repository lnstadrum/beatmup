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

#include "../basic_types.h"
#include "../context.h"
#include "../memory.h"
#include "abstract_bitmap.h"
#include <vector>

namespace Beatmup {

    /**
        Bitmap whose memory is managed by the Beatmup engine.
        Main pixel data container used internally by Beatmup. Applications would typically use a different incarnation
        of AbstractBitmap implementing I/O operations, and InternalBitmap instances are used to exchange data between
        different processing entities (AbstractTask instances) within the application.
    */
    class InternalBitmap : public AbstractBitmap {
    private:
        PixelFormat pixelFormat;
        int width, height;
        AlignedMemory memory;

        void lockPixelData();
        inline void unlockPixelData() {}

    public:
        /**
            Creates a bitmap.
            The new bitmap created this way is "dirty" (contains random content). It is up to the application to fill
            it with a content (e.g., set to zero).
            \param ctx              A Beatmup context instance
            \param pixelFormat      Pixel format
            \param width            Bitmap width in pixels
            \param height           Bitmap height in pixels
            \param allocate         If `true`, a storage is allocated in RAM. Otherwise the allocation is deferred
                                    till the first use of the bitmap data by CPU. It is convenient to not allocate
                                    if the bitmap is only used as a texture handler to store intremediate data when
                                    processing on GPU.
        */
        InternalBitmap(Context& ctx, PixelFormat pixelFormat, int width, int height, bool allocate = true);

        /**
            Loads image from a BMP file.
            \param ctx              A Beatmup context instance
            \param bmpFilename      Name of the .bmp file to be loaded
            Raises exceptions if the input file is not readable or not a valid BMP image.
        */
        InternalBitmap(Context& ctx, const char* bmpFilename);

        /**
            Changes bitmap size. Reallocates the memory if necessary.
            Bitmap becomes "dirty" (contains no valid content).
            \param[in] width        New width in pixels
            \param[in] height    New height in pixels
        */
        void reshape(int width, int height);

        const PixelFormat getPixelFormat() const;
        const int getWidth() const;
        const int getHeight() const;
        const msize getMemorySize() const;
        const pixbyte* getData(int x, int y) const;
        pixbyte* getData(int x, int y);
    };

}
