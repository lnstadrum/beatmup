package Beatmup.Shading;

import Beatmup.*;
import Beatmup.Utils.VariablesBundle;

/**
 * Custom pixel shader
 */
public class Shader extends VariablesBundle {
    private native static long newShader(Context context);
    private native void setSourceCode(long handle, String code);

    public Shader(Context context) {
        super(newShader(context));
    }


    public void setSourceCode(String code) {
        setSourceCode(handle, code);
    }
}
