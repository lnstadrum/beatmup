/*
	Low-level graphic pipeline manipulation
*/

#pragma once

#include "../geometry.h"
#include "../gpu/texture_handler.h"
#include "../bitmap/abstract_bitmap.h"
#include "../bitmap/pixel_arithmetic.h"
#include "program.h"
#include <mutex>


#ifdef BEATMUP_OPENGLVERSION_GLES20
#define BGL_SHADER_HEADER_VERSION "#version 200 es"
#elif BEATMUP_OPENGLVERSION_GLES31
#define BGL_SHADER_HEADER_VERSION "#version 310 es"
#else
#define BGL_SHADER_HEADER_VERSION "#version 430"
#endif

#define BGL_SHADER_CODE(X) BGL_SHADER_HEADER_VERSION "\n" #X


namespace Beatmup {
	class GraphicPipeline {
		friend class GL::TextureHandler;
	private:
		class Impl;
		Impl* impl;
		std::mutex access;

		GraphicPipeline(const GraphicPipeline&) = delete;				//!< disabling copy constructor

	public:
		enum class Interpolation {
			NEAREST,
			LINEAR
		};

		enum class Limit {
			LOCAL_GROUPS_X,
			LOCAL_GROUPS_Y,
			LOCAL_GROUPS_Z,
			LOCAL_GROUPS_TOTAL,
			SHARED_MEM,
		};

		GraphicPipeline();
		~GraphicPipeline();

		void lock();

		void unlock();

		void switchDisplay(void* data);

		void swapBuffers();

		void blend(GL::TextureHandler&, const pixfloat4&, const AffineMapping&);

		void blendMasked(
			const AffineMapping& baseMapping,
			GL::TextureHandler& image,
			const AffineMapping& imageMapping,
			AbstractBitmap& mask,
			const AffineMapping& maskMapping,
			const pixfloat4& modulation,
			const pixfloat4& bgColor
		);
		
		/**
			Renderns an image cropped by a parametric shape
			\param baseMapping			base mapping to the scene
			\param image				the image to render
			\param imageMapping			image to base mapping
			\param maskMapping			mask to base mapping
			\param border				border width
			\param slope				border slope width
			\param radius				corner radius
			\param referenceSize		if positive, the widths above are treated in pixels, and this parameter is taken as output width
			\param bgColor				a color to fill mask areas where the image is not present
		*/
		void blendShaped(
			const AffineMapping& baseMapping,
			GL::TextureHandler& image,
			const AffineMapping& imageMapping,
			const AffineMapping& maskMapping,
			const float border,
			const float slope,
			const float radius,
			const int referenceSize,
			const pixfloat4& modulation,
			const pixfloat4& bgColor
		);

		void blendCustom(GL::TextureHandler*, const AffineMapping&, GL::Program&);


		GL::VertexShader& getBlendingVertexShader() const;


		void paveBackground(AbstractBitmap& image);

		/**
			Plugs a texture handler to the pipeline output
		*/
		void setOutput(AbstractBitmap&);

		/**
			Unbinds a bitmap from output and switches to screen
		*/
		void resetOutput();
		
		ImageResolution getOutputResolution() const;

		/**
			Resets texture unit counter when binding
		*/
		void resetTextureBinding();

		unsigned int bindSampler(GL::TextureHandler& texture, int unit);
		unsigned int bindImage(GL::TextureHandler& texture, int unit, bool read, bool write);

		void setInterpolation(const Interpolation interpolation);

		void fetchPixels(AbstractBitmap& bitmap);

		int getLimit(Limit limit) const;
	};
}