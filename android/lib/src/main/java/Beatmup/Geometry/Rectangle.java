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

package Beatmup.Geometry;

/**
 * Floating point-valued rectangle
 */
public class Rectangle {
    public float x1, y1, x2, y2;

    public Rectangle() {
        x1 = y1 = x2 = y2 = 0;
    }

    public Rectangle(float x1, float y1, float x2, float y2) {
        this.x1 = x1;
        this.y1 = y1;
        this.x2 = x2;
        this.y2 = y2;
    }

    public Rectangle(float centerX, float centerY, float size) {
        x1 = centerX - size/2;
        y1 = centerY - size/2;
        x2 = centerX + size/2;
        y2 = centerY + size/2;
    }

    public float getWidth() {
        return x2-x1;
    }

    public float getHeight() {
        return y2-y1;
    }

    public float getCenterX() {
        return (x1 + x2) / 2;
    }

    public float getCenterY() {
        return (y1 + y2) / 2;
    }

    /**
     * Computes integer-valued rectangle
     * @param width     output domain width in pixels
     * @param height    output domain height in pixels
     * @return the resulting rectangle
     */
    public IntRectangle toIntegerRect(int width, int height) {
        return new IntRectangle(
            Math.round(x1*width),
            Math.round(y1*height),
            Math.round(x2*width),
            Math.round(y2*height)
        );
    }

    /**
     * Expands the rectangle so that it have a given point inside or on the boundary
     * @param x     horizontal coordinate of the given point
     * @param y     horizontal coordinate of the given point
     */
    public void expandTo(float x, float y) {
        if (x < x1)
            x1 = x;
        if (x2 < x)
            x2 = x;
        if (y < y1)
            y1 = y;
        if (y2 < y)
            y2 = y;
    }

    /**
     * @return a big square centered at the rectangle center
     */
    public Rectangle squarizeMax() {
        Rectangle rectangle = new Rectangle();
        float
                centerX = (x1+x2)/2,
                centerY = (y1+y2)/2,
                sizeDiv2 = Math.abs(Math.max(x2-x1, y2-y1))/2;
        rectangle.x1 = centerX-sizeDiv2;
        rectangle.y1 = centerY-sizeDiv2;
        rectangle.x2 = centerX+sizeDiv2;
        rectangle.y2 = centerY+sizeDiv2;
        return rectangle;
    }


    public static float fit(float innerWidth, float innerHeight, float outerWidth, float outerHeight) {
        return innerWidth * outerHeight > innerHeight * outerWidth ?
                outerWidth / innerWidth : outerHeight / innerHeight;
    }

}
