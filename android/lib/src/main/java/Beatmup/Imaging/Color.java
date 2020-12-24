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
 * Integer RGBA color value
 */
public class Color {
    public int r;       //!< red
    public int g;       //!< green
    public int b;       //!< blue
    public int a;       //!< alpha

    /**
     * Creates a zero-valued color.
     */
    public Color() {
        r = g = b = a = 0;
    }

    /**
     * Creates a color by aggregating the four channels values components
     * @param r     the red channel value
     * @param g     the green channel value
     * @param b     the blue channel value
     * @param a     the alpha channel value
     */
    public Color(int r, int g, int b, int a) {
        this.r = r;
        this.g = g;
        this.b = b;
        this.a = a;
        // This constructor is called by JNI, do not remove it.
    }

    /**
     * Constructs a color from its integer code.
     * @param code      The integer code.
     */
    public Color(int code) {
        b = code % 256;
        g = (code >> 8) % 256;
        r = (code >> 16) % 256;
        a = (code >> 24) % 256;
    }

    /**
     * @return the integer code of the current color.
     */
    public int getRgbaCode() {
        return r | g << 8 | b << 16 | a << 24;
    }

    /**
     * Builds a color from a string.
     * Accepts hexadecimal RGB and RGBA notations like "7f7f7f", "7f7f7fFF" (case-insensitive) and a comma-separed list of 3 or 4 decimal integer values like "127,127,127,255".
     * @param expr  The string expression
     * @return the corresponding color. If cannot parsem throws an exception.
     */
    public static Color parseString(String expr) throws IllegalArgumentException {
        // hex notation
        if (expr.matches("(\\d|[a-fA-F]){6}")) {
            Color c = new Color(Integer.parseInt(expr, 16));
            c.a = 255;
            return c;
        }
        if (expr.matches("(\\d|[a-fA-F]){8}"))
            return new Color(Integer.parseInt(expr, 16));
        // comma split notation
        String vals[] = expr.split(",");
        if (vals.length != 3 && vals.length != 4)
            throw new IllegalArgumentException("Cannot parse color from expression '" +expr+ "'");
        return new Color(
            Integer.parseInt(vals[0]),
            Integer.parseInt(vals[1]),
            Integer.parseInt(vals[2]),
            vals.length > 3 ? Integer.parseInt(vals[3]) : 255
        );
    }

    /**
     * Returns a scaled version of the current color by given factors.
     * @param factorRGB         The color components scaling factor
     * @param factorAlpha       The alpha component scaling factor
     * @return the scaled color value. The current instance of Color is not changed.
     */
    public Color scale(float factorRGB, float factorAlpha) {
        return new Color(
                Math.round(r * factorRGB),
                Math.round(g * factorRGB),
                Math.round(b * factorRGB),
                Math.round(a * factorAlpha)
        );
    }

    /**
     * Returns a fully saturated color from its hue.
     * @param hueDegrees    The hue in degrees.
     * @return the color having the given hue.
     */
    public static Color byHue(float hueDegrees) {
        final double
            SQRT3 = 1.732050807568877,
            hue = Math.toRadians(hueDegrees),
            r = Math.max(0, (2 * Math.cos(hue) + 1) / 3),
            g = Math.max(0, (SQRT3 * Math.sin(hue) - Math.cos(hue) + 1) / 3),
            b = Math.max(0, (1 - SQRT3 * Math.sin(hue) - Math.cos(hue)) / 3),
            norm = Math.max(r, Math.max(g, b));
        return new Color(
            (int) Math.floor(255 * r / norm),
            (int) Math.floor(255 * g / norm),
            (int) Math.floor(255 * b / norm),
            255
        );
    }

    /**
     * Predefined color constants.
     */
    public static final Color
            TRANSPARENT_BLACK = new Color(0, 0, 0, 0),
            BLACK       = new Color(0, 0, 0, 255),
            WHITE       = new Color(255, 255, 255, 255),
            RED         = new Color(255, 0, 0, 255),
            GREEN       = new Color(0, 255, 0, 255),
            BLUE        = new Color(0, 0, 255, 255),
            YELLOW      = new Color(255, 255, 0, 255),
            PURPLE      = new Color(255, 0, 255, 255),
            ORANGE      = new Color(255, 127, 0, 255),
            GRAY        = new Color(127, 127, 127, 255),

            FECAMP_SKY  = new Color(100, 140, 189, 255);    // sky color in fecamp.bmp
}