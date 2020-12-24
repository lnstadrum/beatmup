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

package Beatmup.Imaging.Filters;

import Beatmup.Exceptions.CoreException;
import Beatmup.Imaging.Color;
import Beatmup.Context;
import Beatmup.Imaging.ColorMatrix;

/**
 * Color matrix filter: applies mapping Ax + B at each pixel of a given image in RGBA space.
 */
public class ColorMatrixTransform extends PixelwiseFilter {
    private static native long newColorMatrixTransform();
    private native void setFromMatrix(long handle, ColorMatrix matrix);
    private native void assignToMatrix(long handle, ColorMatrix matrix);
    private native void setCoefficients(long handle, int outputChannel, float add, float r, float g, float b, float a) throws CoreException;
    private native void setHSVCorrection(long handle, float Hdegrees, float S, float V);
    private native void setColorInversion(long handle, float preservedD, float preservedG, float preservedB, float S, float V);
    private native void setAllowIntegerApprox(long handle, boolean allow);
    private native boolean allowIntegerApprox(long handle);

    /**
     * Creates a color matrix transform instance.
     * @param context   A Beatmup context instance.
     */
    public ColorMatrixTransform(Context context) {
        super(context, newColorMatrixTransform());
    }

    /**
     * Sets color matrix coefficients for a specific output color channel.
     * @param outChannel        matrix line number (output channel)
     * @param bias              constant to add to the output channel
     * @param r                 red channel coefficient
     * @param g                 green channel coefficient
     * @param b                 blue channel coefficient
     * @param a                 alpha channel coefficient
     * @throws CoreException if the output channel index is out of range
     */
    public void setCoefficients(int outChannel, float bias, float r, float g, float b, float a) throws CoreException {
        setCoefficients(handle, outChannel, bias, r, g, b, a);
    }

    /**
     * Sets a predefined color matrix from a preset.
     * @param preset    the preset to set
     */
    public void setPreset(Preset preset) throws IllegalArgumentException {
        try {
            switch (preset) {
                case IDENTITY:
                    setCoefficients(0, 0, 1, 0, 0, 0);
                    setCoefficients(1, 0, 0, 1, 0, 0);
                    setCoefficients(2, 0, 0, 0, 1, 0);
                    setCoefficients(3, 0, 0, 0, 0, 1);
                    return;
                case GRAYSCALE:
                    setCoefficients(0, 0, 1.0f / 3, 1.0f / 3, 1.0f / 3, 0);
                    setCoefficients(1, 0, 1.0f / 3, 1.0f / 3, 1.0f / 3, 0);
                    setCoefficients(2, 0, 1.0f / 3, 1.0f / 3, 1.0f / 3, 0);
                    setCoefficients(3, 0, 0, 0, 0, 1);
                    return;
                case NEGATIVE:
                    setCoefficients(0, 1, -1, 0, 0, 0);
                    setCoefficients(1, 1, 0, -1, 0, 0);
                    setCoefficients(2, 1, 0, 0, -1, 0);
                    setCoefficients(3, 1, 0, 0, 0, 1);
                    return;
            }
        } catch (CoreException whatever) {}
        throw new IllegalArgumentException("Unknown preset: " + preset.toString());
    }

    /**
     * Resets the current transformation to a matrix performing standard HSV correction.
     * @param hueShiftDegrees       Hue shift in degrees
     * @param saturationFactor      Saturation scaling factor
     * @param valueFactor           Value scaling factor
     */
    public void setHSVCorrection(float hueShiftDegrees, float saturationFactor, float valueFactor) {
        setHSVCorrection(handle, hueShiftDegrees, saturationFactor, valueFactor);
    }

    /**
     * Resets the current transformation to a fancy color inversion mode with a fixed hue point.
     * @param preservedColor        A color giving the fixed hue point: all the colors of the same hue are preserved.
     * @param saturationFactor      Saturation scaling factor (in HSV sense)
     * @param valueFactor           Value scaling factor (in HSV sense)
     */
    public void setColorInversion(Color preservedColor, float saturationFactor, float valueFactor) {
        setColorInversion(handle, preservedColor.r / 255.0f, preservedColor.g / 255.0f, preservedColor.b / 255.0f, saturationFactor, valueFactor);
    }

    /**
     * Predefined color matrix transformation presets.
     */
    public enum Preset {
        IDENTITY,       //!< bypass
        GRAYSCALE,      //!< color to gray
        NEGATIVE        //!< color inversion
    }
}
