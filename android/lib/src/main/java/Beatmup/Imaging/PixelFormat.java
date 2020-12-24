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

package Beatmup.Imaging;

/**
 * Enumeration of Beatmup pixel formats
 */
public enum PixelFormat {
    SingleByte,
    TripleByte,
    QuadByte,

    SingleFloat,
    TripleFloat,
    QuadFloat,

    BinaryMask,
    QuaternaryMask,
    HexMask;

    private int[] BITS_PER_PIXEL = {8, 24, 32, 32, 96, 128, 1, 2, 4};

    public int getBitsPerPixel() {
        return BITS_PER_PIXEL[ordinal()];
    }
}