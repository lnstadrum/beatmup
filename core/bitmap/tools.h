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
#include "abstract_bitmap.h"
#include "internal_bitmap.h"
#include "../geometry.h"
#include "pixel_arithmetic.h"

namespace Beatmup {
    /**
        Set of handy operations with images
    */
    namespace BitmapTools {
        /**
            Makes a copy of a bitmap.
            The copy is done in an AbstractTask run in the default thread pool of the context the bitmap is attached to.
            \param[in] bitmap       The bitmap to copy
        */
        InternalBitmap* makeCopy(AbstractBitmap& bitmap);

        /**
            Makes a copy of a bitmap converting the data to a given pixel format.
            The copy is done in an AbstractTask run in the default thread pool of the context the bitmap is attached to.
            \param[in] bitmap       The bitmap to copy
            \param[in] pixelFormat  Pixel format of the copy
        */
        InternalBitmap* makeCopy(AbstractBitmap& bitmap, PixelFormat pixelFormat);

        /**
            Makes a copy of a bitmap for a given Context converting the data to a given pixel format.
            Can be used to exchange image content between different instances of Context.
            The copy is done in an AbstractTask run in the default thread pool of the source bitmap context.
            \param[in] bitmap       The bitmap to copy
            \param[in] context      The Context instance the copy is associated with
            \param[in] pixelFormat  Pixel format of the copy
        */
        InternalBitmap* makeCopy(AbstractBitmap& bitmap, Context& context, PixelFormat pixelFormat);

        /**
            Renders a chessboard image.
            \param[in] context      A Context instance
            \param[in] width        Width in pixels of the resulting bitmap
            \param[in] height       Height in pixels of the resulting bitmap
            \param[in] cellSize     Size of a single chessboard cell in pixels
            \param[in] pixelFormat  Pixel format of the resulting bitmap
        */
        InternalBitmap* chessboard(Context& context, int width, int height, int cellSize, PixelFormat pixelFormat = BinaryMask);

        /**
            Replaces a rectangular area in a bitmap by random noise.
            \param[in] bitmap       The bitmap
            \param[in] area         The area in pixels to process
        */
        void noise(AbstractBitmap& bitmap, IntRectangle area);

        /**
            Fills a given bitmap with random noise.
            \param[in] bitmap       The bitmap to fill
        */
        void noise(AbstractBitmap& bitmap);

        /**
            Makes a bitmap area opaque.
            Applies for bitmaps having the alpha channel (of QuadByte and QuadFloat pixel formats). Bitmaps of other
            pixel formats remain unchanged.
            \param[in] bitmap       The bitmap
            \param[in] area         The area in pixels to process
        */
        void makeOpaque(AbstractBitmap& bitmap, IntRectangle area);

        /**
            Inverses colors of an image in a pixelwise fashion.
            \param[in] input        The input image. Its content unchanged.
            \param[in] output       The output image.
        */
        void invert(AbstractBitmap& input, AbstractBitmap& output);

        /**
            Goes through a bitmap in scanline order (left to right, top to bottom) until a pixel of a given color is met.
            \param[in] source       The bitmap to scan
            \param[in] val          The color value to look for
            \param[in] startFrom    Starting pixel position
            \return the next closest position of the searched value (in scanline order) or (-1,-1) if not found.
         */
        IntPoint scanlineSearch(AbstractBitmap& source, pixint4 val, const IntPoint& startFrom);
        IntPoint scanlineSearch(AbstractBitmap& source, pixfloat4 val, const IntPoint& startFrom);
    }
}
