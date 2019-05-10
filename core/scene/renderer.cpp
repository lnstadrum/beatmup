#include "renderer.h"
#include "../geometry.h"
#include "../bitmap/bitmap_access.h"
#include "../bitmap/interpolation.h"
#include "../bitmap/internal_bitmap.h"
#include "../bitmap/tools.h"
#include "../exception.h"
#include "../gpu/pipeline.h"

#include "../debug.h"

using namespace Beatmup;


/**
	\internal
	Lock a bitmap used by several threads
*/
void SceneRenderer::safeLockBitmap(BitmapPtr bitmap) {
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
void SceneRenderer::safeUnlockBitmap(BitmapPtr bitmap) {
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
	Recursive scene rendering instruction
*/
void SceneRenderer::renderLayer(GraphicPipeline& gpu, TaskThread& thread, const Scene::Layer& layer, const AffineMapping& base, unsigned int recursionLevel) {
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

	case Scene::Layer::Type::BitmapLayer: {
		Scene::BitmapLayer& l = layer.castTo<Scene::BitmapLayer>();
		switch (l.source) {
		case Scene::BitmapLayer::ImageSource::BITMAP:
			if (l.bitmap) {
				safeLockBitmap(l.bitmap);
				gpu.blend(*l.bitmap, l.modulation, base * l.mapping * l.bitmapMapping);
				l.invAr = l.bitmap->getInvAspectRatio();
			}
			break;
#ifdef BEATMUP_PLATFORM_ANDROID
		case Scene::BitmapLayer::ImageSource::CAMERA:
			if (!cameraFrame && eventListener)
				eventListener->onCameraFrameRendering(cameraFrame);
			if (cameraFrame) {
				gpu.blend(*cameraFrame, l.modulation, base * l.mapping * l.bitmapMapping);
				l.invAr = cameraFrame->getInvAspectRatio();
			}
			break;
#endif
		}
	}
	break;

	case Scene::Layer::Type::MaskedBitmapLayer: {
		Scene::MaskedBitmapLayer& l = layer.castTo<Scene::MaskedBitmapLayer>();
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
				l.invAr = l.bitmap->getInvAspectRatio();
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
			l.invAr = cameraFrame->getInvAspectRatio();
			break;
#endif
		}
	}
	break;

	case Scene::Layer::Type::ShapedBitmapLayer: {
		Scene::ShapedBitmapLayer& l = layer.castTo<Scene::ShapedBitmapLayer>();
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
					l.inPixels ? (referenceWidth > 0 ? referenceWidth : resolution.getWidth()) : 0,
					l.modulation,
					l.bgColor
				);
				l.invAr = l.bitmap->getInvAspectRatio();
			}
			break;

#ifdef BEATMUP_PLATFORM_ANDROID
		case Scene::BitmapLayer::ImageSource::CAMERA:
			if (!cameraFrame && eventListener)
				eventListener->onCameraFrameRendering(cameraFrame);
			if (cameraFrame) {
				gpu.blendShaped(
					base * layer.mapping,
					*cameraFrame,
					l.bitmapMapping,
					l.maskMapping,
					l.borderWidth, l.slopeWidth, l.cornerRadius,
					l.inPixels ? (referenceWidth > 0 ? referenceWidth : resolution.getWidth()) : 0,
					l.modulation,
					l.bgColor
				);
				l.invAr = cameraFrame->getInvAspectRatio();
			}
			break;
#endif
		}
	}
	break;

	case Scene::Layer::Type::ShadedBitmapLayer: {
		Scene::ShadedBitmapLayer& l = layer.castTo<Scene::ShadedBitmapLayer>();
		if (l.layerShader)
			switch (l.source) {
			case Scene::BitmapLayer::ImageSource::BITMAP:
				if (l.bitmap)
					safeLockBitmap(l.bitmap);
				l.layerShader->blend(gpu, l.bitmap, base * l.mapping * l.bitmapMapping);
				if (l.bitmap)
					l.invAr = l.bitmap->getInvAspectRatio();
				break;
#ifdef BEATMUP_PLATFORM_ANDROID
			case Scene::BitmapLayer::ImageSource::CAMERA:
				if (!cameraFrame && eventListener)
					eventListener->onCameraFrameRendering(cameraFrame);
				l.layerShader->blend(gpu, cameraFrame, base * l.mapping * l.bitmapMapping);
				if (cameraFrame)
					l.invAr = cameraFrame->getInvAspectRatio();
				break;
#endif
			}
	}
	break;

	default:
		Insanity::insanity("Incorrect layer type");
	}
}


const Scene* SceneRenderer::getScene() const {
	return scene;
}
	

AbstractBitmap* SceneRenderer::getOutput() const {
	return output;
}


void SceneRenderer::setScene(Scene& scene) {
	this->scene = &scene;
}


void SceneRenderer::setOutput(AbstractBitmap& output) {
	this->output = &output;
}


void SceneRenderer::resetOutput() {
	this->output = NULL;
}

	
void SceneRenderer::setOutputMapping(const OutputMapping newMapping) {
	outputMapping = newMapping;
}


SceneRenderer::OutputMapping SceneRenderer::getOutputMapping() const {
	return outputMapping;
}


void SceneRenderer::setOutputReferenceWidth(int newWidth) {
	referenceWidth = newWidth;
}


int SceneRenderer::getOutputReferenceWidth() const {
	return referenceWidth;
}


void SceneRenderer::setBackgroundImage(AbstractBitmap* bitmap) {
	background = bitmap;
}


void SceneRenderer::setOutputPixelsFetching(bool fetch) {
	this->outputPixelsFetching = fetch;
}


bool SceneRenderer::getOutputPixelsFetching() const {
	return outputPixelsFetching;
}


Scene::Layer* SceneRenderer::pickLayer(float x, float y, bool normalized) const {
	if (scene == nullptr)
		return nullptr;
	// mapping to [0..1]² domain first
	if (normalized) {
		y *= resolution.getAspectRatio();
	}
	else {
		x /= resolution.getWidth();
		y /= resolution.getHeight();
	}
	const Point p = outputCoords.getInverse(x, y);
	return scene->getLayer(p.x, p.y);
}


void SceneRenderer::setRenderingEventListener(RenderingEventListener* eventListener) {
	this->eventListener = eventListener;
}


void SceneRenderer::beforeProcessing(ThreadIndex threadCount, GraphicPipeline* gpu) {
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


void SceneRenderer::afterProcessing(ThreadIndex threadCount, bool aborted) {
	for (auto it : lockedBitmaps)
		it.first->unlockPixels();
	lockedBitmaps.clear();
	if (output)
		output->unlockPixels();
}


bool SceneRenderer::doRender(GraphicPipeline& gpu, TaskThread& thread) {
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
	resolution = gpu.getOutputResolution();
	outputCoords.setIdentity();
	switch (outputMapping) {
	case FIT_WIDTH_TO_TOP:
		outputCoords.matrix.scale(1.0f, resolution.getAspectRatio());
		break;
	case FIT_WIDTH:
		outputCoords.matrix.scale(1.0f, resolution.getAspectRatio());
		outputCoords.setCenterPosition(Point(0.5f, 0.5f));
		break;
	case FIT_HEIGHT:
		outputCoords.matrix.scale(resolution.getInvAspectRatio(), 1.0f);
		outputCoords.setCenterPosition(Point(0.5f, 0.5f));
		break;
	case STRETCH:
		// identity is okay
		break;
	}

	// go
	for (int i = 0; i < scene->getLayerCount() && !thread.isTaskAborted(); ++i) {
		Scene::Layer& layer = scene->getLayer(i);
		if (layer.visible)
			renderLayer(gpu, thread, layer, outputCoords);
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


bool SceneRenderer::processOnGPU(GraphicPipeline& gpu, TaskThread& thread) {
	if (!scene)
		return true;
	Scene::LockGuard lock(scene);

	// go
	return doRender(gpu, thread);
}


bool SceneRenderer::process(TaskThread& thread) {
	BEATMUP_ERROR("GPU is required for rendering");
	return true;
}


SceneRenderer::SceneRenderer():
	scene(NULL), output(NULL), background(NULL),
	outputMapping(FIT_WIDTH_TO_TOP),
	referenceWidth(0),
	outputPixelsFetching(false),
	eventListener(NULL)
{}


SceneRenderer::~SceneRenderer() {}