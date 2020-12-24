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

package Beatmup.Shading;

import android.util.ArrayMap;

import java.util.Map;

import Beatmup.Bitmap;
import Beatmup.Context;
import Beatmup.Task;

/**
 * A Task applying an image shader to bitmaps
 */
public class ShaderApplicator extends Task {
    private Map<String, Bitmap> samplers;
    private Bitmap output;
    private ImageShader shader;

    private static native long newShaderApplicator();
    private native void addSampler(long handle, Bitmap bitmap, String uniformName);
    private native void setOutput(long handle, Bitmap bitmap);
    private native void setShader(long handle, ImageShader shader);

    /**
     * Creates a new shader applicator.
     * @param context   A Beatmup context instance
     */
    public ShaderApplicator(Context context) {
        super(context, newShaderApplicator());
        samplers = new ArrayMap<>();
    }

    /**
     * Attaches an ImageShader to the applicator.
     * @param shader    The shader
     */
    public void setShader(ImageShader shader) {
        this.shader = shader;
        setShader(handle, shader);
    }

    /**
     * @return currently used ImageShader or null of not set
     */
    public ImageShader getShader() {
        return shader;
    }

    /**
     * Adds an image to sample.
     * @param input     The image
     * @param name      GLSL sample variable name the image is bound to
     */
    public void addSampler(Bitmap input, String name) {
        this.samplers.put(name, input);
        addSampler(handle, input, name);
    }

    /**
     * Adds the main image.
     * The image is bound to {@link ImageShader#INPUT_IMAGE_ID) variable} which supports {@link Beatmup.Android.ExternalBitmap} textures to be used directly.
     * @param input     The image
     */
    public void addSampler(Bitmap input) {
        addSampler(input, ImageShader.INPUT_IMAGE_ID);
    }

    /**
     * Sets the output image.
     * It stores the shader output result after the ShaderApplicator task is run.
     * @param output    The image
     */
    public void setOutput(Bitmap output) {
        this.output = output;
        setOutput(handle, output);
    }

    /**
     * @return the output image or null if not set.
     */
    public Bitmap getOutput() {
        return output;
    }
}
