/*
    Beatmup image and signal processing library
    Copyright (C) 2020, lnstadrum

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

import Beatmup.Bitmap;
import Beatmup.Context;
import Beatmup.Task;

/**
 * Base class for image filters processing a given bitmap in a pixelwise fashion.
 */
class PixelwiseFilter extends Task {
    protected PixelwiseFilter(Context context, long handle) {
        super(context, handle);
    }

    private native void setInput(long handle, Bitmap input);
    private native void setOutput(long handle, Bitmap output);

    private Bitmap input, output;

    /**
     * Sets an image to process.
     * @param input     The image
     */
    public void setInput(Bitmap input) {
        this.input = input;
        setInput(handle, input);
    }

    /**
     * Sets an image receiving the filter result.
     * If the output image is not of the same size as the input image, an exception is thrown during the task execution.
     * @param output    The image
     */
    public void setOutput(Bitmap output) {
        this.output = output;
        setOutput(handle, output);
    }

    /**
     * @return the input image or null if not set.
     */
    public Bitmap getInput() {
        return input;
    }

    /**
     * @return the output image or null if not set.
     */
    public Bitmap getOutput() {
        return output;
    }
}
