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

package Beatmup.Rendering;

import Beatmup.Bitmap;
import Beatmup.Context;
import Beatmup.Exceptions.CoreException;
import Beatmup.Task;

/**
 * Task rendering a Scene.
 * The rendering may be done to a given bitmap or on screen (default).
 * When rendering on screen, the image is sent to BasicDisplay bound to the Context running the rendering task.
 */
public class SceneRenderer extends Task {
    // references to protect things of being garbage-collected while they are actually used in the native code
    private Scene scene;
    private Bitmap background;
    private Bitmap output;

    // native methods
    private static native long newSceneRenderer(Context ctx);
    private native void setOutput(long handle, Bitmap bitmap);
    private native void resetOutput(long handle);
    private native void setScene(long handle, Scene scene);
    private native void setOutputMapping(long handle, int mapping);
    private native int getOutputMapping(long handle);
    private native void setOutputReferenceWidth(long handle, int width);
    private native int getOutputReferenceWidth(long handle);
    private native void setOutputPixelsFetching(long handle, boolean fetch);
    private native boolean getOutputPixelsFetching(long handle);
    private native void setBackgroundBitmap(long handle, Bitmap bitmap);
    private native Scene.Layer pickLayer(long handle, float x, float y, boolean inPixels);

    /**
     * Scene coordinates to output (screen or bitmap) pixel coordinates mapping
     */
    public enum OutputMapping {
        STRETCH,            //!< output viewport covers entirely the scene axis span, aspect ratio is not preserved in general
        FIT_WIDTH_TO_TOP,   //!< width is covered entirely, height is resized to keep aspect ratio, the top borders are aligned
        FIT_WIDTH,          //!< width is covered entirely, height is resized to keep aspect ratio, point (0.5, 0.5) is mapped to the output center
        FIT_HEIGHT          //!< height is covered entirely, width is resized to keep aspect ratio, point (0.5, 0.5) is mapped to the output center
    }


    /**
     * Creates new scene renderer
     * @param context Beatmup core context
     */
    public SceneRenderer(Context context) {
        super(context, newSceneRenderer(context));
    }

    /**
     * Attaches a bitmap to the renderer output.
     * This disables the on-screen rendering.
     * @param bitmap    the bitmap the scene will be rendered to
     */
    public void setOutput(Bitmap bitmap) {
        setOutput(handle, bitmap);
        this.output = bitmap;
    }

    /**
     * Removes bitmap from the renderer output and switches to on-screen rendering.
     * The rendering is done on the display currently connected to the Context running the rendering task.
     */
    public void resetOutput() {
        this.output = null;
        resetOutput(handle);
    }

    /**
     * @return a bitmap bound to the renderer or `null`.
     */
    public Bitmap getOutput() {
        return output;
    }

    /**
     * Sets a scene to be rendered
     * @param scene     the scene
     */
    public void setScene(Scene scene) {
        setScene(handle, scene);
        this.scene = scene;
    }

    /**
     * @return a scene used by the renderer or null.
     */
    public Scene getScene() {
        return this.scene;
    }

    /**
     * Sets the output mapping specifying how the scene coordinates [0,1]² are mapped to the output (screen or bitmap) pixel coordinates.
     * @param mapping the new mapping
     */
    public void setOutputMapping(OutputMapping mapping) {
        setOutputMapping(handle, mapping.ordinal());
    }

    /**
     * @return the output mapping specifying how the scene coordinates [0,1]² are mapped to the output (screen or bitmap) pixel coordinates.
     */
    public OutputMapping getOutputMapping() {
        return OutputMapping.values()[ getOutputMapping(handle) ];
    }

    /**
     * Sets a value overriding output width for elements that have their size in pixels, in order to render a resolution-independent picture.
     * If set negative or zero, the actual output width is taken.
     * @param newWidth the new reference width value
     */
    public void setOutputReferenceWidth(int newWidth) {
        setOutputReferenceWidth(handle, newWidth);
    }

    /**
     * @return Retrieves value overriding output width for elements that have their size in pixels, in order to render a resolution-independent picture
     */
    public int getOutputReferenceWidth() {
        return getOutputReferenceWidth(handle);
    }

    /**
     * Sets an image to pave the background.
     * @param bitmap    the image
     */
    public void setBackground(Bitmap bitmap) {
        this.background = bitmap;
        if (bitmap != null)
            setBackgroundBitmap(handle, bitmap);
        else
            setBackgroundBitmap(handle, null);
    }

    /**
     * @return the bitmap currently set to fill the background, null if not set.
     */
    public Bitmap getBackground() {
        return background;
    }

    /**
     * Specifies whether the output image data is pulled from GPU to CPU memory every time the rendering is done.
     * This is convenient if the rendered image is an application output result, and is further stored or sent through the network.
     * Otherwise, if the image is to be further processed inside %Beatmup, the pixel transfer likely introduces an unnecessary latency and may cause
     * FPS drop in real-time rendering.
     * Has no effect in on-screen rendering.
     * @param fetch     If `true`, pixels are pulled to CPU memory.
     */
    public void setOutputPixelsFetching(boolean fetch) {
        setOutputPixelsFetching(handle, fetch);
    }

    public boolean getOutputPixelFetching() {
        return getOutputPixelsFetching(handle);
    }

    /**
     * Renders the scene
     * @return rendering time in ms
     */
    public float render() throws CoreException {
        return context.performTask(this);
    }

    /**
     * Retrieves a scene layer visible at a given point, if any.
     * In contrast to Scene::getLayer() it takes into account the output mapping.
     * "Phantom" layers are ignored.
     * @param x             X coordinate.
     * @param y             Y coordinate.
     * @param inPixels      If `true`, the coordinates taken in pixels.
     * @return the topmost layer at the given point if any, null otherwise.
     */
    public Scene.Layer pickLayer(float x, float y, boolean inPixels) {
        return pickLayer(handle, x, y, inPixels);
    }

}
