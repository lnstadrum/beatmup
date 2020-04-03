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
    Recursive scene rendering instruction
*/
void SceneRenderer::renderLayer(RenderingContext& context, TaskThread& thread, Scene::Layer& layer, const AffineMapping& base, unsigned int recursionLevel) {
    if (recursionLevel >= MAX_RECURSION_LEVEL)
        return;

    switch (layer.getType()) {

    case Scene::Layer::Type::SceneLayer: {
        const Scene& scene = layer.castTo<Scene::SceneLayer>().getScene();
        for (int i = 0; i < scene.getLayerCount() && !thread.isTaskAborted(); ++i) {
            Scene::Layer& l = scene.getLayer(i);
            if (l.isVisible())
                renderLayer(context, thread, l, base * layer.getMapping(), recursionLevel + 1);
        }
    }
    break;

    default:
        context.setMapping(base * layer.getMapping());
        layer.render(context);
        break;
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
    this->output = nullptr;
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
    // mapping to [0..1]^2 domain first
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


void SceneRenderer::setRenderingEventListener(RenderingContext::EventListener* eventListener) {
    this->eventListener = eventListener;
}


void SceneRenderer::beforeProcessing(ThreadIndex threadCount, GraphicPipeline* gpu) {
    if (eventListener)
        eventListener->onRenderingStart();

    // reset camera frame
    cameraFrame = nullptr;
}


void SceneRenderer::afterProcessing(ThreadIndex threadCount, bool aborted) {
}


bool SceneRenderer::doRender(GraphicPipeline& gpu, TaskThread& thread) {
    // check if there is the content to render
    if (!scene)
        return true;

    // setting output
    if (output)
        gpu.bindOutput(*output);
    else {
        gpu.unbindOutput();
    }

    RenderingContext context(gpu, eventListener,
        referenceWidth > 0 ? referenceWidth : gpu.getOutputResolution().getWidth(),
        output == nullptr);

    // background
    if (background) {
        context.lockBitmap(background);
        gpu.getRenderingPrograms().paveBackground(&gpu, *background, output == nullptr);
    }

    // enable blending
    gpu.switchAlphaBlending(true);

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
        if (layer.isVisible())
            renderLayer(context, thread, layer, outputCoords);
    }

    gpu.flush();

    // swap output if needed (and if the rendering task was not aborted)
    if (!thread.isTaskAborted()) {
        if (!output)
            gpu.swapBuffers();

        if (output && outputPixelsFetching) {
            AbstractBitmap::WriteLock lock(*output);
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
    throw RuntimeError("GPU is required for rendering");
    return true;
}


SceneRenderer::SceneRenderer():
    scene(nullptr), background(nullptr), output(nullptr),
    outputMapping(FIT_WIDTH_TO_TOP),
    referenceWidth(0),
    outputPixelsFetching(false),
    eventListener(nullptr)
{}


SceneRenderer::~SceneRenderer() {}
