/*
    Scene representation
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
        A scene (ordered set of layers)
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
            Abstract scene layer
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
                Returns layer type
            */
            inline Type getType() const { return type; }

            inline const char* getName() const { return name.c_str(); }
            inline void setName(const char* name) { this->name = name; }

            inline AffineMapping& getMapping() { return mapping; }
            inline const AffineMapping& getMapping() const { return mapping; }
            inline void setMapping(const AffineMapping& mapping) { this->mapping = mapping; }

            /**
                Tests if a point is on this layer
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
                Makes/unmakes layer "phantom" (i.e. whether it should not be picked when selecting layer by point).
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
            Layer having a bitmap
        */
        class BitmapLayer : public Layer {
            friend class Scene;
            friend class SceneRenderer;
        public:
            /**
                Specifies for a given scene layer where the image it displays comes from
            */

        protected:
            BitmapLayer();
            BitmapLayer(Type type);
            GL::TextureHandler* resolveContent(RenderingContext& context);

            /**
                Configures current rendering program to render this layer
            */
            void configure(RenderingContext& context, GL::TextureHandler* content);
            void render(RenderingContext& context);

            float invAr;					//!< inversed aspect ratio of what is rendered (set by SceneRenderer)
            AbstractBitmap* bitmap;				//!< content to display, used if the image source is set to BITMAP
            AffineMapping bitmapMapping;	//!< bitmap mapping w.r.t. the layer mapping
            color4i modulation;               //!< modulation color

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
            Layer containing a bitmap cropped by a mask
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
            Layer containing a bitmap cropped by a bitmap mask
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
            Layer containing a bitmap cropped by a shape mask
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
            Custom-shaded bitmap layer
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
            inline ImageShader* getLayerShader() const { return shader; }
            inline void setLayerShader(ImageShader* shader) { this->shader = shader; }
        };

        Scene();
        ~Scene();

        /**
            Create a new layer
        */
        BitmapLayer& newBitmapLayer(const char* name);
        BitmapLayer& newBitmapLayer();
        MaskedBitmapLayer& newMaskedBitmapLayer(const char* name);
        MaskedBitmapLayer& newMaskedBitmapLayer();
        ShapedBitmapLayer& newShapedBitmapLayer(const char* name);
        ShapedBitmapLayer& newShapedBitmapLayer();
        ShadedBitmapLayer& newShadedBitmapLayer(const char* name);
        ShadedBitmapLayer& newShadedBitmapLayer();

        /**
            Adds a subscene
        */
        SceneLayer& addScene(const Scene& scene);

        /**
            Retrieves layer by name
        */
        Layer* getLayer(const char* name) const;

        /**
            Retrieves layer by index
        */
        Layer& getLayer(int index) const;

        /**
            Retrieves a layer by its position
        */
        Layer* getLayer(float x, float y, unsigned int recursionDepth = 0) const;

        /**
            Returns layer index in the scene or -1 if not found
        */
        int getLayerIndex(const Layer& layer) const;

        /**
            Returns total number of layers
        */
        int getLayerCount() const;

        /**
            Computes absolute mapping of a given layer with respect to the scene mapping
            \return `true` if the given layer is reached, `false` otherwise (mapping is not changed in this case)
        */
        bool resolveMapping(const Layer& layer, AffineMapping& mapping) const;

        /**
            Adds an existing layer to the scene
        */
        void attachLayer(Layer& layer);

        /**
            Detaches a layer from the scene
        */
        Layer* detachLayer(int index);
    };

}
