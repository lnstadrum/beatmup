package Beatmup.Imaging.Filters;

import Beatmup.Bitmap;
import Beatmup.Context;
import Beatmup.Task;

/**
 * Basic correction operations
 */
public class ImageTuning extends Task {
    private static native long newImageTuning();
    private native void setBitmaps(long handle, Bitmap input, Bitmap output);

    private native void setHueOffset(long handle, float val);
    private native void setSaturationFactor(long handle, float val);
    private native void setValueFactor(long handle, float val);
    private native void setBrightness(long handle, float val);
    private native void setContrast(long handle, float val);

    private native float getHueOffset(long handle);
    private native float getSaturationFactor(long handle);
    private native float getValueFactor(long handle);
    private native float getBrightness(long handle);
    private native float getContrast(long handle);

    public ImageTuning(Context context) {
        super(context, newImageTuning());
    }

    public void setBitmaps(Bitmap input, Bitmap output) {
        setBitmaps(handle, input, output);
    }

    public void setHueOffset(float val) {
        setHueOffset(handle, val);
    }
    
    public void setSaturationFactor(float val) {
        setSaturationFactor(handle, val);
    }
    
    public void setValueFactor(float val) {
        setValueFactor(handle, val);
    }

    public void setBrightness(float val) {
        setBrightness(handle, val);
    }

    public void setContrast(float val) {
        setContrast(handle, val);
    }

    public float getHueOffset() {
        return getHueOffset(handle);
    }

    public float getSaturationFactor() {
        return getSaturationFactor(handle);
    }

    public float getValueFactor() {
        return getValueFactor(handle);
    }

    public float getBrightness() {
        return getBrightness(handle);
    }

    public float getContrast() {
        return getContrast(handle);
    }
}
