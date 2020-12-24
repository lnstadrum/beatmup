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

package Beatmup.Imaging.Filters;

import Beatmup.Context;
import Beatmup.Task;

/**
 * Sepia filter.
 * An example of PixelwiseFilter implementation.
 */
public class Sepia extends PixelwiseFilter {
    private static native long newSepia();

    /**
     * Creates a Sepia filter instance.
     * @param context       A Beatmup context instance
     */
    public Sepia(Context context) {
        super(context, newSepia());
    }
}
