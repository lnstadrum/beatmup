package Beatmup.Rendering;

import Beatmup.Bitmap;
import Beatmup.Imaging.Color;
import Beatmup.Geometry.AffineMapping;
import Beatmup.Shading.Shader;

/**
 * Scene representation
 */
public class Scene extends Beatmup.Object {
    private static native long newScene();

    private native long newSceneLayer(long handle, SceneLayer object, Scene subscene);
    private native long newBitmapLayer(long handle, BitmapLayer object);
    private native long newMaskedBitmapLayer(long handle, MaskedBitmapLayer object);
    private native long newShapedBitmapLayer(long handle, ShapedBitmapLayer object);
    private native long newShadedBitmapLayer(long handle, ShadedBitmapLayer object);

    private native void deleteLayers();

    private native int getLayerCount(long handle);
    private native Layer getLayerByIndex(long handle, int index);
    private native Layer getLayerByName(long handle, String name);
    private native Layer getLayerAtPoint(long handle, float x, float y);

    private native void setLayerName(long handle, String newName);
    private native String getLayerName(long handle);

    private native void setLayerVisibility(long handle, boolean visible);
    private native boolean getLayerVisibility(long handle);

    private native void setLayerPhantomFlag(long handle, boolean phantom);
    private native boolean getLayerPhantomFlag(long handle);

    private native void setLayerTransform(long handle, float x, float y, float a11, float a12, float a21, float a22);
    private native void getLayerTransform(long handle, AffineMapping target);

    private native void setLayerX(long handle, float x);
    private native float getLayerX(long handle);

    private native void setLayerY(long handle, float y);
    private native float getLayerY(long handle);

    private native float getLayerScaleX(long handle);
    private native float getLayerScaleY(long handle);
    private native float getLayerOrientation(long handle);

    private native void setLayerCenterPos(long handle, float x, float y);
    private native void scaleLayer(long handle, float factor, float fixX, float fixY);
    private native void rotateLayer(long handle, float angleDegrees, float fixX, float fixY);

    private native void setBitmapLayerModulationColor(long handle, float r, float g, float b, float a);
    private native void getBitmapLayerModulationColor(long handle, Color color);

    private native void setBitmapLayerMaskPos(long handle, float x, float y);
    private native void scaleBitmapLayerMask(long handle, float x, float y);
    private native void rotateBitmapLayerMask(long handle, float angleDegrees);
    private native void skewBitmapLayerMask(long handle, float x, float y);

    private native void setBitmapLayerBgColor(long handle, float r, float g, float b, float a);
    private native void getBitmapLayerBgColor(long handle, Color color);

    private native void setBitmapLayerImageTransform(long handle, float x, float y, float a11, float a12, float a21, float a22);
    private native void getBitmapLayerImageTransform(long handle, AffineMapping target);

    private native void setBitmapLayerMaskTransform(long handle, float x, float y, float a11, float a12, float a21, float a22);
    private native void getBitmapLayerMaskTransform(long handle, AffineMapping target);

    private native void setBitmapLayerBitmap(long handle, Bitmap bitmap);

    private native void setMaskedBitmapLayerMask(long handle, Bitmap mask);

    private native void setShapedBitmapLayerInPixelsSwitch(long handle, boolean inPixels);
    private native boolean getShapedBitmapLayerInPixelsSwitch(long handle);

    private native void setShapedBitmapLayerCornerRadius(long handle, float radius);
    private native float getShapedBitmapLayerCornerRadius(long handle);

    private native void setShapedBitmapLayerBorderWidth(long handle, float thickness);
    private native float getShapedBitmapLayerBorderWidth(long handle);

    private native void setShapedBitmapLayerSlopeWidth(long handle, float thickness);
    private native float getShapedBitmapLayerSlopeWidth(long handle);

    private native void setShadedBitmapLayerShader(long handle, Shader shader);

    /**
     * Creates new scene instance
     */
    public Scene() {
        super(newScene());
    }

    /**
     * Creates a new layer containing a whole scene
     * @param subscene      the scene to bind to the layer
     * @return the new layer
     */
    public SceneLayer newSceneLayer(Scene subscene) {
        return new SceneLayer(this, subscene);
    }

    /**
     * Creates new scene layer that will contain a bitmap
     * @return the new layer
     */
    public BitmapLayer newBitmapLayer() {
        return new BitmapLayer(this);
    }

    /**
     * Creates new scene layer that will contain a bitmap and apply a mask to it
     * @return the new layer
     */
    public MaskedBitmapLayer newMaskedBitmapLayer() {
        return new MaskedBitmapLayer(this);
    }

    /**
     * Creates new scene layer that will contain a bitmap and apply a parametric mask (shape)
     * @return the new layer
     */
    public ShapedBitmapLayer newShapedBitmapLayer() {
        return new ShapedBitmapLayer(this);
    }

    public ShadedBitmapLayer newShadedBitmapLayer() { return new ShadedBitmapLayer(this); }

    /**
     * @return the nubmer of layers in the scene
     */
    public int getLayerCount() {
        return getLayerCount(handle);
    }

    /**
     * Returns layer by its index
     * @param index     index (number) of a layer in the scene
     * @return the layer or `null` in case of incorrect index value
     */
    public Layer getLayer(int index) {
        return getLayerByIndex(handle, index);
    }

    /**
     * Retrieves a layer by its name
     * @param name      target layer name
     * @return a layer having given name or `null` if not found
     */
    public Layer getLayer(String name) {
        return getLayerByName(handle, name);
    }

    /**
     * Retrieves a layer at a given point in the scene coordinate space.
     * @param x     Point X coordinate
     * @param y     Point Y coordinate
     * @return a reference to the layer or `null` if not found
     */
    public Layer getLayer(float x, float y) {
        return getLayerAtPoint(handle, x, y);
    }

    /**
     * Basic scene layer
     */
    public static class Layer {
        protected Scene scene;
        protected long handle;

        protected Layer(Scene scene) {
            this.scene = scene;
        }

        @Override
        public boolean equals(Object obj) {
            return (obj instanceof Layer) && (handle == ((Layer) obj).handle);
        }

        /**
         * @return layer name
         */
        public String getName() {
            return scene.getLayerName(handle);
        }

        public float getX() {
            return scene.getLayerX(handle);
        }

        public float getY() {
            return scene.getLayerY(handle);
        }

        public float getScaleX() {
            return scene.getLayerScaleX(handle);
        }

        public float getScaleY() {
            return scene.getLayerScaleY(handle);
        }

        public float getOrientation() {
            return scene.getLayerOrientation(handle);
        }

        /**
         * Renames the layer
         * @param newName   new layer name
         */
        public void setName(String newName) {
            scene.setLayerName(handle, newName);
        }

        /**
         * Changes layer position in the scene
         * @param x     new horizontal position (0 for leftmost, 1 for rightmost scene points)
         * @param y     new vertical position (0 for topmost scene points)
         */
        public void setPosition(float x, float y) {
            scene.setLayerX(handle, x);
            scene.setLayerY(handle, y);
        }

        /**
         * Changes layer position in the scene
         * @param x     new horizontal position of the layer center (0 for leftmost, 1 for rightmost scene points)
         * @param y     new vertical position of the layer center (0 for topmost scene points)
         */
        public void setCenterPosition(float x, float y) {
            scene.setLayerCenterPos(handle, x, y);
        }

        /**
         * Changes horizontal position of the layer
         * @param x     new horizontal position (0 for leftmost, 1 for rightmost scene points)
         */
        public void setX(float x) {
            scene.setLayerX(handle, x);
        }

        /**
         * Changes vertical position of the layer
         * @param y     new vertical position (0 for topmost scene points)
         */
        public void setY(float y) {
            scene.setLayerY(handle, y);
        }

        /**
         * Changes layer scale
         * @param factor     layer scale
         */
        public void scale(float factor) {
            scene.scaleLayer(handle, factor, 0, 0);
        }

        /**
         * Rotates the layer
         * @param angle     rotation angle relatively to the current layer orientation in degrees
         */
        public void rotate(float angle) {
            scene.rotateLayer(handle, angle, 0, 0);
        }

        /**
         * Assigns an affine mapping to the layer
         * @param mapping       the mapping
         */
        public void setTransform(AffineMapping mapping) {
            scene.setLayerTransform(handle, mapping.x, mapping.y, mapping.a11, mapping.a12, mapping.a21, mapping.a22);
        }

        /**
         * @return layer mapping w.r.t. scene
         */
        public AffineMapping getTransform() {
            AffineMapping mapping = new AffineMapping();
            scene.getLayerTransform(handle, mapping);
            return mapping;
        }

        public void getTransform(AffineMapping mapping) {
            scene.getLayerTransform(handle, mapping);
        }

        /**
         * Manages layer visibility when rendering
         * @param visible   if `true`, the layer is rendered, otherwise ignored
         */
        public void setVisibility(boolean visible) {
            scene.setLayerVisibility(handle, visible);
        }

        public boolean getVisibility() {
            return scene.getLayerVisibility(handle);
        }

        public void hide() { setVisibility(false); }
        public void show() { setVisibility(true); }

        /**
         * Enables/disables phantom layer mode
         * @param phantom       if `true`, layer is invisible on layer selection by point
         */
        public void setPhantom(boolean phantom) {
            scene.setLayerPhantomFlag(handle, phantom);
        }

        /**
         * @return `true` if the phantom mode is enabled
         */
        public boolean getPhantom() {
            return scene.getLayerPhantomFlag(handle);
        }
    }


    /**
     * Simple bitmap scene layer with no mask
     */
    public static class BitmapLayer extends Layer {

        private Bitmap bitmap;

        private BitmapLayer(Scene scene) {
            super(scene);
            handle = scene.newBitmapLayer(scene.handle, this);
        }

        /**
         * Sets a bitmap to render in this layer
         * @param bitmap    the new bitmap
         */
        public void setBitmap(Bitmap bitmap) {
            this.bitmap = bitmap;
            scene.setBitmapLayerBitmap(handle, bitmap);
        }

        /**
         * Retrieves the bitmap held in this layer to render
         * @return      the bitmap or `null` if not set
         */
        public Bitmap getBitmap() {
            return bitmap;
        }

        /**
         * Safely recycles the layer bitmap
         */
        public synchronized void recycleBitmap() {
            scene.setBitmapLayerBitmap(handle, null);
            Bitmap buffer = bitmap;
            bitmap = null;
            Bitmap.recycle(buffer);
        }

        /**
         * Sets new image transform with respect to the layer
         * @param mapping       the new transform
         */
        public void setImageTransform(AffineMapping mapping) {
            scene.setBitmapLayerImageTransform(handle, mapping.x, mapping.y, mapping.a11, mapping.a12, mapping.a21, mapping.a22);
        }

        /**
         * @return image mapping with respect to the layer
         */
        public AffineMapping getImageTransform() {
            AffineMapping mapping = new AffineMapping();
            scene.getBitmapLayerImageTransform(handle, mapping);
            return mapping;
        }

        public void getImageTransform(AffineMapping mapping) {
            scene.getBitmapLayerImageTransform(handle, mapping);
        }

        public void setModulationColor(Color color) {
            scene.setBitmapLayerModulationColor(handle, color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f);
        }

        public Color getModulationColor() {
            Color color = new Color();
            scene.getBitmapLayerModulationColor(handle, color);
            return color;
        }

        public void setTransparency(float transparency) {
            scene.setBitmapLayerModulationColor(handle, 1.0f, 1.0f, 1.0f, transparency);
        }
    }


    /**
     * Custom masked bitmap layer
     */
    public static class CustomMaskedBitmapLayer extends BitmapLayer {
        private CustomMaskedBitmapLayer(Scene scene) {
            super(scene);
        }

        public void setMaskPosition(float x, float y) {
            scene.setBitmapLayerMaskPos(handle, x, y);
        }

        public void rotateMask(float angleDegrees) {
            scene.rotateBitmapLayerMask(handle, angleDegrees);
        }

        public void scaleMask(float x, float y) {
            scene.scaleBitmapLayerMask(handle, x, y);
        }

        public void skewMask(float xDegrees, float yDegrees) {
            scene.skewBitmapLayerMask(handle, xDegrees, yDegrees);
        }

        /**
         * Sets new mask transform with respect to the layer
         * @param mapping       the new transform
         */
        public void setMaskTransform(AffineMapping mapping) {
            scene.setBitmapLayerMaskTransform(handle, mapping.x, mapping.y, mapping.a11, mapping.a12, mapping.a21, mapping.a22);
        }

        /**
         * @return mask mapping with respect to the layer
         */
        public AffineMapping getMaskTransform() {
            AffineMapping mapping = new AffineMapping();
            scene.getBitmapLayerMaskTransform(handle, mapping);
            return mapping;
        }

        public void getMaskTransform(AffineMapping mapping) {
            scene.getBitmapLayerMaskTransform(handle, mapping);
        }

        public void setBackgroundColor(Color color) {
            scene.setBitmapLayerBgColor(handle, color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
        }

        public Color getBackgroundColor() {
            Color color = new Color();
            scene.getBitmapLayerBgColor(handle, color);
            return color;
        }
    }


    /**
     * Bitmap scene layer that uses another bitmap as a mask
     */
    public static class MaskedBitmapLayer extends CustomMaskedBitmapLayer {
        private Bitmap mask;

        private MaskedBitmapLayer(Scene scene) {
            super(scene);
            handle = scene.newMaskedBitmapLayer(scene.handle, this);
        }

        public void setMask(Bitmap mask) {
            this.mask = mask;
            scene.setMaskedBitmapLayerMask(handle, mask);
        }

        public Bitmap getMask() {
            return mask;
        }
    }


    /**
     * Bitmap scene layer having parametric mask (a shape)
     */
    public static class ShapedBitmapLayer extends CustomMaskedBitmapLayer {
        private ShapedBitmapLayer(Scene scene) {
            super(scene);
            handle = scene.newShapedBitmapLayer(scene.handle, this);
        }

        public void setBorderWidth(float width) {
            scene.setShapedBitmapLayerBorderWidth(handle, width);
        }

        public float getBorderWidth() {
            return scene.getShapedBitmapLayerBorderWidth(handle);
        }

        public void setSlopeWidth(float width) {
            scene.setShapedBitmapLayerSlopeWidth(handle, width);
        }

        public float getSlopeWidth() {
            return scene.getShapedBitmapLayerSlopeWidth(handle);
        }

        public void setCornerRadius(float radius) {
            scene.setShapedBitmapLayerCornerRadius(handle, radius);
        }

        public float getCornerRadius() {
            return scene.getShapedBitmapLayerCornerRadius(handle);
        }

        public void setInPixels(boolean inPixels) {
            scene.setShapedBitmapLayerInPixelsSwitch(handle, inPixels);
        }

        public boolean getInPixels() {
            return scene.getShapedBitmapLayerInPixelsSwitch(handle);
        }
    }

    /**
     * Bitmap layer using a custom shader
     */
    public static class ShadedBitmapLayer extends BitmapLayer {
        private Shader shader;

        private ShadedBitmapLayer(Scene scene) {
            super(scene);
            handle = scene.newShadedBitmapLayer(scene.handle, this);
        }

        public void setShader(Shader shader) {
            this.shader = shader;
            scene.setShadedBitmapLayerShader(handle, shader);
        }

        public Shader getShader() {
            return shader;
        }
    }

    /**
     * Layer containing a scene
     */
    public static class SceneLayer extends Layer {
        private Scene subscene;
        private SceneLayer(Scene scene, Scene subscene) {
            super(scene);
            this.subscene = subscene;
            handle = scene.newSceneLayer(scene.handle, this, subscene);
        }

        public Scene getScene() {
            return subscene;
        }
    }

    @Override
    protected void finalize() throws Throwable {
        deleteLayers();
        super.finalize();
    }
}