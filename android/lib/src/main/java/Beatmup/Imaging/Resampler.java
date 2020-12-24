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

import Beatmup.Bitmap;
import Beatmup.Context;
import Beatmup.Task;

/**
 * Resamples an image to a given resolution.
 * Implements different resampling approaches, including standard ones (bilinear, bicubic, etc.) and a neural network-based 2x upsampling approach dubbed as "x2".
 */
public class Resampler extends Task {
    /**
     * Resampling mode (algorithm) specification.
     */
    public enum Mode {
        NEAREST_NEIGHBOR,    //!< zero-order: usual nearest neighbor
        BOX,                 //!< "0.5-order": anti-aliasing box filter; identical to nearest neighbor when upsampling
        LINEAR,              //!< first order: bilinear interpolation
        CUBIC,               //!< third order: bicubic interpolation
        CONVNET              //!< upsampling x2 using a convolutional neural network
    };

    private static native long newResampler(Context context);
    private native void setInput(long handle, Bitmap input);
    private native void setOutput(long handle, Bitmap output);
    private native void setMode(long handle, int mode);
    private native int getMode(long handle);
    private native void setCubicParameter(long handle, float val);
    private native float getCubicParameter(long handle);

    /**
     * Creates a resampler.
     * @param context       A context instance used to manage resources required to run some of resampling algorithms.
     */
    public Resampler(Context context) {
        super(context, newResampler(context));
    }

    /**
     * Sets the image to process.
     * @param input     The input image
     */
    public void setInput(Bitmap input) {
        setInput(handle, input);
    }

    /**
     * Sets the output image.
     * The output image contains the resampled version of the input one once the task finishes.
     * @param output    The output image
     */
    public void setOutput(Bitmap output) {
        setOutput(handle, output);
    }

    /**
     * Sets the resampling algorithm to use.
     * @param mode      The algorithm to use
     */
    public void setMode(Mode mode) {
        setMode(handle, mode.ordinal());
    }

    /**
     * @return currently selected resampling algorithm.
     */
    public Mode getMode() {
        return Mode.values()[ getMode(handle) ];
    }

    /**
     * Sets cubic interpolation parameter ("alpha").
     * Has no impact if the resampling mode is different from cubic.
     * @param alpha     The alpha parameter value
     */
    public void setCubicParameter(float alpha) {
        setCubicParameter(handle, alpha);
    }

    /**
     * @return cubic interpolation parameter ("alpha").
     */
    public float getCubicParameter() {
        return getCubicParameter(handle);
    }
}
