package Beatmup.Imaging.Filters;

import Beatmup.Bitmap;
import Beatmup.Context;
import Beatmup.Task;

/**
 * Bitmap resampling facility
 */
public class Resampler extends Task {
    public enum Mode {
        NEAREST_NEIGHBOR,    //!< zero-order: usual nearest neighbor
        BOX,                 //!< "0.5-order": anti-aliasing box filter; identical to nearest neigbor when upsampling
        LINEAR,              //!< first order: bilinear interpolation
        CUBIC,               //!< third order: bicubic interpolation
        CONVNET              //!< upsampling x2 using a convolutional neural network
    };

    private static native long newResampler();
    private native void setBitmaps(long handle, Bitmap in, Bitmap out);
    private native void setMode(long handle, int mode);
    private native int getMode(long handle);
    private native void setCubicParameter(long handle, float val);
    private native float getCubicParameter(long handle);

    public Resampler(Context context) {
        super(context, newResampler());
    }

    public void setBitmaps(Bitmap input, Bitmap output) {
        setBitmaps(handle, input, output);
    }

    public void setMode(Mode mode) {
        setMode(handle, mode.ordinal());
    }

    public Mode getMode() {
        return Mode.values()[ getMode(handle) ];
    }

    public void setCubicParameter(float alpha) {
        setCubicParameter(handle, alpha);
    }

    public float getCubicParameter() {
        return getCubicParameter();
    }
}
