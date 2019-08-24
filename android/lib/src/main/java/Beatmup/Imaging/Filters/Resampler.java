package Beatmup.Imaging.Filters;

import Beatmup.Bitmap;
import Beatmup.Context;
import Beatmup.Task;

/**
 * Bitmap resampling facility
 */
public class Resampler extends Task {
    private static native long newResampler();
    private native void setBitmaps(long handle, Bitmap in, Bitmap out);

    public Resampler(Context context) {
        super(context, newResampler());
    }

    public void setBitmaps(Bitmap input, Bitmap output) {
        setBitmaps(handle, input, output);
    }
}
