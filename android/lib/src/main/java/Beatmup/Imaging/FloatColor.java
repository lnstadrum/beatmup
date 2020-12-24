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
 * Floating-point RGBA color value.
 */
public class FloatColor {
    public float r;     //!< red
    public float g;     //!< green
    public float b;     //!< blue
    public float a;     //!< alpha

    /**
     * Creates a zero-valued color.
     */
    public FloatColor() {
        r = g = b = a = 0;
    }

    /**
     * Creates a color by aggregating the four channels values components
     * @param r     the red channel value
     * @param g     the green channel value
     * @param b     the blue channel value
     * @param a     the alpha channel value
     */
    public FloatColor(float r, float g, float b, float a) {
        this.r = r;
        this.g = g;
        this.b = b;
        this.a = a;
        // This constructor is called by JNI, do not remove it.
    }

    /**
     * @return an integer approximation of the current color mapping [0, 1] input range to [0, 255] output range.
     */
    public Color getIntColor() {
        return new Color(Math.round(r * 255), Math.round(g * 255), Math.round(b * 255), Math.round(a * 255));
    }
}
