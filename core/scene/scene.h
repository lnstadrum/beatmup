/*
	Scene representation
*/

#pragma once
#include "../exception.h"
#include "../bitmap/abstract_bitmap.h"
#include "../bitmap/pixel_arithmetic.h"
#include "../geometry.h"
#include "../utils/lockable_object.h"
#include "layer_shader.h"
#include <string>

namespace Beatmup {

	/**
		A scene (ordered set of layers)
	*/
	class Scene : public LockableObject {
	private:
		class Impl;
		Impl* impl;

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
			Layer(const Layer&) = delete;		//!< disabling copying constructor
		public:
			enum Type {
				SceneLayer = 0,			//!< layer containing a scene
				BitmapLayer,			//!< layer displaying a bitmap
				MaskedBitmapLayer,		//!< layer displaying a bitmap with mask
				ShapedBitmapLayer,		//!< layer displaying a bitmap within a shape
				ShadedBitmapLayer		//!< layer displaying a bitmap through a custom fragment shader
			};

			std::string name;			//!< layer name
			AffineMapping mapping;		//!< layer mapping
			bool visible;				//!< layer visibility
			bool phantom;				//!< if `true`, layer is ignored by selection by point

			/**
				Returns layer object type
			*/
			inline Type getType() const { return type; }

			/**
				Tests if a point is on this layer
			*/
			virtual bool testPoint(float x, float y) const;

			/**
				Picks a child layer at given point, if any
			*/
			virtual Layer* getChild(float x, float y) const;
			
			template<class LayerType> LayerType& castTo() const {
				return (LayerType&)(*this);
			}

		protected:
			Layer(Type type);

		private:
			Type type;
		};


		/**
			Layer that contains an entire scene
		*/
		class SceneLayer : public Layer {
			friend class Impl;
		private:
			const Beatmup::Scene& scene;
			SceneLayer(const Beatmup::Scene& scene);
		public:
			const Beatmup::Scene& getScene() const { return scene; }
			bool testPoint(float x, float y) const;
			Layer* getChild(float x, float y) const;
		};


		/**
			Layer containing a bitmap cropped by a mask
		*/
		class BitmapLayer : public Layer {
			friend class Impl;
		private:
			BitmapLayer();
		protected:
			BitmapLayer(Type type);
		public:
			enum ImageSource {
				BITMAP
#ifdef BEATMUP_PLATFORM_ANDROID
				, CAMERA
#endif
			};

			BitmapPtr bitmap;				//!< content to display, used if the image source is set to BITMAP
			ImageSource source;				//!< content source
			pixfloat4 modulation;			//!< modulating color
			AffineMapping bitmapMapping;	//!< bitmap mapping w.r.t. the layer mapping
			bool testPoint(float x, float y) const;
		};


		/**
			Layer containing a bitmap cropped by a mask
		*/
		class CustomMaskedBitmapLayer : public BitmapLayer {
			friend class Impl;
		protected:
			CustomMaskedBitmapLayer(Type type);
		public:
			AffineMapping maskMapping;		//!< mask mapping w.r.t. the layer mapping
			pixfloat4 bgColor;				//!< color to fill mask areas where the bitmap is not present
		};


		/**
			Layer containing a bitmap cropped by a bitmap mask
		*/
		class MaskedBitmapLayer : public CustomMaskedBitmapLayer {
			friend class Impl;
		private:
			MaskedBitmapLayer();
		public:
			BitmapPtr mask;		//!< mask bitmap
			bool testPoint(float x, float y) const;
		};


		/**
			Layer containing a bitmap cropped by a shape mask
		*/
		class ShapedBitmapLayer : public CustomMaskedBitmapLayer {
			friend class Impl;
		private:
			ShapedBitmapLayer();
		public:
			enum Shape {
				SQUARE
			};

			Shape shape;
			float borderWidth;		//!< constant border thickness
			float slopeWidth;		//!< thickness of the smoothed line between border and image
			float cornerRadius;		//!< border corner radius
			bool inPixels;			//!< if `true`, the widths and radius are set in pixels
			
			bool testPoint(float x, float y) const;
		};


		/**
			Custom-shaded bitmap layer
		*/
		class ShadedBitmapLayer : public BitmapLayer {
			friend class Impl;
		private:
			ShadedBitmapLayer();
		public:
			LayerShader* layerShader;
		};

		Scene();
		~Scene();

		/**
			Create a new layer
		*/
		BitmapLayer& newBitmapLayer(const std::string& name);
		BitmapLayer& newBitmapLayer();
		MaskedBitmapLayer& newMaskedBitmapLayer(const std::string& name);
		MaskedBitmapLayer& newMaskedBitmapLayer();
		ShapedBitmapLayer& newShapedBitmapLayer(const std::string& name);
		ShapedBitmapLayer& newShapedBitmapLayer();
		ShadedBitmapLayer& newShadedBitmapLayer(const std::string& name);
		ShadedBitmapLayer& newShadedBitmapLayer();

		/**
			Adds a subscene
		*/
		SceneLayer& addScene(const Scene& scene);

		/**
			Retrieves layer by name
		*/
		Layer* getLayer(const std::string name) const;

		/**
			Retrieves layer by index
		*/
		Layer& getLayer(int index) const;

		/**
			Retrieves a layer by its position
		*/
		Layer* getLayer(float x, float y) const;

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