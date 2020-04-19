package Beatmup.Shading;

import Beatmup.*;
import Beatmup.Utils.VariablesBundle;

/**
 * Custom pixel shader
 */
public class Shader extends VariablesBundle {
    public static String
            INPUT_IMAGE_ID        = getInputImageId(),
            INPUT_IMAGE_DECL_TYPE = getInputImageDeclType();

    private native static long newShader(Context context);
    private native void setSourceCode(long handle, String code);
    private static native String getInputImageId();
    private static native String getInputImageDeclType();

    public Shader(Context context) {
        super(newShader(context));
    }


    public void setSourceCode(String code) {
        setSourceCode(handle, code);
    }
}
