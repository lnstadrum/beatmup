package Beatmup.Utils;

import Beatmup.*;
import Beatmup.Imaging.ColorMatrix;

/**
 * Collection of shader variables values
 */
public class VariablesBundle extends Beatmup.Object {
    private native void setInteger1(long handle, String name, int value);
    private native void setInteger2(long handle, String name, int x, int y);
    private native void setInteger3(long handle, String name, int x, int y, int z);
    private native void setInteger4(long handle, String name, int x, int y, int z, int w);

    private native void setFloat1(long handle, String name, float value);
    private native void setFloat2(long handle, String name, float x, float y);
    private native void setFloat3(long handle, String name, float x, float y, float z);
    private native void setFloat4(long handle, String name, float x, float y, float z, float w);

    private native void setFloatMatrix(long handle, String name, float[] x);
    private native void setFloatMatrixFromColorMatrix(long handle, String name, ColorMatrix matrix);

    protected VariablesBundle(long handle) {
        super(handle);
    }

    public void setInteger(String name, int value) {
        setInteger1(handle, name, value);
    }

    public void setInteger(String name, int x, int y) {
        setInteger2(handle, name, x, y);
    }

    public void setInteger(String name, int x, int y, int z) {
        setInteger3(handle, name, x, y, z);
    }

    public void setInteger(String name, int x, int y, int z, int w) {
        setInteger4(handle, name, x, y, z, w);
    }

    public void setFloat(String name, float value) {
        setFloat1(handle, name, value);
    }

    public void setFloat(String name, float x, float y) {
        setFloat2(handle, name, x, y);
    }

    public void setFloat(String name, float x, float y, float z) {
        setFloat3(handle, name, x, y, z);
    }

    public void setFloat(String name, float x, float y, float z, float w) {
        setFloat4(handle, name, x, y, z, w);
    }

    public void setFloatMatrix(String name, float[] coefficients) {
        setFloatMatrix(handle, name, coefficients);
    }

    public void setColorMatrix(String name, ColorMatrix matrix) {
        setFloatMatrixFromColorMatrix(handle, name, matrix);
    }
}
