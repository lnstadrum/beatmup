/*
    Beatmup image and signal processing library
    Copyright (C) 2020, lnstadrum

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

import Beatmup.Context;
import Beatmup.Utils.VariablesBundle;

/**
 * A GLSL program to process images
 */
public class ImageShader extends VariablesBundle {
    public static String
            INPUT_IMAGE_ID        = getInputImageId(),              //!< main input image sampler variable name
            INPUT_IMAGE_DECL_TYPE = getInputImageDeclType();        //!< main input image sampler variable type

    // native methods
    private native static long newImageShader(Context context);
    private native void setSourceCode(long handle, String code);
    private static native String getInputImageId();
    private static native String getInputImageDeclType();

    /**
     * Constructs a new image shader.
     * @param context   A Beatmup context instance
     */
    public ImageShader(Context context) {
        super(newImageShader(context));
    }

    /**
     * Sets or updates GLSL fragment source code.
     * The main input image is expected to be bound to a sampler variable named {@link #INPUT_IMAGE_ID} of type {@link #INPUT_IMAGE_DECL_TYPE}.
     * @param code      The GLSL source code
     */
    public void setSourceCode(String code) {
        setSourceCode(handle, code);
    }
}
