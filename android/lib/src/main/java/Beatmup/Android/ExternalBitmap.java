package Beatmup.Android;

import android.graphics.SurfaceTexture;

import Beatmup.Bitmap;
import Beatmup.Context;

public class ExternalBitmap extends Bitmap {
    private native static long newExternalImage(Context context);
    private native void bind(long handle);
    private native void notifyUpdate(long handle, int width, int height);
    private SurfaceTexture surfaceTexture;

    public ExternalBitmap(Context context) {
        super(context, newExternalImage(context));
        bind(handle);
    }

    public void notifyUpdate(int width, int height) {
        notifyUpdate(handle, width, height);
    }

    public SurfaceTexture getSurfaceTexture() {
        return surfaceTexture;
    }
}
