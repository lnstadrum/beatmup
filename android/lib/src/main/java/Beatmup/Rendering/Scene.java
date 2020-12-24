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
import Beatmup.Imaging.Color;
import Beatmup.Geometry.AffineMapping;
import Beatmup.Shading.ImageShader;

/**
 * An ordered set of layers representing a renderable content.
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

    private native void setShadedBitmapLayerShader(long handle, ImageShader shader);

    /**
     * Creates a new scene.
     */
    public Scene() {
        super(newScene());
    }

    /**
     * Creates a new layer containing an entire scene.
     * @param subscene      the scene to bind to the layer
     * @return the new layer
     */
    public SceneLayer newSceneLayer(Scene subscene) {
        return new SceneLayer(this, subscene);
    }

    /**
     * Creates a new scene layer containing a bitmap
     * @return the new layer
     */
    public BitmapLayer newBitmapLayer() {
        return new BitmapLayer(this);
    }

    /**
     * Creates a new scene layer containing a bitmap and applying a bitmap mask to it when rendering.
     * @return the new layer
     */
    public MaskedBitmapLayer newMaskedBitmapLayer() {
        return new MaskedBitmapLayer(this);
    }

    /**
     * Creates a new scene layer containing a bitmap and applying a parametric mask (shape) when rendering.
     * @return the new layer
     */
    public ShapedBitmapLayer newShapedBitmapLayer() {
        return new ShapedBitmapLayer(this);
    }

    /**
     * Creates a new scene layer containing a bitmap and a GLSL fragment shader used in rendering.
     * @return the new layer
     */
    public ShadedBitmapLayer newShadedBitmapLayer() { return new ShadedBitmapLayer(this); }

    /**
     * @return the total number of layers in the scene.
     */
    public int getLayerCount() {
        return getLayerCount(handle);
    }

    /**
     * Returns layer by its index.
     * @param index     zero-based index of a layer in the scene
     * @return the layer or `null` if the index value is out of range.
     */
    public Layer getLayer(int index) {
        return getLayerByIndex(handle, index);
    }

    /**
     * Retrieves a layer by its name.
     * @param name      target layer name
     * @return a layer with the specific name or `null` if not found
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
     * Abstract scene layer having name, type, geometry and some content to display.
     * The layer geometry is defined by an AffineMapping describing the position and the orientation of the layer content in the rendered image.
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

        /**
         * @return horizontal coordinate of the layer with respect to the scene.
         */
        public float getX() {
            return scene.getLayerX(handle);
        }

        /**
         * @return vertical coordinate of the layer with respect to the scene.
         */
        public float getY() {
            return scene.getLayerY(handle);
        }

        /**
         * @return layer orientation in degrees.
         */
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
         * Changes layer position in the scene.
         * @param x     new normalized horizontal position
         * @param y     new normalized vertical position
         */
        public void setPosition(float x, float y) {
            scene.setLayerX(handle, x);
            scene.setLayerY(handle, y);
        }

        /**
         * Changes layer position in the scene by setting its center coordinates.
         * @param x     new normalized horizontal position of the layer center
         * @param y     new normalized vertical position of the layer center
         */
        public void setCenterPosition(float x, float y) {
            scene.setLayerCenterPos(handle, x, y);
        }

        /**
         * Changes horizontal position of the layer
         * @param x     new normalized horizontal position
         */
        public void setX(float x) {
            scene.setLayerX(handle, x);
        }

        /**
         * Changes vertical position of the layer
         * @param y     new normalized vertical position
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
         * @param angle     rotation angle in degrees
         */
        public void rotate(float angle) {
            scene.rotateLayer(handle, angle, 0, 0);
        }

        /**
         * Assigns a given affine mapping to the layer mapping.
         * @param mapping       the mapping
         */
        public void setTransform(AffineMapping mapping) {
            scene.setLayerTransform(handle, mapping.x, mapping.y, mapping.a11, mapping.a12, mapping.a21, mapping.a22);
        }

        /**
         * @return the layer mapping with respect to the scene
         */
        public AffineMapping getTransform() {
            AffineMapping mapping = new AffineMapping();
            scene.getLayerTransform(handle, mapping);
            return mapping;
        }

        /**
         * Assigns the layer mapping with respect to the scene to a given mapping object.
         * @param mapping   The mapping
         */
        public void getTransform(AffineMapping mapping) {
            scene.getLayerTransform(handle, mapping);
        }

        /**
         * Manages the layer visibility in the rendered scene.
         * @param visible   if `true`, the layer is rendered, otherwise it is ignored
         */
        public void setVisibility(boolean visible) {
            scene.setLayerVisibility(handle, visible);
        }

        /**
         * @return `true` if the layer is visible.
         */
        public boolean getVisibility() {
            return scene.getLayerVisibility(handle);
        }

        /**
         * Hides the layer.
         */
        public void hide() { setVisibility(false); }

        /**
         * Makes the layer visible.
         */
        public void show() { setVisibility(true); }

        /**
         * Makes/unmakes the layer "phantom".
         * Phantom layers are rendered as usual but not picked when searching a layer by point.
         * @param phantom       if `true`, the layer goes "phantom"
         */
        public void setPhantom(boolean phantom) {
            scene.setLayerPhantomFlag(handle, phantom);
        }

        /**
         * @return `true` if the layer is "phantom".
         */
        public boolean getPhantom() {
            return scene.getLayerPhantomFlag(handle);
        }
    }


    /**
     * Layer having an image to render.
     * The image has a position and orientation with respect to the layer. This is expressed with an affine mapping applied on top of the layer mapping.
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

        /**
         * Retrieves the image transform in layer coordinates.
         * @param mapping       The mapping object to update with the image transform
         */
        public void getImageTransform(AffineMapping mapping) {
            scene.getBitmapLayerImageTransform(handle, mapping);
        }

        /**
         * Sets the image modulation color.
         * The color multiplies in component-wise fashion the image colors when rendering.
         * @param color     The new modulation color
         */
        public void setModulationColor(Color color) {
            scene.setBitmapLayerModulationColor(handle, color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f);
        }

        /**
         * @return the modulation color.
         */
        public Color getModulationColor() {
            Color color = new Color();
            scene.getBitmapLayerModulationColor(handle, color);
            return color;
        }

        /**
         * Sets the image modulation color to enable transparency effect.
         * @param transparency      The transparency value; 0 for fully transparent picture, 1 for the full opacity.
         */
        public void setTransparency(float transparency) {
            scene.setBitmapLayerModulationColor(handle, 1.0f, 1.0f, 1.0f, transparency);
        }
    }


    /**
     * Layer containing a bitmap and a mask applied to the bitmap when rendering.
     * Both bitmap and mask have their own positions and orientations relative to the layer's position and orientation.
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
     * Bitmap layer using another bitmap as a mask
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
     * Layer containing a bitmap and a parametric mask (shape)
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
        private ImageShader shader;

        private ShadedBitmapLayer(Scene scene) {
            super(scene);
            handle = scene.newShadedBitmapLayer(scene.handle, this);
        }

        public void setShader(ImageShader shader) {
            this.shader = shader;
            scene.setShadedBitmapLayerShader(handle, shader);
        }

        public ImageShader getShader() {
            return shader;
        }
    }


    /**
     * Layer containing an entire scene
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