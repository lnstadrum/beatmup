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
 * RGBA color mapping.
 * A color value is mapped onto another color value by multiplying the 4x4 matrix by an RGBA input column.
 */
public class ColorMatrix extends Beatmup.Object {
    private static native long newColorMatrix();
    private static native long newColorMatrix(float hueDegrees, float S, float V);
    private static native long newColorMatrix(float r, float g, float b, float S, float V);
    private static native void assign(long handle, long fromHandle);
    private static native void multiply(long leftHandle, long rightHandle, long resultHandle);
    private native float getElement(long handle, int row, int col);
    private native void setElement(long handle, int row, int col, float val);

    /**
     * Creates a new ColorMatrix instance
     */
    public ColorMatrix() {
        super(newColorMatrix());
    }

    /**
     * Constructs a color matrix representing an HSV correction transformation.
     * @param hueDegrees        Hue offset in degrees
     * @param saturationFactor  Saturation scaling factor
     * @param valueFactor       Value scaling factor
     */
    public ColorMatrix(float hueDegrees, float saturationFactor, float valueFactor) {
        super(newColorMatrix(hueDegrees, saturationFactor, valueFactor));
    }

    /**
     * Constructs a color matrix representing continuous color inversion with a fixed hue point.
     * @param preservedColor	 Color giving a hue value that remains unchanged by the transform.
     * @param saturationFactor  Saturation scaling factor
     * @param valueFactor       Value scaling factor
     */
    public ColorMatrix(Color preservedColor, float saturationFactor, float valueFactor) {
        super(newColorMatrix(preservedColor.r, preservedColor.g, preservedColor.b, saturationFactor, valueFactor));
    }

    /**
     * Returns color matrix element value at a specific position.
     * @param row       The row number
     * @param col       The column number
     * @return matrix value at the given position.
     */
    public float get(int row, int col) {
        return getElement(handle, row, col);
    }

    /**
     * Changes color matrix element value at a specific position.
     * @param row       The row number
     * @param col       The column number
     * @param val       The new value to store at the given position
     */
    public void set(int row, int col, float val) {
        setElement(handle, row, col, val);
    }
}
