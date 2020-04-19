package Beatmup.Shading;

import android.util.ArrayMap;

import java.util.Map;

import Beatmup.Bitmap;
import Beatmup.Context;
import Beatmup.Task;

/**
 * Lightweight task applying a layer shader to a bitmap
 */
public class ShaderApplicator extends Task {
    Map<String, Bitmap> samplers;
    Bitmap output;
    Shader shader;

    private static native long newShaderApplicator();
    private native void addSampler(long handle, Bitmap bitmap, String uniformName);
    private native void setOutput(long handle, Bitmap bitmap);
    private native void setShader(long handle, Shader shader);

    public ShaderApplicator(Context context) {
        super(context, newShaderApplicator());
        samplers = new ArrayMap<>();
    }

    public void setShader(Shader shader) {
        this.shader = shader;
        setShader(handle, shader);
    }

    public Shader getShader() {
        return shader;
    }

    public void addSampler(Bitmap input, String name) {
        this.samplers.put(name, input);
        addSampler(handle, input, name);
    }

    public void addSampler(Bitmap input) {
        addSampler(input, Shader.INPUT_IMAGE_ID);
    }

    public Bitmap getOutput() {
        return output;
    }

    public void setOutput(Bitmap output) {
        this.output = output;
        setOutput(handle, output);
    }
}
