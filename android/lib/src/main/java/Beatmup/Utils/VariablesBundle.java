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

package Beatmup.Utils;

import Beatmup.*;
import Beatmup.Imaging.ColorMatrix;

/**
 * Collection storing GLSL program parameters (scalars, matrices, vectors) to communicate them from user to GPU-managing thread.
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
