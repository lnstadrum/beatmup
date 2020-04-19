package Beatmup.Imaging;

import android.graphics.*;

import Beatmup.*;

/**
 * A matrix 4x4 representing a color transform
 */
public class ColorMatrix extends Beatmup.Object {
    private static native long newColorMatrix();
    private static native void assign(long handle, long fromHandle);
    private static native void multiply(long leftHandle, long rightHandle, long resultHandle);
    private native void setHSVCorrection(long handle, float hueDegrees, float S, float V);
    private native void setColorInversion(long handle, float r, float g, float b, float S, float V);
    private native float getElement(long handle, int row, int col);
    private native void setElement(long handle, int row, int col, float val);

    private ColorMatrix(long handle) {
        super(handle);
    }


    public ColorMatrix() {
        super(newColorMatrix());
    }


    public void setHSVCorrection(float hueDegrees, float saturationScale, float valueScale) {
        setHSVCorrection(handle, hueDegrees, saturationScale, valueScale);
    }


    public void setColorInversion(Color constant, float saturationScale, float valueScale) {
        setColorInversion(handle, constant.r, constant.g, constant.b, saturationScale, valueScale);
    }


    public float get(int row, int col) {
        return getElement(handle, row, col);
    }


    public void set(int row, int col, float val) {
        setElement(handle, row, col, val);
    }
}
