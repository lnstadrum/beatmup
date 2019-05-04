/*
	Scene renderer
*/

#pragma once
#include "scene.h"
#include "../gpu/gpu_task.h"

namespace Beatmup {
	class RenderingEventListener {
	public:
		virtual void onRenderingStart() = 0;

		/**
			Called once per rendering session just before the camera frame is going to be rendered
		*/
		virtual void onCameraFrameRendering(GL::TextureHandler*& cameraFrame) = 0;
	};


	class SceneRenderer : public GPUTask {
	protected:
		class Impl;
		Impl *impl;
		bool processOnGPU(GraphicPipeline& gpu, TaskThread& thread);
		void beforeProcessing(ThreadIndex threadCount, GraphicPipeline* gpu);
		void afterProcessing(ThreadIndex threadCount, bool aborted);

	public:
		SceneRenderer();
		~SceneRenderer();

		/**
			Scene coordinates to output (screen or bitmap) mapping
		*/
		enum OutputMapping {
			STRETCH = 0,		//!< output viewport covers entirely the scene axis span, aspect ratio is not preserved in general
			FIT_WIDTH_TO_TOP,	//!< width is covered entirely, height is resized to keep aspect ratio, the top borders are aligned
			FIT_WIDTH,			//!< width is covered entirely, height is resized to keep aspect ratio, point (0.5, 0.5) is mapped to the output center
			FIT_HEIGHT			//!< height is covered entirely, width is resized to keep aspect ratio, point (0.5, 0.5) is mapped to the output center
		};

		/**
			Attaches a bitmap to the renderer output
		*/
		void setOutput(AbstractBitmap& bitmap);

		/**
			\return a pointer to output bitmap
		*/
		AbstractBitmap* getOutput() const;

		/**
			Removes a bitmap from the renderer output, if any, and switches to on-screen rendering
		*/
		void resetOutput();

		const Scene* getScene() const;

		void setScene(Scene& scene);

		/**
			\brief Sets the output mapping specifying how the scene coordinates [0,1]� are mapped to the output (screen or bitmap)
			\param		the new mapping 
		*/
		void setOutputMapping(OutputMapping);

		/**
			\brief Retrieves the output mapping specifying how the scene coordinates [0,1]� are mapped to the output (screen or bitmap)
			\return the actual mapping
		*/
		OutputMapping getOutputMapping() const;

		/**
			\brief	Sets a value overriding output width for elements that have their size in pixels, in order to render a resolution-independent picture
					If set negative or zero, the actual output width is taken
			\param	the new reference width in pixels
		*/
		void setOutputReferenceWidth(int newWidth);

		/**
			\brief Retrieves value overriding output width for elements that have their size in pixels, in order to render a resolution-independent picture
		*/
		int getOutputReferenceWidth() const;

		/**
			\brief Sets an image to pave the background when rendering to screen
		*/
		void setBackgroundImage(AbstractBitmap*);

		/**
			\brief Specifies whether the output bitmap should be fetched from GPU to CPU RAM every time the rendering is done.
		*/
		void setOutputPixelsFetching(bool fetch);

		bool getOutputPixelsFetching() const;

		void setRenderingEventListener(RenderingEventListener* eventListener);
	};
}