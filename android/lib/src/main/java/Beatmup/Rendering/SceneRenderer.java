package Beatmup.Rendering;

import Beatmup.Bitmap;
import Beatmup.Context;
import Beatmup.Task;

/**
 * Renderer: performs rendering of a given scene to a given bitmap
 */
public class SceneRenderer extends Task {
    // references to avoid GC
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
    private native Scene.Layer pickLayer(long handle, float x, float y, boolean normalized);

    /**
     * Scene coordinates to output (screen or bitmap) mapping
     */
    public enum OutputMapping {
        STRETCH,            //!< output viewport covers entirely the scene axis span
        FIT_WIDTH_TO_TOP,	//!< width is covered entirely, height is resized to keep aspect ratio, the top borders are aligned
        FIT_WIDTH,			//!< width is covered entirely, height is resized to keep aspect ratio, point (0.5, 0.5) is mapped to the output center
        FIT_HEIGHT			//!< height is covered entirely, width is resized to keep aspect ratio, point (0.5, 0.5) is mapped to the output center
    }


    /**
     * Creates new scene renderer
     * @param context Beatmup core context
     */
    public SceneRenderer(Context context) {
        super(context, newSceneRenderer(context));
    }

    /**
     * Attaches a bitmap to the renderer output
     * @param bitmap    the bitmap the scene will be rendered to
     */
    public void setOutput(Bitmap bitmap) {
        setOutput(handle, bitmap);
        this.output = bitmap;
    }

    /**
     * Removes bitmap from the renderer output and switches to on-screen rendering
     */
    public void resetOutput() {
        this.output = null;
        resetOutput(handle);
    }


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
     * @return a scene given to this renderer instance
     */
    public Scene getScene() {
        return this.scene;
    }

    /**
     * Sets the output mapping specifying how the scene coordinates [0,1]² are mapped to the output (screen or bitmap)
     * @param mapping the new mapping
     */
    public void setOutputMapping(OutputMapping mapping) {
        setOutputMapping(handle, mapping.ordinal());
    }

    /**
     * @return the output mapping specifying how the scene coordinates [0,1]² are mapped to the output (screen or bitmap)
     */
    public OutputMapping getOutputMapping() {
        return OutputMapping.values()[ getOutputMapping(handle) ];
    }

    /**
     * Sets a value overriding output width for elements that have their size in pixels, in order to render a resolution-independent picture
     * If set negative or zero, the actual output width is taken
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
     * Sets a bitmap filling the background when rendering on screen
     * @param bitmap    the bitmap
     */
    public void setBackground(Bitmap bitmap) {
        this.background = bitmap;
        if (bitmap != null)
            setBackgroundBitmap(handle, bitmap);
        else
            setBackgroundBitmap(handle, null);
    }

    /**
     * @return the bitmap currently set as the screen background, null if not
     */
    public Bitmap getBackground() {
        return background;
    }

    /**
     * Specifies whether the output bitmap should be fetched from GPU to CPU RAM every time
     * the rendering is done.
     * @param fetch     if `true`, do fetch
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
    public float render() {
        return context.performTask(this);
    }

    /**
     * Retrieves a scene layer at a given point, if any.
     * @param x             X coordinate.
     * @param y             Y coordinate.
     * @param normalized    If `true`, the coordinates are normalized to [0..1)*[0..h/w) range.
     *                      Otherwise they are interpreted in pixels.
     * @return the topmost layer at the given point if any, null otherwise.
     */
    public Scene.Layer pickLayer(float x, float y, boolean normalized) {
        return pickLayer(handle, x, y, normalized);
    }

    public void repeatRender(boolean abortCurrent) {
        context.repeatTask(this, abortCurrent);
    }

}
