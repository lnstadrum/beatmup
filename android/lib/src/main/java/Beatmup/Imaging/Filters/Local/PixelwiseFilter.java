package Beatmup.Imaging.Filters.Local;

import Beatmup.Bitmap;
import Beatmup.Context;
import Beatmup.Task;

/**
 * Custom pixelwise filter
 */
class PixelwiseFilter extends Task {

    protected PixelwiseFilter(Context context, long handle) {
        super(context, handle);
    }

    private native void setBitmaps(long handle, Bitmap input, Bitmap output);

    public void setBitmaps(Bitmap input, Bitmap output) {
        setBitmaps(handle, input, output);
    }
}
