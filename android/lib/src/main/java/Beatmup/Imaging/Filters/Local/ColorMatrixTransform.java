package Beatmup.Imaging.Filters.Local;

import Beatmup.Imaging.Color;
import Beatmup.Context;
import Beatmup.Imaging.ColorMatrix;

/**
 * Pixelwise color matrix transformation
 * y = Ax + B
 * `y` and `x` are 4x1 vectors representing input and output pixel colors, `A` is a 4x4 matrix, `B`
 * is a 4x1 vector
 * Input and output color mapping to the 4-component vectors is performed according to the core
 * color arithmetic.
 */
public class ColorMatrixTransform extends PixelwiseFilter {
    private static native long newColorMatrixTransform();
    private native void setFromMatrix(long handle, ColorMatrix matrix);
    private native void assignToMatrix(long handle, ColorMatrix matrix);
    private native void setCoefficients(long handle, int outputChannel, float add, float in0, float in1, float in2, float in3);
    private native void setHSVCorrection(long handle, float Hdegrees, float S, float V);
    private native void setColorInversion(long handle, float preservedD, float preservedG, float preservedB, float S, float V);
    private native void setAllowIntegerApprox(long handle, boolean allow);
    private native boolean allowIntegerApprox(long handle);

    public ColorMatrixTransform(Context context) {
        super(context, newColorMatrixTransform());
    }

    /**
     * Set transformation matrix coefficients
     * @param outputChannel     output channel / matrix column number
     * @param add               additive constant
     * @param in0               first input channel weight
     * @param in1               second input channel weight
     * @param in2               third input channel weight
     * @param in3               fourth input channel weight
     */
    public void setCoefficients(int outputChannel, float add, float in0, float in1, float in2, float in3) {
        setCoefficients(handle, outputChannel, add, in0, in1, in2, in3);
    }

    /**
     * Sets a predefined color matrix from a preset
     * @param preset    the preset to set
     */
    public void setPreset(Preset preset) {
        switch (preset) {
            case IDENTITY:
                setCoefficients(0,0, 1,0,0,0);
                setCoefficients(1,0, 0,1,0,0);
                setCoefficients(2,0, 0,0,1,0);
                setCoefficients(3,0, 0,0,0,1);
                return;
            case GRAYSCALE:
                setCoefficients(0,0, 1.0f/3, 1.0f/3, 1.0f/3, 0);
                setCoefficients(1,0, 1.0f/3, 1.0f/3, 1.0f/3, 0);
                setCoefficients(2,0, 1.0f/3, 1.0f/3, 1.0f/3, 0);
                setCoefficients(3,0, 0,0,0,1);
                return;
            case NEGATIVE:
                setCoefficients(0, 1, -1,0,0,0);
                setCoefficients(1, 1, 0,-1,0,0);
                setCoefficients(2, 1, 0,0,-1,0);
                setCoefficients(3, 1, 0,0,0,1);
                return;
        }
        throw new IllegalArgumentException("Unknown preset: " + preset.toString());
    }


    /**
     * Initializes transform matrix to perform standard HSV correction
     * @param hueOffset         hue offset in degrees
     * @param saturationScale   saturation scale
     * @param valueScale        value scale
     */
    public void setHSVCorrection(float hueOffset, float saturationScale, float valueScale) {
        setHSVCorrection(handle, hueOffset, saturationScale, valueScale);
    }


    /**
     * Initializes transform matrix to perform color inversion with a fixed colors
     * @param preservedColor        a color whose hue specifies a fixed line: all the colors with the same hue are preserved
     * @param saturationScale       saturation scale
     * @param valueScale            value scale
     */
    public void setColorInversion(Color preservedColor, float saturationScale, float valueScale) {
        setColorInversion(handle, preservedColor.r / 255.0f, preservedColor.g / 255.0f, preservedColor.b / 255.0f, saturationScale, valueScale);
    }


    /**
     * Predefined presets
     */
    public enum Preset {
        IDENTITY,       //!< bypass
        GRAYSCALE,      //!< color to gray
        NEGATIVE        //!< color inversion
    }
}
