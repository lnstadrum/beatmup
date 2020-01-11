/**
	Rendering context collecting all necessary data when rendering a scene
*/
#pragma once

#include "../gpu/pipeline.h"
#include "../gpu/program.h"
#include <map>

namespace Beatmup {
	/**
		Stores the rendering context: current program, current mapping in the scene space, common rendering properties,
		locked bitmaps, etc. Provides a set of tools and shortcuts to access rendering programs routines.
	*/
	class RenderingContext {
		friend class SceneRenderer;
	public:
		class EventListener {
		public:
			virtual void onRenderingStart() = 0;
		};

	private:
		std::map<BitmapPtr, int> lockedBitmaps;		//!< all the locked bitmaps with reference counters
		EventListener* eventListener;
		AffineMapping mapping;

		const float outputWidth;
		const bool renderingOnScreen;

		GraphicPipeline& gpu;
		RenderingContext(GraphicPipeline& gpu, EventListener* eventListener, const float outputWidth, const bool renderingOnScreen);
		~RenderingContext();
	public:
		void lockBitmap(BitmapPtr bitmap);
		void unlockBitmap(BitmapPtr bitmap);
		GraphicPipeline& getGpu() { return gpu; }

		/**
			Initiates the rendering operation.
		*/
		void blend();

		void enableProgram(RenderingPrograms::Program type);
		void enableProgram(GL::Program& program);
		GL::Program& getProgram();

		void bindMask(AbstractBitmap& mask);

		void setMapping(const AffineMapping& mapping) { this->mapping = mapping; }
		const AffineMapping& getMapping() const { return mapping; }

		const float getOutputWidth() const { return outputWidth; }
		const bool isRenderingOnScreen() const { return renderingOnScreen; }
	};

}
