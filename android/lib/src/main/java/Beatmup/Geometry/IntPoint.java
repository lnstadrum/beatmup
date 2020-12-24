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
 * Integer-valued 2D point
 */
public class IntPoint {
    public int x, y;

    public IntPoint() {
        x = y = 0;
    }

    public IntPoint(int x, int y) {
        this.x = x;
        this.y = y;
    }

    public IntPoint(IntPoint another) {
        this.x = another.x;
        this.y = another.y;
    }

    public void set(int x, int y) {
        this.x = x;
        this.y = y;
    }

    public void assign(IntPoint another) {
        this.x = another.x;
        this.y = another.y;
    }
}