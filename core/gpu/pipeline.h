/*
	Low-level graphic pipeline manipulation
*/

#pragma once

#include "../geometry.h"
#include "../gpu/texture_handler.h"
#include "../bitmap/abstract_bitmap.h"
#include "../bitmap/pixel_arithmetic.h"
#include "program.h"
#include "../scene/rendering_programs.h"

#include <mutex>


namespace Beatmup {
	class GraphicPipeline {
		friend class GL::TextureHandler;
	private:
		class Impl;
		Impl* impl;
		std::mutex access;

		RenderingPrograms renderingPrograms;

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


		/**
			Plugs a texture handler to the pipeline output
		*/
		void setOutput(AbstractBitmap&);

		/**
			Unbinds a bitmap from output and switches to screen
		*/
		void resetOutput();
		
		ImageResolution getOutputResolution() const;

		void bind(GL::TextureHandler& texture, int unit, bool repeat);
		void bind(GL::TextureHandler& texture, int imageUnit, bool read, bool write);

		void setInterpolation(const Interpolation interpolation);

		void fetchPixels(AbstractBitmap& bitmap);

		int getLimit(Limit limit) const;

		inline RenderingPrograms & getRenderingPrograms() { return renderingPrograms; }
	};
}