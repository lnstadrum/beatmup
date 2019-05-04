#include "renderer.h"
#include "../geometry.h"
#include "../bitmap/bitmap_access.h"
#include "../bitmap/interpolation.h"
#include "../bitmap/internal_bitmap.h"
#include "../bitmap/tools.h"
#include "../exception.h"
#include "../gpu/pipeline.h"

#include <algorithm>
#include <vector>
#include <map>
#include <mutex>

#include "../debug.h"

using namespace Beatmup;


class SceneRenderer::Impl {
private:
	std::map<BitmapPtr, int> lockedBitmaps;		//!< all the locked bitmaps with reference counters
	Scene* scene;								//!< content to render
	AbstractBitmap* background;					//!< used to pave the screen before rendering
	BitmapPtr output;							//!< output bitmap
	OutputMapping outputMapping;				//!< specfies how the scene coordinates [0,1] are mapped to the output (screen or bitmap)
	
	int referenceWidth;							//!< value overriding output width for elements that have their size in pixels, in order to render a resolution-independent picture

	bool outputPixelsFetching;					//!< if `true`, the output bitmap data is fetched from GPU to CPU RAM every time the rendering is done

	GL::TextureHandler* cameraFrame;			//!< last got camera frame; set to NULL before rendering, then asked from outside through eventListener

	RenderingEventListener* eventListener;
	
	static const unsigned int
		MAX_RECURSION_LEVEL = 256;

protected:
	/**
		\internal
		Lock a bitmap used by several threads
	*/
	void safeLockBitmap(BitmapPtr bitmap) {
		if (lockedBitmaps.count(bitmap) == 0) {
			// newbie got
			lockedBitmaps[bitmap] = 1;
			bitmap->lockPixels(ProcessingTarget::GPU);
		}
		else
			// already locked, just increase reference counter
			lockedBitmaps[bitmap]++;
	}

	/**
		\internal
		Unlocks a prevously used bitmap
	*/
	void safeUnlockBitmap(BitmapPtr bitmap) {
#ifdef BEATMUP_DEBUG
		if (lockedBitmaps.count(bitmap) == 0)
			BEATMUP_ERROR("Bitmap is not locked");
#endif
		int refs = --lockedBitmaps[bitmap];
		if (refs == 0) {
			bitmap->unlockPixels();
			lockedBitmaps.erase(bitmap);
		}
	}
	

	/**
		\internal
		Renders a layer
	*/
	void renderLayer(GraphicPipeline& gpu, TaskThread& thread, const Scene::Layer& layer, const AffineMapping& base, unsigned int recursionLevel = 0) {
		if (recursionLevel >= MAX_RECURSION_LEVEL)
			return;

		switch (layer.getType()) {

		case Scene::Layer::Type::SceneLayer: {
			const Scene& scene = layer.castTo<Scene::SceneLayer>().getScene();
			for (int i = 0; i < scene.getLayerCount() && !thread.isTaskAborted(); ++i) {
				const Scene::Layer& l = scene.getLayer(i);
				if (l.visible)
					renderLayer(gpu, thread, l, base * layer.mapping, recursionLevel + 1);
			}
		}
		break;

		case Scene::Layer::BitmapLayer: {
			const Scene::BitmapLayer& l = layer.castTo<Scene::BitmapLayer>();
			switch (l.source) {
			case Scene::BitmapLayer::ImageSource::BITMAP:
				if (l.bitmap) {
					safeLockBitmap(l.bitmap);
					gpu.blend(*l.bitmap, l.modulation, base * l.mapping * l.bitmapMapping);
				}
				break;
#ifdef BEATMUP_PLATFORM_ANDROID
			case Scene::BitmapLayer::ImageSource::CAMERA:
				if (!cameraFrame && eventListener)
					eventListener->onCameraFrameRendering(cameraFrame);
				if (cameraFrame)
					gpu.blend(*cameraFrame, l.modulation, base * l.mapping * l.bitmapMapping);
				break;
#endif
			}
		}
		break;

		case Scene::Layer::MaskedBitmapLayer: {
			const Scene::MaskedBitmapLayer& l = layer.castTo<Scene::MaskedBitmapLayer>();
			switch (l.source) {
			case Scene::BitmapLayer::ImageSource::BITMAP:
				if (l.bitmap) {
					safeLockBitmap(l.bitmap);
					if (l.mask) {
						safeLockBitmap(l.mask);
						gpu.blendMasked(
							base * layer.mapping,
							*l.bitmap,
							l.bitmapMapping,
							*l.mask,
							l.maskMapping,
							l.modulation,
							l.bgColor
						);
					}
					else
						gpu.blend(*l.bitmap, l.modulation, base * layer.mapping * l.bitmapMapping);
				}
				break;

#ifdef BEATMUP_PLATFORM_ANDROID
			case Scene::BitmapLayer::ImageSource::CAMERA:
				if (!cameraFrame && eventListener)
					eventListener->onCameraFrameRendering(cameraFrame);
				if (!cameraFrame)
					break;
				if (l.mask) {					
					safeLockBitmap(l.mask);
					gpu.blendMasked(
						base * layer.mapping,
						*cameraFrame,
						l.bitmapMapping,
						*l.mask,
						l.maskMapping,
						l.modulation,
						l.bgColor
					);
				}
				else
					gpu.blend(*cameraFrame, l.modulation, base * layer.mapping * l.bitmapMapping);
				break;
#endif
			}
		}
		break;

		case Scene::Layer::ShapedBitmapLayer: {
			const Scene::ShapedBitmapLayer& l = layer.castTo<Scene::ShapedBitmapLayer>();
			switch (l.source) {
			case Scene::BitmapLayer::ImageSource::BITMAP:
				if (l.bitmap) {
					safeLockBitmap(l.bitmap);
					gpu.blendShaped(
						base * layer.mapping,
						*l.bitmap,
						l.bitmapMapping,
						l.maskMapping,
						l.borderWidth, l.slopeWidth, l.cornerRadius,
						l.inPixels ? (referenceWidth > 0 ? referenceWidth : gpu.getOutputResolution().width) : 0,
						l.modulation,
						l.bgColor
					);
				}
				break;

#ifdef BEATMUP_PLATFORM_ANDROID
			case Scene::BitmapLayer::ImageSource::CAMERA:
				if (!cameraFrame && eventListener)
					eventListener->onCameraFrameRendering(cameraFrame);
				if (cameraFrame)
					gpu.blendShaped(
						base * layer.mapping,
						*cameraFrame,
						l.bitmapMapping,
						l.maskMapping,
						l.borderWidth, l.slopeWidth, l.cornerRadius,
						l.inPixels ? (referenceWidth > 0 ? referenceWidth : gpu.getOutputResolution().width) : 0,
						l.modulation,
						l.bgColor
					);
				break;
#endif
			}
		}
		break;

		case Scene::Layer::ShadedBitmapLayer: {
			const Scene::ShadedBitmapLayer& l = layer.castTo<Scene::ShadedBitmapLayer>();
			if (l.layerShader)
				switch (l.source) {
				case Scene::BitmapLayer::ImageSource::BITMAP:
					if (l.bitmap)
						safeLockBitmap(l.bitmap);
					l.layerShader->blend(gpu, l.bitmap, base * l.mapping * l.bitmapMapping);
					break;
#ifdef BEATMUP_PLATFORM_ANDROID
				case Scene::BitmapLayer::ImageSource::CAMERA:
					if (!cameraFrame && eventListener)
						eventListener->onCameraFrameRendering(cameraFrame);
					l.layerShader->blend(gpu, cameraFrame, base * l.mapping * l.bitmapMapping);
					break;
#endif
				}
		}
		break;

		default:
			Insanity::insanity("Incorrect layer type");
		}
	}


	/**
		Computes initial renderer mapping
	*/
	void computeBaseMapping(GraphicPipeline& gpu, AffineMapping& result) {
		const ImageResolution res = gpu.getOutputResolution();
		result.setIdentity();
		switch (outputMapping) {
			case FIT_WIDTH_TO_TOP:
				result.matrix.scale(1.0f, res.getAspectRatio());
				break;
			case FIT_WIDTH: {
				result.matrix.scale(1.0f, res.getAspectRatio());
				result.setCenterPosition(Point(0.5f, 0.5f));
				break;
			}
			case FIT_HEIGHT: {
				result.matrix.scale((float)res.height / res.width, 1.0f);
				result.setCenterPosition(Point(0.5f, 0.5f));
				break;
			}
		}
	}


public:
	Impl() :
		scene(NULL), output(NULL), background(NULL),
		outputMapping(FIT_WIDTH_TO_TOP),
		referenceWidth(0),
		outputPixelsFetching(false),
		eventListener(NULL)
	{}


	const Scene* getScene() const {
		return scene;
	}
	

	AbstractBitmap* getOutput() const {
		return output;
	}


	void setScene(Scene& scene) {
		this->scene = &scene;
	}


	void setOutput(AbstractBitmap& output) {
		this->output = &output;
	}


	void resetOutput() {
		this->output = NULL;
	}

	
	void setOutputMapping(OutputMapping newMapping) {
		outputMapping = newMapping;
	}


	OutputMapping getOutputMapping() const {
		return outputMapping;
	}


	void setOutputReferenceWidth(int newWidth) {
		referenceWidth = newWidth;
	}


	int getOutputReferenceWidth() const {
		return referenceWidth;
	}


	void setBackgroundImage(AbstractBitmap* bitmap) {
		background = bitmap;
	}


	void setOutputPixelsFetching(bool fetch) {
		this->outputPixelsFetching = fetch;
	}


	bool getOutputPixelsFetching() const {
		return outputPixelsFetching;
	}


	void setRenderingEventListener(RenderingEventListener* eventListener) {
		this->eventListener = eventListener;
	}


	void beforeProcessing(ThreadIndex threadCount, GraphicPipeline* gpu) {
		if (eventListener)
			eventListener->onRenderingStart();
		
		// reset camera frame
		cameraFrame = NULL;

		lockedBitmaps.clear();

		// locking the output
		if (output) {
			output->lockPixels(ProcessingTarget::GPU);
			output->invalidate(ProcessingTarget::CPU);
		}

		// locking the background
		if (background)
			safeLockBitmap(background);
	}


	void afterProcessing() {
		for (auto it : lockedBitmaps)
			it.first->unlockPixels();
		lockedBitmaps.clear();
		if (output)
			output->unlockPixels();
	}


	bool doRender(GraphicPipeline& gpu, TaskThread& thread) {
		// check if there is the content to render
		if (!scene)
			return true;

		// setting output
		if (output)
			gpu.setOutput(*output);
		else {
			gpu.resetOutput();
			if (background)
				gpu.paveBackground(*background);
		}

		// compute initial mapping
		AffineMapping base;
		computeBaseMapping(gpu, base);

		// go
		for (int i = 0; i < scene->getLayerCount() && !thread.isTaskAborted(); ++i) {
			Scene::Layer& layer = scene->getLayer(i);
			if (layer.visible)
				renderLayer(gpu, thread, layer, base);
		}

		// swap output if needed (and if the rendering task was not aborted)
		if (!thread.isTaskAborted()) {
			if (!output)
				gpu.swapBuffers();

			if (output && outputPixelsFetching) {
				output->lockPixels(ProcessingTarget::CPU);
				gpu.fetchPixels(*output);
			}
		}

		return true;
	}


	bool processOnGPU(GraphicPipeline& gpu, TaskThread& thread) {
		if (!scene)
			return true;
		Scene::LockGuard lock(scene);

		// go
		return doRender(gpu, thread);
	}


	bool process(TaskThread& thread) {
		BEATMUP_ERROR("GPU is required for rendering");
		return true;
	}
};

void SceneRenderer::beforeProcessing(ThreadIndex threadCount, GraphicPipeline* gpu) {
	impl->beforeProcessing(threadCount, gpu);
}

void SceneRenderer::afterProcessing(ThreadIndex threadCount, bool aborted) {
	impl->afterProcessing();
}

bool SceneRenderer::processOnGPU(GraphicPipeline& gpu, TaskThread& thread) {
	return impl->processOnGPU(gpu, thread);
}

SceneRenderer::SceneRenderer() {
	impl = new SceneRenderer::Impl();
}

SceneRenderer::~SceneRenderer() {
	delete impl;
}

void SceneRenderer::setOutput(AbstractBitmap& bitmap) {
	impl->setOutput(bitmap);
}

AbstractBitmap* SceneRenderer::getOutput() const {
	return impl->getOutput();
}

void SceneRenderer::resetOutput() {
	impl->resetOutput();
}

const Scene* SceneRenderer::getScene() const {
	return impl->getScene();
}

void SceneRenderer::setScene(Scene& scene) {
	impl->setScene(scene);
}

void SceneRenderer::setOutputMapping(OutputMapping mapping) {
	impl->setOutputMapping(mapping);
}

SceneRenderer::OutputMapping SceneRenderer::getOutputMapping() const {
	return impl->getOutputMapping();
}

void SceneRenderer::setOutputReferenceWidth(int newWidth) {
	impl->setOutputReferenceWidth(newWidth);
}

int SceneRenderer::getOutputReferenceWidth() const {
	return impl->getOutputReferenceWidth();
}

void SceneRenderer::setBackgroundImage(AbstractBitmap* bitmap) {
	impl->setBackgroundImage(bitmap);
}

void SceneRenderer::setOutputPixelsFetching(bool fetch) {
	impl->setOutputPixelsFetching(fetch);
}

bool SceneRenderer::getOutputPixelsFetching() const {
	return impl->getOutputPixelsFetching();
}

void SceneRenderer::setRenderingEventListener(RenderingEventListener* eventListener) {
	impl->setRenderingEventListener(eventListener);
}