/*
    Useful utilities to work with bitmaps
*/

#pragma once
#include "abstract_bitmap.h"
#include "internal_bitmap.h"
#include "../geometry.h"
#include "pixel_arithmetic.h"

namespace Beatmup {
    namespace BitmapTools {
        /**
            \brief Makes a copy of a bitmap with a specified pixel format
        */
        AbstractBitmap* makeCopy(AbstractBitmap& source, PixelFormat newPixelFormat);
        
        /**
            \brief Makes a copy of a bitmap
        */
        AbstractBitmap* makeCopy(AbstractBitmap& source);

        /**
            Generates a chessboard
        */
        AbstractBitmap* chessboard(Context& ctx, int width, int height, int cellSize, PixelFormat pixelFormat = BinaryMask);
                
        /**
            Makes a bitmap area opaque
        */
        void makeOpaque(AbstractBitmap&, IntRectangle);

        /**
            Computes the pixelwise inverse of a bitmap
        */
        void invert(AbstractBitmap& input, AbstractBitmap& output);


        /**
         * Searches for a pixel of a given value in scaline order starting from a given point
         * @param source 		the bitmap to look in
         * @param val 			the pixel value to look for
         * @param startFrom 	the starting position
         * @return the next closest position of the searched value (in scaline order) or (-1,-1) if
         * not found.
         */
        IntPoint scanlineSearch(AbstractBitmap& source, pixint4 val, const IntPoint& startFrom);
        IntPoint scanlineSearch(AbstractBitmap& source, pixfloat4 val, const IntPoint& startFrom);
    }
}