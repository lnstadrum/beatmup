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

#include "bitmap_access.h"
#include "mask_bitmap_access.h"

const int
    Beatmup::MASK_LUT_1_BIT[2] = { 0, 255 },
    Beatmup::MASK_LUT_2_BITS[4] = { 0, 85, 170, 255 },
    Beatmup::MASK_LUT_4_BITS[16] = { 0, 17, 34, 51, 68, 85, 102, 119, 136, 153, 170, 187, 204, 221, 238, 255 };


void Beatmup::UnsupportedPixelFormat::check(AbstractBitmap& bitmap, PixelFormat pf) {
    if (bitmap.getPixelFormat() != pf)
        throw UnsupportedPixelFormat(bitmap.getPixelFormat());
}


void Beatmup::UnsupportedPixelFormat::assertMask(AbstractBitmap& bitmap) {
    if (!bitmap.isMask())
        throw UnsupportedPixelFormat(bitmap.getPixelFormat());
}


void Beatmup::UnsupportedPixelFormat::assertFloat(AbstractBitmap& bitmap) {
    if (!bitmap.isFloat())
        throw UnsupportedPixelFormat(bitmap.getPixelFormat());
}


void Beatmup::UnsupportedPixelFormat::assertInt(AbstractBitmap& bitmap) {
    if (!bitmap.isInteger())
        throw UnsupportedPixelFormat(bitmap.getPixelFormat());
}
