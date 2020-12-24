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

#pragma once
#include "rendering_context.h"
#include "../exception.h"
#include "../bitmap/abstract_bitmap.h"
#include "../geometry.h"
#include "../utils/lockable_object.h"
#include "../shading/image_shader.h"

#include <string>
#include <vector>


namespace Beatmup {
    class SceneRenderer;

    /**
        An ordered set of layers representing a renderable content.
    */
    class Scene : public LockableObject {
    public:
        class Layer;
    private:
        std::vector<Layer*> layers;		//!< scene layers

        template<class Type> Type& newLayer(const char* name) {
            Type* l = new Type();
            l->setName(name);
            layers.push_back(l);
            return *l;
        }


    public:
        class SceneIntegrityError : public Beatmup::Exception {
        private:
            std::string getSceneLog(const Scene& scene, const std::string prefix, int recursionLeft = 100) const;
        public:
            SceneIntegrityError(const std::string reason, const Scene& scene);
        };


        /**
            Abstract scene layer having name, type, geometry and some content to display.
            The layer geometry is defined by an AffineMapping describing the position and the orientation of the layer content in the rendered image.
        */
        class Layer : public Object {
            friend class SceneRenderer;
            Layer(const Layer&) = delete;		//!< disabling copying constructor
        public:
            enum class Type {
                SceneLayer = 0,			//!< layer containing a scene
                BitmapLayer,			//!< layer displaying a bitmap
                MaskedBitmapLayer,		//!< layer displaying a bitmap with mask
                ShapedBitmapLayer,		//!< layer displaying a bitmap within a shape
                ShadedBitmapLayer		//!< layer displaying a bitmap through a custom fragment shader
            };

        protected:
            Layer(Type type);
            virtual void render(RenderingContext& context) {}

            AffineMapping mapping;		//!< layer mapping
            bool visible;				//!< layer visibility
            bool phantom;				//!< if `true`, layer is ignored by selection by point

        private:
            Type type;
            std::string name;			//!< layer name

        public:
            /**
                \return type of the current layer.
            */
            inline Type getType() const { return type; }

            inline const std::string& getName() const { return name; }
            inline void setName(const char* name) { this->name = name; }

            inline AffineMapping& getMapping() { return mapping; }
            inline const AffineMapping& getMapping() const { return mapping; }
            inline void setMapping(const AffineMapping& mapping) { this->mapping = mapping; }

            /**
                Tests if a given point falls in the layer
            */
            virtual bool testPoint(float x, float y) const;

            /**
                Picks a child layer at given point, if any
            */
            virtual Layer* getChild(float x, float y, unsigned int recursionDepth = 0) const;

            /**
                Returns layer visibility flag
            */
            inline bool isVisible() const { return visible; }

            /**
                Returns `true` if the layer is ignored when searching a layer by point.
            */
            inline bool isPhantom() const { return phantom; }

            /**
                Sets layer visibility
            */
            inline void setVisible(bool visible) { this->visible = visible; }

            /**
                Makes/unmakes the layer "phantom".
                Phantom layers are rendered as usual but not picked when searching a layer by point.
            */
            inline void setPhantom(bool phantom) {	this->phantom = phantom; }

            template<class LayerType> LayerType& castTo() const {
                return (LayerType&)(*this);
            }
        };


        /**
            Layer containing an entire scene
        */
        class SceneLayer : public Layer {
            friend class Scene;
        private:
            const Beatmup::Scene& scene;
            SceneLayer(const Beatmup::Scene& scene);
        public:
            const Beatmup::Scene& getScene() const { return scene; }
            bool testPoint(float x, float y) const;
            Layer* getChild(float x, float y, unsigned int recursionDepth = 0) const;
        };


        /**
            Layer having an image to render.
            The image has a position and orientation with respect to the layer. This is expressed with an affine mapping applied on top of the layer
            mapping.
        */
        class BitmapLayer : public Layer {
            friend class Scene;
            friend class SceneRenderer;

        protected:
            BitmapLayer();
            BitmapLayer(Type type);
            GL::TextureHandler* resolveContent(RenderingContext& context);

            /**
                Configures current rendering program to render this layer
            */
            void configure(RenderingContext& context, GL::TextureHandler* content);
            void render(RenderingContext& context);

            float invAr;					//!< inverse aspect ratio of what is rendered (set by SceneRenderer)
            AbstractBitmap* bitmap;			//!< content to display, used if the image source is set to BITMAP
            AffineMapping bitmapMapping;	//!< bitmap mapping w.r.t. the layer mapping
            color4i modulation;             //!< modulation color

        public:
            bool testPoint(float x, float y) const;

            inline const AbstractBitmap* getBitmap() const { return bitmap; }
            inline void setBitmap(AbstractBitmap* bitmap) { this->bitmap = bitmap; }

            inline AffineMapping& getBitmapMapping() { return bitmapMapping; }
            inline const AffineMapping& getBitmapMapping() const { return bitmapMapping; }
            inline void setBitmapMapping(const AffineMapping& mapping) { this->bitmapMapping = mapping; }

            inline color4i getModulationColor() const { return modulation; }
            inline void setModulationColor(color4i color) { this->modulation = color; }
        };


        /**
            Layer containing a bitmap and a mask applied to the bitmap when rendering.
            Both bitmap and mask have their own positions and orientations relative to the layer's position and orientation.
        */
        class CustomMaskedBitmapLayer : public BitmapLayer {
        protected:
            CustomMaskedBitmapLayer(Type type);
            void configure(RenderingContext& context, GL::TextureHandler* content);

            AffineMapping maskMapping;		//!< mask mapping w.r.t. the layer mapping
            color4i bgColor;				//!< color to fill mask areas where the bitmap is not present
        public:
            inline AffineMapping& getMaskMapping() { return maskMapping; }
            inline const AffineMapping& getMaskMapping() const { return maskMapping; }
            inline void setMaskMapping(const AffineMapping& mapping) { this->maskMapping = mapping; }

            inline color4i getBackgroundColor() const { return bgColor; }
            inline void setBackgroundColor(color4i color) { this->bgColor = color; }
        };


        /**
            Bitmap layer using another bitmap as a mask
        */
        class MaskedBitmapLayer : public CustomMaskedBitmapLayer {
            friend class Scene;
            friend class SceneRenderer;
        private:
            AbstractBitmap* mask;		//!< mask bitmap
        protected:
            MaskedBitmapLayer();
            void render(RenderingContext& context);
        public:
            inline const AbstractBitmap* getMask() const { return mask; }
            inline void setMask(AbstractBitmap* mask) { this->mask = mask; }
            bool testPoint(float x, float y) const;
        };


        /**
            Layer containing a bitmap and a parametric mask (shape)
        */
        class ShapedBitmapLayer : public CustomMaskedBitmapLayer {
            friend class Scene;
            friend class SceneRenderer;
        private:
            float borderWidth;		//!< constant border thickness
            float slopeWidth;		//!< thickness of the smoothed line between border and image
            float cornerRadius;		//!< border corner radius
            bool inPixels;			//!< if `true`, the widths and radius are set in pixels

        protected:
            ShapedBitmapLayer();
            void render(RenderingContext& context);

        public:
            inline float getBorderWidth() const { return borderWidth; }
            inline void setBorderWidth(float borderWidth) { this->borderWidth = borderWidth; }

            inline float getSlopeWidth() const { return slopeWidth; }
            inline void setSlopeWidth(float slopeWidth) { this->slopeWidth = slopeWidth; }

            inline float getCornerRadius() const { return cornerRadius; }
            inline void setCornerRadius(float cornerRadius) { this->cornerRadius = cornerRadius; }

            inline bool getInPixels() const { return inPixels; }
            inline void setInPixels(bool inPixels) { this->inPixels = inPixels; }

            bool testPoint(float x, float y) const;
        };


        /**
            Bitmap layer using a custom shader
        */
        class ShadedBitmapLayer : public BitmapLayer {
            friend class Scene;
            friend class SceneRenderer;
        private:
            ImageShader* shader;
        protected:
            ShadedBitmapLayer();
            void render(RenderingContext& context);
        public:
            inline ImageShader* getShader() const { return shader; }
            inline void setShader(ImageShader* shader) { this->shader = shader; }
        };

    // Scene members
    public:

        Scene();
        ~Scene();

        BitmapLayer& newBitmapLayer(const char* name);
        BitmapLayer& newBitmapLayer();
        MaskedBitmapLayer& newMaskedBitmapLayer(const char* name);
        MaskedBitmapLayer& newMaskedBitmapLayer();
        ShapedBitmapLayer& newShapedBitmapLayer(const char* name);
        ShapedBitmapLayer& newShapedBitmapLayer();
        ShadedBitmapLayer& newShadedBitmapLayer(const char* name);
        ShadedBitmapLayer& newShadedBitmapLayer();

        /**
            Adds a subscene to the current scene.
        */
        SceneLayer& addScene(const Scene& scene);

        /**
            Retrieves a layer by its name or null if not found
        */
        Layer* getLayer(const char* name) const;

        /**
            Retrieves a layer by its index
        */
        Layer& getLayer(int index) const;

        /**
            Retrieves a layer present at a specific point of the scene or null if not found
        */
        Layer* getLayer(float x, float y, unsigned int recursionDepth = 0) const;

        /**
            Returns layer index in the scene or -1 if not found
        */
        int getLayerIndex(const Layer& layer) const;

        /**
            Returns total number of layers in the scene
        */
        int getLayerCount() const;

        /**
            Computes absolute mapping of a given layer with respect to the scene mapping
            \return `true` if the given layer is reached, `false` otherwise (mapping is not changed in this case)
        */
        bool resolveMapping(const Layer& layer, AffineMapping& mapping) const;

        /**
            Attaches an existing layer to the scene
        */
        void attachLayer(Layer& layer);

        /**
            Detaches a layer from the scene
        */
        Layer* detachLayer(int index);
    };

}
