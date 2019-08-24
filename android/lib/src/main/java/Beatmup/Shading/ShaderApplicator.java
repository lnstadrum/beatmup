package Beatmup.Shading;

import Beatmup.Bitmap;
import Beatmup.Context;
import Beatmup.Task;

/**
 * Lightweight task applying a layer shader to a bitmap
 */
public class ShaderApplicator extends Task {
    Bitmap input, output;
    Shader shader;

    private static native long newShaderApplicator();
    private native void setInput(long handle, Bitmap bitmap);
    private native void setOutput(long handle, Bitmap bitmap);
    private native void setLayerShader(long handle, Shader shader);

    public ShaderApplicator(Context context) {
        super(context, newShaderApplicator());
    }

    public void setShader(Shader shader) {
        this.shader = shader;
        setLayerShader(handle, shader);
    }

    public Shader getShader() {
        return shader;
    }

    public Bitmap getInput() {
        return input;
    }

    public void setInput(Bitmap input) {
        this.input = input;
        setInput(handle, input);
    }

    public Bitmap getOutput() {
        return output;
    }

    public void setOutput(Bitmap output) {
        this.output = output;
        setOutput(handle, output);
    }
}
