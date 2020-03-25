#include "wrapper.h"

#include "jniheaders/Beatmup_Bitmap.h"
#include "jniheaders/Beatmup_Android_ExternalBitmap.h"
#include "jniheaders/Beatmup_Rendering_Scene.h"
#include "jniheaders/Beatmup_Rendering_SceneRenderer.h"
#include "jniheaders/Beatmup_Shading_Shader.h"
#include "jniheaders/Beatmup_Shading_ShaderApplicator.h"
#include "jniheaders/Beatmup_Imaging_BinaryOperation.h"
#include "jniheaders/Beatmup_Imaging_FloodFill.h"
#include "jniheaders/Beatmup_Imaging_Filters_Local_PixelwiseFilter.h"
#include "jniheaders/Beatmup_Imaging_Filters_Local_ColorMatrixTransform.h"
#include "jniheaders/Beatmup_Imaging_Filters_ImageTuning.h"
#include "jniheaders/Beatmup_Imaging_Filters_Resampler.h"
#include "jniheaders/Beatmup_Imaging_Filters_Local_Sepia.h"
#include "jniheaders/Beatmup_Imaging_ColorMatrix.h"

#include "android/context.h"
#include "android/bitmap.h"
#include "android/external_bitmap.h"
#include "../core/bitmap/resampler.h"

#include <core/context.h>
#include <core/geometry.h>
#include <core/bitmap/internal_bitmap.h>
#include <core/bitmap/resampler.h>
#include <core/bitmap/crop.h>
#include <core/bitmap/operator.h>
#include <core/masking/flood_fill.h>
#include <core/scene/renderer.h>
#include <core/shading/shader_applicator.h>
#include <core/filters/local/pixelwise_filter.h>
#include <core/filters/local/color_matrix.h>
#include <core/filters/local/sepia.h>
#include <core/filters/tuning.h>
#include <core/color/matrix.h>
#include <core/contours/contours.h>
#include <core/gpu/swapper.h>
#include <core/exception.h>

/////////////////////////////////////////////////////////////////////////////////////////////
//                                          SCENE
/////////////////////////////////////////////////////////////////////////////////////////////

/**
    Creates new scene
*/
JNIMETHOD(jlong, newScene, Java_Beatmup_Rendering_Scene, newScene)(JNIEnv * jenv, jclass) {
    BEATMUP_ENTER;
    return (jlong) (new Beatmup::Scene());
}


/**
    Creates new layer containing a scene
 */
JNIMETHOD(jlong, newSceneLayer, Java_Beatmup_Rendering_Scene, newSceneLayer)(JNIEnv * jenv, jobject, jlong hScene, jobject jLayer, jobject jSubscene) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene, subscene, jSubscene);
    BEATMUP_OBJ(Beatmup::Scene, scene, hScene);
    Beatmup::Scene::Layer* newbie = &scene->addScene(*subscene);
    BEATMUP_REFERENCE(jLayer, newbie);      // adding reference
    return (jlong)(newbie);
}

/**
    Creates new bitmap layer in a given scene
*/
JNIMETHOD(jlong, newBitmapLayer, Java_Beatmup_Rendering_Scene, newBitmapLayer)(JNIEnv * jenv, jobject, jlong hScene, jobject jLayer) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene, scene, hScene);
    Beatmup::Scene::Layer* newbie = &scene->newBitmapLayer();
    BEATMUP_REFERENCE(jLayer, newbie);      // adding reference
    return (jlong)(newbie);
}

/**
    Creates new masked bitmap layer in a given scene
*/
JNIMETHOD(jlong, newMaskedBitmapLayer, Java_Beatmup_Rendering_Scene, newMaskedBitmapLayer)(JNIEnv * jenv, jobject, jlong hScene, jobject jLayer) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene, scene, hScene);
    Beatmup::Scene::Layer* newbie = &scene->newMaskedBitmapLayer();
    BEATMUP_REFERENCE(jLayer, newbie);      // adding reference
    return (jlong)(newbie);
}

/**
     Creates new shaped bitmap layer in a given scene
 */
JNIMETHOD(jlong, newShapedBitmapLayer, Java_Beatmup_Rendering_Scene, newShapedBitmapLayer)(JNIEnv * jenv, jobject, jlong hScene, jobject jLayer) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene, scene, hScene);
    Beatmup::Scene::Layer* newbie = &scene->newShapedBitmapLayer();
    BEATMUP_REFERENCE(jLayer, newbie);      // adding reference
    return (jlong)(newbie);
}


JNIMETHOD(jlong, newShadedBitmapLayer, Java_Beatmup_Rendering_Scene, newShadedBitmapLayer)(JNIEnv * jenv, jobject, jlong hScene, jobject jLayer) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene, scene, hScene);
    Beatmup::Scene::Layer* newbie = &scene->newShadedBitmapLayer();
    BEATMUP_REFERENCE(jLayer, newbie);      // adding reference
    return (jlong)(newbie);
}

/**
    Removes global references
*/
JNIMETHOD(void, deleteLayers, Java_Beatmup_Rendering_Scene, deleteLayers)(JNIEnv * jenv, jobject jScene) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene, scene, jScene);
    int n = scene->getLayerCount();
    for (int i = 0; i < n; i++) {
        Beatmup::Scene::Layer &l = scene->getLayer(i);
        BEATMUP_DELETE_REFERENCE(&l);
    }
}

/**
    Returns number of layers
*/
JNIMETHOD(jint, getLayerCount, Java_Beatmup_Rendering_Scene, getLayerCount)(JNIEnv * jenv, jobject, jlong hScene) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene, scene, hScene);
    return scene->getLayerCount();
}

/**
    Retrieves layer by its index
*/
JNIMETHOD(jobject, getLayerByIndex, Java_Beatmup_Rendering_Scene, getLayerByIndex)(JNIEnv * jenv, jobject, jlong hScene, jint index) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene, scene, hScene);
    return $pool.getJavaReference(&scene->getLayer(index));
}

/**
    Retrieves a layer by name
*/
JNIMETHOD(jobject, getLayerByName, Java_Beatmup_Rendering_Scene, getLayerByName)(JNIEnv * jenv, jobject, jlong hScene, jstring name) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene, scene, hScene);
    BEATMUP_STRING(name);
    Beatmup::Scene::Layer* layer = scene->getLayer(nameStr.c_str());
    if (!layer)
        return NULL;
    return $pool.getJavaReference(layer);
}


JNIMETHOD(jobject, getLayerAtPoint, Java_Beatmup_Rendering_Scene, getLayerAtPoint)(JNIEnv * jenv, jobject, jlong hScene, jfloat x, jfloat y) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene, scene, hScene);
    Beatmup::Scene::Layer* layer = scene->getLayer(x, y);
    if (!layer)
        return NULL;
    return $pool.getJavaReference(layer);
}

/**
    Renames given layer
*/
JNIMETHOD(void, setLayerName, Java_Beatmup_Rendering_Scene, setLayerName)(JNIEnv * jenv, jobject, jlong hLayer, jstring name) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene::Layer, layer, hLayer);
    BEATMUP_STRING(name);
    layer->setName(nameStr.c_str());
}

/**
    Returns layer name
*/
JNIMETHOD(jstring, getLayerName, Java_Beatmup_Rendering_Scene, getLayerName)(JNIEnv * jenv, jobject, jlong hLayer) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene::Layer, layer, hLayer);
    return jenv->NewStringUTF(layer->getName());
}


JNIMETHOD(void, setLayerVisibility, Java_Beatmup_Rendering_Scene, setLayerVisibility)(JNIEnv * jenv, jobject, jlong hLayer, jboolean visible) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene::Layer, layer, hLayer);
    layer->setVisible(visible == JNI_TRUE);
}


JNIMETHOD(jboolean, getLayerVisibility, Java_Beatmup_Rendering_Scene, getLayerVisibility)(JNIEnv * jenv, jobject, jlong hLayer) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene::Layer, layer, hLayer);
    return layer->isVisible() ? JNI_TRUE : JNI_FALSE;
}

/**
    Phantom layer mode management
*/
JNIMETHOD(void, setLayerPhantomFlag, Java_Beatmup_Rendering_Scene, setLayerPhantomFlag)(JNIEnv * jenv, jobject, jlong hLayer, jboolean phantom) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene::Layer, layer, hLayer);
    layer->setPhantom(phantom == JNI_TRUE);
}


JNIMETHOD(jboolean, getLayerPhantomFlag, Java_Beatmup_Rendering_Scene, getLayerPhantomFlag)(JNIEnv * jenv, jobject, jlong hLayer) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene::Layer, layer, hLayer);
    return layer->isPhantom() ? JNI_TRUE : JNI_FALSE;
}


JNIMETHOD(void, setLayerTransform, Java_Beatmup_Rendering_Scene, setLayerTransform)(JNIEnv * jenv, jobject, jlong hLayer, jfloat x, jfloat y, jfloat a11, jfloat a12, jfloat a21, jfloat a22) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene::Layer, layer, hLayer);
    Beatmup::AffineMapping& mapping = layer->getMapping();
    mapping.position.x = x;
    mapping.position.y = y;
    mapping.matrix.setElements((float)a11, (float)a12, (float)a21, (float)a22);
}


JNIMETHOD(void, getLayerTransform, Java_Beatmup_Rendering_Scene, getLayerTransform)(JNIEnv * jenv, jobject, jlong hLayer, jobject mapping) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene::Layer, layer, hLayer);
    $pool.factory.setAffineMapping(jenv, layer->getMapping(), mapping);
}

/**
    Changes layer horizontal position
*/
JNIMETHOD(void, setLayerX, Java_Beatmup_Rendering_Scene, setLayerX)(JNIEnv * jenv, jobject, jlong hLayer, jfloat x) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene::Layer, layer, hLayer);
    layer->getMapping().position.x = x;
}

/**
    Returns layer horizontal position
*/
JNIMETHOD(jfloat, getLayerX, Java_Beatmup_Rendering_Scene, getLayerX)(JNIEnv * jenv, jobject, jlong hLayer) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene::Layer, layer, hLayer);
    return layer->getMapping().position.x;
}

/**
    Changes layer vertical position
*/
JNIMETHOD(void, setLayerY, Java_Beatmup_Rendering_Scene, setLayerY)(JNIEnv * jenv, jobject, jlong hLayer, jfloat y) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene::Layer, layer, hLayer);
    layer->getMapping().position.y = y;
}

/**
    Returns layer vertical position
*/
JNIMETHOD(jfloat, getLayerY, Java_Beatmup_Rendering_Scene, getLayerY)(JNIEnv * jenv, jobject, jlong hLayer) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene::Layer, layer, hLayer);
    return layer->getMapping().position.y;
}

/**
    Returns layer X scaling
*/
JNIMETHOD(jfloat, getLayerScaleX, Java_Beatmup_Rendering_Scene, getLayerScaleX)(JNIEnv * jenv, jobject, jlong hLayer) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene::Layer, layer, hLayer);
    return layer->getMapping().matrix.getScalingX();
}

/**
    Returns layer Y scaling
*/
JNIMETHOD(jfloat, getLayerScaleY, Java_Beatmup_Rendering_Scene, getLayerScaleY)(JNIEnv * jenv, jobject, jlong hLayer) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene::Layer, layer, hLayer);
    return layer->getMapping().matrix.getScalingY();
}

/**
    Returns layer orientation in degrees
*/
JNIMETHOD(jfloat, getLayerOrientation, Java_Beatmup_Rendering_Scene, getLayerOrientation)(JNIEnv * jenv, jobject, jlong hLayer) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene::Layer, layer, hLayer);
    return layer->getMapping().matrix.getOrientationDegrees();
}

/**
    Sets layer center position
*/
JNIMETHOD(void, setLayerCenterPos, Java_Beatmup_Rendering_Scene, setLayerCenterPos)(JNIEnv * jenv, jobject, jlong hLayer, jfloat x, jfloat y) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene::Layer, layer, hLayer);
    layer->getMapping().setCenterPosition(Beatmup::Point(x, y));
}


JNIMETHOD(void, scaleLayer, Java_Beatmup_Rendering_Scene, scaleLayer)(JNIEnv * jenv, jobject, jlong hLayer, jfloat factor, jfloat fixx, jfloat fixy) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene::Layer, layer, hLayer);
    layer->getMapping().scale(factor, Beatmup::Point(fixx, fixy));
}


JNIMETHOD(void, rotateLayer, Java_Beatmup_Rendering_Scene, rotateLayer)(JNIEnv * jenv, jobject, jlong hLayer, jfloat a, jfloat fixx, jfloat fixy) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene::Layer, layer, hLayer);
    layer->getMapping().rotateDegrees(a, Beatmup::Point(fixx, fixy));
}


JNIMETHOD(void, setBitmapLayerModulationColor, Java_Beatmup_Rendering_Scene, setBitmapLayerModulationColor)(JNIEnv * jenv, jobject, jlong hLayer, jfloat r, jfloat g, jfloat b, jfloat a) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene::BitmapLayer, layer, hLayer);
    layer->setModulationColor(Beatmup::pixfloat4(r, g, b, a));
}


JNIMETHOD(void, getBitmapLayerModulationColor, Java_Beatmup_Rendering_Scene, getBitmapLayerModulationColor)(JNIEnv * jenv, jobject, jlong hLayer, jobject jColor) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene::BitmapLayer, layer, hLayer);
    $pool.factory.setColor(jenv, layer->getModulationColor(), jColor);
}


JNIMETHOD(void, setBitmapLayerMaskPos, Java_Beatmup_Rendering_Scene, setBitmapLayerMaskPos)(JNIEnv * jenv, jobject, jlong hLayer, jfloat x, jfloat y) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene::CustomMaskedBitmapLayer, layer, hLayer);
    Beatmup::AffineMapping& mapping = layer->getMaskMapping();
    mapping.position.x = (float)x;
    mapping.position.y = (float)y;
}


JNIMETHOD(void, scaleBitmapLayerMask, Java_Beatmup_Rendering_Scene, scaleBitmapLayerMask)(JNIEnv * jenv, jobject, jlong hLayer, jfloat factor, jfloat fixx, jfloat fixy) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene::CustomMaskedBitmapLayer, layer, hLayer);
    layer->getMaskMapping().scale((float)factor, Beatmup::Point((float)fixx,(float)fixy));
}


JNIMETHOD(void, rotateBitmapLayerMask, Java_Beatmup_Rendering_Scene, rotateBitmapLayerMask)(JNIEnv * jenv, jobject, jlong hLayer, jfloat a, jfloat fixx, jfloat fixy) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene::CustomMaskedBitmapLayer, layer, hLayer);
    layer->getMaskMapping().rotateDegrees((float)a, Beatmup::Point((float)fixx, (float)fixy));
}


JNIMETHOD(void, skewBitmapLayerMask, Java_Beatmup_Rendering_Scene, skewBitmapLayerMask)(JNIEnv * jenv, jobject, jlong hLayer, jfloat x, jfloat y) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene::CustomMaskedBitmapLayer, layer, hLayer);
    layer->getMaskMapping().matrix.skewDegrees((float)x, (float)y);
}


JNIMETHOD(void, setBitmapLayerBgColor, Java_Beatmup_Rendering_Scene, setBitmapLayerBgColor)(JNIEnv * jenv, jobject, jlong hLayer, jfloat r, jfloat g, jfloat b, jfloat a) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene::CustomMaskedBitmapLayer, layer, hLayer);
    layer->setBackgroundColor(Beatmup::pixfloat4(r, g, b, a));
}


JNIMETHOD(void, getBitmapLayerBgColor, Java_Beatmup_Rendering_Scene, getBitmapLayerBgColor)(JNIEnv * jenv, jobject, jlong hLayer, jobject jColor) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene::CustomMaskedBitmapLayer, layer, hLayer);
    $pool.factory.setColor(jenv, layer->getBackgroundColor(), jColor);
}


JNIMETHOD(void, setBitmapLayerImageTransform, Java_Beatmup_Rendering_Scene, setBitmapLayerImageTransform)(JNIEnv * jenv, jobject, jlong hLayer, jfloat x, jfloat y, jfloat a11, jfloat a12, jfloat a21, jfloat a22) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene::BitmapLayer, layer, hLayer);
    Beatmup::AffineMapping& mapping = layer->getBitmapMapping();
    mapping.position.x = x;
    mapping.position.y = y;
    mapping.matrix.setElements((float)a11, (float)a12, (float)a21, (float)a22);
}


JNIMETHOD(void, getBitmapLayerImageTransform, Java_Beatmup_Rendering_Scene, getBitmapLayerImageTransform)(JNIEnv * jenv, jobject, jlong hLayer, jobject mapping) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene::BitmapLayer, layer, hLayer);
    $pool.factory.setAffineMapping(jenv, layer->getBitmapMapping(), mapping);
}


JNIMETHOD(void, setBitmapLayerMaskTransform, Java_Beatmup_Rendering_Scene, setBitmapLayerMaskTransform)(JNIEnv * jenv, jobject, jlong hLayer, jfloat x, jfloat y, jfloat a11, jfloat a12, jfloat a21, jfloat a22) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene::CustomMaskedBitmapLayer, layer, hLayer);
    Beatmup::AffineMapping& mapping = layer->getMaskMapping();
    mapping.position.x = x;
    mapping.position.y = y;
    mapping.matrix.setElements((float)a11, (float)a12, (float)a21, (float)a22);
}


JNIMETHOD(void, getBitmapLayerMaskTransform, Java_Beatmup_Rendering_Scene, getBitmapLayerMaskTransform)(JNIEnv * jenv, jobject, jlong hLayer, jobject mapping) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene::CustomMaskedBitmapLayer, layer, hLayer);
    $pool.factory.setAffineMapping(jenv, layer->getMaskMapping(), mapping);
}


JNIMETHOD(void, setBitmapLayerBitmap, Java_Beatmup_Rendering_Scene, setBitmapLayerBitmap)(JNIEnv * jenv, jobject, jlong hLayer, jobject jBitmap) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene::BitmapLayer, layer, hLayer);
    // deleting a global reference if layer has already a bitmap
    if (!jBitmap)
        layer->setBitmap(nullptr);
    else {
        // creating global reference and setting new bitmap
        BEATMUP_OBJ(Beatmup::AbstractBitmap, bitmap, jBitmap);
        layer->setBitmap(bitmap);
    }
}

JNIMETHOD(void, setMaskedBitmapLayerMask, Java_Beatmup_Rendering_Scene, setMaskedBitmapLayerMask)(JNIEnv * jenv, jobject, jlong hLayer, jobject jMask) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene::MaskedBitmapLayer, layer, hLayer);
    if (!jMask)
        layer->setMask(nullptr);
    else {
        BEATMUP_OBJ(Beatmup::AbstractBitmap, mask, jMask);
        layer->setMask(mask);
    }
}

/**
    Sets shaped bitmap mask corners radius
 */
JNIMETHOD(void, setShapedBitmapLayerCornerRadius, Java_Beatmup_Rendering_Scene, setShapedBitmapLayerCornerRadius)(JNIEnv * jenv, jobject, jlong hLayer, jfloat radius) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene::ShapedBitmapLayer, layer, hLayer);
    layer->setCornerRadius((float) radius);
}


JNIMETHOD(jfloat, getShapedBitmapLayerCornerRadius, Java_Beatmup_Rendering_Scene, getShapedBitmapLayerCornerRadius)(JNIEnv * jenv, jobject, jlong hLayer) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene::ShapedBitmapLayer, layer, hLayer);
    return (jfloat)layer->getCornerRadius();
}


JNIMETHOD(void, setShapedBitmapLayerBorderWidth, Java_Beatmup_Rendering_Scene, setShapedBitmapLayerBorderWidth)(JNIEnv * jenv, jobject, jlong hLayer, jfloat width) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene::ShapedBitmapLayer, layer, hLayer);
    layer->setBorderWidth((float) width);
}


JNIMETHOD(jfloat, getShapedBitmapLayerBorderWidth, Java_Beatmup_Rendering_Scene, getShapedBitmapLayerBorderWidth)(JNIEnv * jenv, jobject, jlong hLayer) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene::ShapedBitmapLayer, layer, hLayer);
    return (jfloat) layer->getBorderWidth();
}


JNIMETHOD(void, setShapedBitmapLayerSlopeWidth, Java_Beatmup_Rendering_Scene, setShapedBitmapLayerSlopeWidth)(JNIEnv * jenv, jobject, jlong hLayer, jfloat width) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene::ShapedBitmapLayer, layer, hLayer);
    layer->setSlopeWidth((float) width);
}


JNIMETHOD(jfloat, getShapedBitmapLayerSlopeWidth, Java_Beatmup_Rendering_Scene, getShapedBitmapLayerSlopeWidth)(JNIEnv * jenv, jobject, jlong hLayer) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene::ShapedBitmapLayer, layer, hLayer);
    return (jfloat) layer->getSlopeWidth();
}


JNIMETHOD(void, setShapedBitmapLayerInPixelsSwitch, Java_Beatmup_Rendering_Scene, setShapedBitmapLayerInPixelsSwitch)(JNIEnv * jenv, jobject, jlong hLayer, jboolean inPixels) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene::ShapedBitmapLayer, layer, hLayer);
    layer->setInPixels(inPixels == JNI_TRUE);
}


JNIMETHOD(jboolean, getShapedBitmapLayerInPixelsSwitch, Java_Beatmup_Rendering_Scene, getShapedBitmapLayerInPixelsSwitch)(JNIEnv * jenv, jobject, jlong hLayer) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene::ShapedBitmapLayer, layer, hLayer);
    return layer->getInPixels() ? JNI_TRUE : JNI_FALSE;
}


JNIMETHOD(void, setShadedBitmapLayerShader, Java_Beatmup_Rendering_Scene, setShadedBitmapLayerShader)(JNIEnv * jenv, jobject, jlong hLayer, jobject jShader) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene::ShadedBitmapLayer, layer, hLayer);
    BEATMUP_OBJ_OR_NULL(Beatmup::ImageShader, shader, jShader);
    layer->setLayerShader(shader);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//                                      SCENE RENDERER
/////////////////////////////////////////////////////////////////////////////////////////////

JNIMETHOD(jlong, newSceneRenderer, Java_Beatmup_Rendering_SceneRenderer, newSceneRenderer)(JNIEnv * jenv, jclass, jobject jEnv) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Android::Context, ctx, jEnv);
    Beatmup::SceneRenderer* renderer = new Beatmup::SceneRenderer();
    return (jlong)renderer;
}


JNIMETHOD(void, setOutput, Java_Beatmup_Rendering_SceneRenderer, setOutput)(JNIEnv * jenv, jobject, jlong hRenderer, jobject jBitmap) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::AbstractBitmap, bitmap, jBitmap);
    BEATMUP_OBJ(Beatmup::SceneRenderer, renderer, hRenderer);
    renderer->setOutput(*bitmap);
}


JNIMETHOD(void, resetOutput, Java_Beatmup_Rendering_SceneRenderer, resetOutput)(JNIEnv * jenv, jobject, jlong hRenderer) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::SceneRenderer, renderer, hRenderer);
    renderer->resetOutput();
}

JNIMETHOD(void, setScene, Java_Beatmup_Rendering_SceneRenderer, setScene)
    (JNIEnv * jenv, jobject, jlong hRenderer, jobject jScene)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Scene, scene, jScene);
    BEATMUP_OBJ(Beatmup::SceneRenderer, renderer, hRenderer);
    renderer->setScene(*scene);
}


JNIMETHOD(void, setOutputMapping, Java_Beatmup_Rendering_SceneRenderer, setOutputMapping)(JNIEnv * jenv, jobject, jlong hRenderer, jint mapping) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::SceneRenderer, renderer, hRenderer);
    renderer->setOutputMapping((Beatmup::SceneRenderer::OutputMapping) mapping);
}


JNIMETHOD(jint, getOutputMapping, Java_Beatmup_Rendering_SceneRenderer, getOutputMapping)(JNIEnv * jenv, jobject, jlong hRenderer) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::SceneRenderer, renderer, hRenderer);
    return renderer->getOutputMapping();
}


JNIMETHOD(void, setOutputReferenceWidth, Java_Beatmup_Rendering_SceneRenderer, setOutputReferenceWidth)(JNIEnv * jenv, jobject, jlong hRenderer, jint width) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::SceneRenderer, renderer, hRenderer);
    renderer->setOutputReferenceWidth((int)width);
}


JNIMETHOD(jint, getOutputReferenceWidth,
          Java_Beatmup_Rendering_SceneRenderer, getOutputReferenceWidth)(JNIEnv * jenv, jobject, jlong hRenderer) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::SceneRenderer, renderer, hRenderer);
    return renderer->getOutputReferenceWidth();
}


JNIMETHOD(void, setOutputPixelsFetching, Java_Beatmup_Rendering_SceneRenderer, setOutputPixelsFetching)(JNIEnv * jenv, jobject, jlong hRenderer, jboolean fetch) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::SceneRenderer, renderer, hRenderer);
    renderer->setOutputPixelsFetching(fetch == JNI_TRUE);
}


JNIMETHOD(jboolean, getOutputPixelsFetching, Java_Beatmup_Rendering_SceneRenderer, getOutputPixelsFetching)(JNIEnv * jenv, jobject, jlong hRenderer) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::SceneRenderer, renderer, hRenderer);
    return renderer->getOutputPixelsFetching() ? JNI_TRUE : JNI_FALSE;
}


JNIMETHOD(void, setBackgroundBitmap, Java_Beatmup_Rendering_SceneRenderer, setBackgroundBitmap)(JNIEnv * jenv, jobject, jlong hRenderer, jobject jBitmap) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::SceneRenderer, renderer, hRenderer);
    BEATMUP_OBJ(Beatmup::AbstractBitmap, bitmap, jBitmap);
    renderer->setBackgroundImage(bitmap);
}


JNIMETHOD(jobject, pickLayer, Java_Beatmup_Rendering_SceneRenderer, pickLayer)(JNIEnv * jenv, jobject, jlong hRenderer, jfloat x, jfloat y, jboolean normalized) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::SceneRenderer, renderer, hRenderer);
    Beatmup::Scene::Layer* layer = renderer->pickLayer(x, y, normalized == JNI_TRUE);
    if (!layer)
        return NULL;
    return $pool.getJavaReference(layer);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//                                          BITMAP
/////////////////////////////////////////////////////////////////////////////////////////////

/**
    Creates new internally managed bitmap
*/
JNIMETHOD(jlong, newInternalBitmap, Java_Beatmup_Bitmap, newInternalBitmap)(JNIEnv * jenv, jclass, jobject jCtx, jint width, jint height, jint pixelFormat) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Context, ctx, jCtx);
    Beatmup::InternalBitmap* bitmap = new Beatmup::InternalBitmap(*ctx, (Beatmup::PixelFormat)pixelFormat, (int)width, (int)height);
    // ctx is used in internal bitmap destructor, so add an internal ref
    $pool.addJavaReference(jenv, jCtx, bitmap);
    return (jlong) bitmap;
}

/**
    Creates new bitmap from Android bitmap object
*/
JNIMETHOD(jlong, newNativeBitmap, Java_Beatmup_Bitmap, newNativeBitmap)(JNIEnv * jenv, jclass, jobject jCtx, jobject bitmap) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Context, ctx, jCtx);
    return (jlong) new Beatmup::Android::Bitmap(*ctx, jenv, bitmap);
}


/**
    Returns bitmap width in pixels
*/
JNIMETHOD(jint, getWidth, Java_Beatmup_Bitmap, getWidth)(JNIEnv * jenv, jobject, jlong hBitmap) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::AbstractBitmap, bitmap, hBitmap);
    return bitmap->getWidth();
}

/**
    Returns bitmap height in pixels
*/
JNIMETHOD(jint, getHeight, Java_Beatmup_Bitmap, getHeight)(JNIEnv * jenv, jobject, jlong hBitmap) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::AbstractBitmap, bitmap, hBitmap);
    return bitmap->getHeight();
}

/**
    Returns bitmap pixel format
*/
JNIMETHOD(jint, getPixelFormat, Java_Beatmup_Bitmap, getPixelFormat)(JNIEnv * jenv, jobject, jlong hBitmap) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::AbstractBitmap, bitmap, hBitmap);
    return bitmap->getPixelFormat();
}


JNIMETHOD(void, zero, Java_Beatmup_Bitmap, zero)(JNIEnv * jenv, jobject, jlong hBitmap) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::AbstractBitmap, bitmap, hBitmap);
    BEATMUP_CATCH({
        bitmap->zero();
    });
}


JNIMETHOD(void, crop, Java_Beatmup_Bitmap, crop)(JNIEnv *jenv, jobject, jlong hInputBitmap, jlong hOutputBitmap, jint x1, jint y1, jint x2, jint y2, jint left, jint top) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::AbstractBitmap, input, hInputBitmap);
    BEATMUP_OBJ(Beatmup::AbstractBitmap, output, hOutputBitmap);
    Beatmup::Crop crop;
    Beatmup::IntRectangle rect(x1,y1,x2,y2);
    Beatmup::IntPoint outOrigin(left, top);
    crop.setInput(input);
    crop.setOutput(output);
    crop.setCropRect(rect);
    crop.setOutputOrigin(outOrigin);
    input->getContext().performTask(crop);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//                                       EXTERNAL BITMAP
/////////////////////////////////////////////////////////////////////////////////////////////

JNIMETHOD(jlong, newExternalImage, Java_Beatmup_Android_ExternalBitmap, newExternalImage)(JNIEnv *jenv, jclass, jobject jCtx) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Context, ctx, jCtx);
    Beatmup::Android::ExternalBitmap* bitmap = new Beatmup::Android::ExternalBitmap(*ctx);
    // ctx is used in internal bitmap destructor, so add an internal ref
    $pool.addJavaReference(jenv, jCtx, bitmap);
    return (jlong) bitmap;
}


JNIMETHOD(void, bind, Java_Beatmup_Android_ExternalBitmap, bind)(JNIEnv * jenv, jobject jobj, jlong hBitmap) {
    BEATMUP_OBJ(Beatmup::Android::ExternalBitmap, bitmap, hBitmap);
    bitmap->bind(jenv, jobj);
}


JNIMETHOD(void, notifyUpdate, Java_Beatmup_Android_ExternalBitmap, notifyUpdate)(JNIEnv * jenv, jobject, jlong hBitmap, jint width, jint height) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Android::ExternalBitmap, bitmap, hBitmap);
    bitmap->notifyUpdate(width, height);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//                                       BINARY OPERATION
/////////////////////////////////////////////////////////////////////////////////////////////

JNIMETHOD(jlong, newBinaryOperation, Java_Beatmup_Imaging_BinaryOperation, newBinaryOperation)(JNIEnv * jenv, jclass) {
    BEATMUP_ENTER;
    return (jlong) new Beatmup::BitmapBinaryOperation();
}


JNIMETHOD(void, setOperand1, Java_Beatmup_Imaging_BinaryOperation, setOperand1)(JNIEnv * jenv, jobject, jlong hInstance, jobject jBitmap) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::BitmapBinaryOperation, operation, hInstance);
    BEATMUP_OBJ(Beatmup::AbstractBitmap, bitmap, jBitmap);
    operation->setOperand1(bitmap);
}


JNIMETHOD(void, setOperand2, Java_Beatmup_Imaging_BinaryOperation, setOperand2)(JNIEnv * jenv, jobject, jlong hInstance, jobject jBitmap) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::BitmapBinaryOperation, operation, hInstance);
    BEATMUP_OBJ(Beatmup::AbstractBitmap, bitmap, jBitmap);
    operation->setOperand2(bitmap);
}


JNIMETHOD(void, setOutput, Java_Beatmup_Imaging_BinaryOperation, setOutput)(JNIEnv * jenv, jobject, jlong hInstance, jobject jBitmap) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::BitmapBinaryOperation, operation, hInstance);
    BEATMUP_OBJ(Beatmup::AbstractBitmap, bitmap, jBitmap);
    operation->setOutput(bitmap);
}


JNIMETHOD(void, setOperation, Java_Beatmup_Imaging_BinaryOperation, setOperation)(JNIEnv * jenv, jobject, jlong hInstance, jint op) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::BitmapBinaryOperation, operation, hInstance);
    operation->setOperation((Beatmup::BitmapBinaryOperation::Operation)op);
}


JNIMETHOD(void, resetCrop, Java_Beatmup_Imaging_BinaryOperation, resetCrop)(JNIEnv * jenv, jobject, jlong hInstance) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::BitmapBinaryOperation, operation, hInstance);
    operation->resetCrop();
}


JNIMETHOD(void, setCrop, Java_Beatmup_Imaging_BinaryOperation, setCrop)(
        JNIEnv * jenv, jobject, jlong hInstance,
        jint w, jint h, jint op1x, jint op1y, jint op2x, jint op2y, jint outx, jint outy) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::BitmapBinaryOperation, operation, hInstance);
    operation->setCropSize(w, h);
    operation->setOp1Origin(Beatmup::IntPoint(op1x, op1y));
    operation->setOp2Origin(Beatmup::IntPoint(op2x, op2y));
    operation->setOutputOrigin(Beatmup::IntPoint(outx, outy));
}


JNIMETHOD(void, getCrop, Java_Beatmup_Imaging_BinaryOperation, getCrop)(
        JNIEnv * jenv, jobject,
        jlong hInstance, jobject size, jobject op1Origin, jobject op2Origin, jobject outputOrigin) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::BitmapBinaryOperation, operation, hInstance);
    $pool.factory.setIntPoint(jenv, operation->getCropWidth(), operation->getCropHeight(), size);
    $pool.factory.setIntPoint(jenv, operation->getOp1Origin(), op1Origin);
    $pool.factory.setIntPoint(jenv, operation->getOp2Origin(), op2Origin);
    $pool.factory.setIntPoint(jenv, operation->getOutputOrigin(), outputOrigin);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//                                       FLOOD FILL
/////////////////////////////////////////////////////////////////////////////////////////////

JNIMETHOD(jlong, newFloodFill, Java_Beatmup_Imaging_FloodFill, newFloodFill)(JNIEnv * jenv, jclass) {
    BEATMUP_ENTER;
    return (jlong) new Beatmup::FloodFill();
}


JNIMETHOD(void, setInput, Java_Beatmup_Imaging_FloodFill, setInput)(JNIEnv * jenv, jobject, jlong hInstance, jobject jBitmap) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::FloodFill, floodFill, hInstance);
    BEATMUP_OBJ(Beatmup::AbstractBitmap, bitmap, jBitmap);
    floodFill->setInput(*bitmap);
}


JNIMETHOD(void, setOutput, Java_Beatmup_Imaging_FloodFill, setOutput)(JNIEnv * jenv, jobject, jlong hInstance, jobject jBitmap) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::FloodFill, floodFill, hInstance);
    BEATMUP_OBJ(Beatmup::AbstractBitmap, bitmap, jBitmap);
    floodFill->setOutput(*bitmap);
}


JNIMETHOD(void, setMaskPos, Java_Beatmup_Imaging_FloodFill, setMaskPos)(JNIEnv * jenv, jobject, jlong hInstance, jint x, jint y) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::FloodFill, floodFill, hInstance);
    Beatmup::IntPoint p((int)x, (int)y);
    floodFill->setMaskPos(p);
}


JNIMETHOD(void, setTolerance, Java_Beatmup_Imaging_FloodFill, setTolerance)(JNIEnv * jenv, jobject, jlong hInstance, jfloat tolerance) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::FloodFill, floodFill, hInstance);
    floodFill->setTolerance((float) tolerance);
}


JNIMETHOD(void, setBorderPostprocessing, Java_Beatmup_Imaging_FloodFill, setBorderPostprocessing)(JNIEnv * jenv, jobject, jlong hInstance, jint op, jfloat hold, jfloat release) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::FloodFill, floodFill, hInstance);
    floodFill->setBorderPostprocessing((Beatmup::FloodFill::BorderMorphology)op, hold, release);
}


JNIMETHOD(void, setSeeds, Java_Beatmup_Imaging_FloodFill, setSeeds)(JNIEnv * jenv, jobject, jlong hInstance, jintArray xy) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::FloodFill, floodFill, hInstance);
    jint* xyPtr = jenv->GetIntArrayElements(xy, NULL);
    int count = (int)jenv->GetArrayLength(xy) / 2;
    floodFill->setSeeds(xyPtr, count);
    jenv->ReleaseIntArrayElements(xy, xyPtr, JNI_ABORT);
}


/**
    Enables / disables border contours computation
 */
JNIMETHOD(void, setComputeContours, Java_Beatmup_Imaging_FloodFill, setComputeContours)(JNIEnv * jenv, jobject, jlong hInstance, jboolean flag) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::FloodFill, floodFill, hInstance);
    floodFill->setComputeContours(flag == JNI_TRUE);
}


JNIMETHOD(jint, getContourCount, Java_Beatmup_Imaging_FloodFill, getContourCount)(JNIEnv * jenv, jobject, jlong hInstance) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::FloodFill, floodFill, hInstance);
    return (jint) floodFill->getContourCount();
}


JNIMETHOD(jint, getContourPointCount, Java_Beatmup_Imaging_FloodFill, getContourPointCount)(JNIEnv * jenv, jobject, jlong hInstance, jint n) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::FloodFill, floodFill, hInstance);
    return floodFill->getContour(n).getPointCount();
}


/**
    Returns a layer border component (a contour)
*/
JNIMETHOD(jintArray, getContour, Java_Beatmup_Imaging_FloodFill, getContour)(JNIEnv * jenv, jobject, jlong hInstance, jint seed, jfloat step) {
    std::vector<int> xy;
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::FloodFill, floodFill, hInstance);
    BEATMUP_CATCH({
        const Beatmup::IntegerContour2D& contour = floodFill->getContour(seed);
        if (contour.getPointCount() > 0) {
            float stepLength2 = step*step;
            unsigned int fragmentLength2 = (unsigned int)(stepLength2 + 1);
            Beatmup::IntPoint prev = contour.getPoint(0);
            Beatmup::IntPoint p;
            for (int n = 0; n < contour.getPointCount(); n++) {
                p = contour.getPoint(n);
                fragmentLength2 += (p - prev).hypot2();
                if (fragmentLength2 >= stepLength2) {
                  xy.push_back(p.x);
                  xy.push_back(p.y);
                  fragmentLength2 = 0;
                }
                prev = p;
            }
            if (fragmentLength2 > 0) {
                xy.push_back(p.x);
                xy.push_back(p.y);
            }
        }
    });
    // copying to java
    jintArray result = jenv->NewIntArray(xy.size());
    jint *javaxy = jenv->GetIntArrayElements(result, NULL);
    int pn = 0;
    for (auto e : xy)
        javaxy[pn++] = e;
    jenv->ReleaseIntArrayElements(result, javaxy, 0);
    return result;
}


/**
 * Returns mask bounding box
 */
JNIMETHOD(jobject, getBoundingBox, Java_Beatmup_Imaging_FloodFill, getBoundingBox)(JNIEnv * jenv, jobject, jlong hInstance) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::FloodFill, floodFill, hInstance);
    return $pool.factory.makeIntRectangle(jenv, floodFill->getBounds());
}

/////////////////////////////////////////////////////////////////////////////////////////////
//                                  PIXELWISE FILTER
/////////////////////////////////////////////////////////////////////////////////////////////

JNIMETHOD(void, setBitmaps, Java_Beatmup_Imaging_Filters_Local_PixelwiseFilter, setBitmaps)(JNIEnv * jenv, jobject, jlong hInstance, jobject jInputBitmap, jobject jOutputBitmap) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Filters::PixelwiseFilter, filter, hInstance);
    BEATMUP_OBJ_OR_NULL(Beatmup::AbstractBitmap, input, jInputBitmap);
    BEATMUP_OBJ_OR_NULL(Beatmup::AbstractBitmap, output, jOutputBitmap);
    filter->setBitmaps(input, output);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//                                  COLOR MATRIX TRANSFORM
/////////////////////////////////////////////////////////////////////////////////////////////

JNIMETHOD(jlong, newColorMatrixTransform, Java_Beatmup_Imaging_Filters_Local_ColorMatrixTransform, newColorMatrixTransform)(JNIEnv * jenv, jclass) {
    BEATMUP_ENTER;
    return (jlong) new Beatmup::Filters::ColorMatrix();
}


JNIMETHOD(void, setFromMatrix, Java_Beatmup_Imaging_Filters_Local_ColorMatrixTransform, setFromMatrix)(JNIEnv * jenv, jobject, jlong hInst, jobject jMat) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Filters::ColorMatrix, filter, hInst);
    BEATMUP_OBJ(Beatmup::Color::Matrix, mat, jMat);
    filter->getMatrix() = *mat;
}


JNIMETHOD(void, assignToMatrix, Java_Beatmup_Imaging_Filters_Local_ColorMatrixTransform, assignToMatrix)(JNIEnv * jenv, jobject, jlong hInst, jobject jMat) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Filters::ColorMatrix, filter, hInst);
    BEATMUP_OBJ(Beatmup::Color::Matrix, mat, jMat);
    *mat = filter->getMatrix();
}


JNIMETHOD(void, setCoefficients, Java_Beatmup_Imaging_Filters_Local_ColorMatrixTransform, setCoefficients)
    (JNIEnv * jenv, jobject, jlong hInstance, jint out, jfloat add, jfloat in0, jfloat in1, jfloat in2, jfloat in3)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Filters::ColorMatrix, filter, hInstance);
    BEATMUP_CATCH({
        filter->setCoefficients(out, add, in0, in1, in2, in3);
    });
}


JNIMETHOD(void, setHSVCorrection, Java_Beatmup_Imaging_Filters_Local_ColorMatrixTransform, setHSVCorrection)
    (JNIEnv * jenv, jobject, jlong hInstance, jfloat h, jfloat s, jfloat v)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Filters::ColorMatrix, filter, hInstance);
    filter->setHSVCorrection((float)h, (float)s, (float)v);
}


JNIMETHOD(void, setColorInversion, Java_Beatmup_Imaging_Filters_Local_ColorMatrixTransform, setColorInversion)
    (JNIEnv * jenv, jobject, jlong hInstance, jfloat r, jfloat g, jfloat b, jfloat s, jfloat v)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Filters::ColorMatrix, filter, hInstance);
    Beatmup::color3f preservedColor{ (float)r, (float)g, (float)b };
    filter->setColorInversion(preservedColor, (float)s, (float)v);
}


JNIMETHOD(void, setAllowIntegerApprox, Java_Beatmup_Imaging_Filters_Local_ColorMatrixTransform, setAllowIntegerApprox)(JNIEnv * jenv, jobject, jlong hInstance, jboolean allow) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Filters::ColorMatrix, filter, hInstance);
    filter->allowIntegerApproximations(allow == JNI_TRUE);
}


JNIMETHOD(jboolean, allowIntegerApprox, Java_Beatmup_Imaging_Filters_Local_ColorMatrixTransform, allowIntegerApprox)(JNIEnv * jenv, jobject, jlong hInstance) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Filters::ColorMatrix, filter, hInstance);
    return (jboolean) filter->isIntegerApproximationsAllowed();
}

/////////////////////////////////////////////////////////////////////////////////////////////
//                                      SEPIA
/////////////////////////////////////////////////////////////////////////////////////////////

JNIMETHOD(jlong, newSepia, Java_Beatmup_Imaging_Filters_Local_Sepia, newSepia)(JNIEnv * jenv, jclass) {
    BEATMUP_ENTER;
    return (jlong) new Beatmup::Filters::Sepia();
}

/////////////////////////////////////////////////////////////////////////////////////////////
//                                  IMAGE TUNING
/////////////////////////////////////////////////////////////////////////////////////////////

JNIMETHOD(jlong, newImageTuning, Java_Beatmup_Imaging_Filters_ImageTuning, newImageTuning)(JNIEnv * jenv, jclass) {
    BEATMUP_ENTER;
    return (jlong) new Beatmup::Filters::ImageTuning();
}


JNIMETHOD(void, setHueOffset, Java_Beatmup_Imaging_Filters_ImageTuning, setHueOffset)(JNIEnv * jenv, jobject, jlong hInstance, jfloat v) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Filters::ImageTuning, filter, hInstance);
    filter->setHueOffset((float)v);
}


JNIMETHOD(void, setSaturationFactor, Java_Beatmup_Imaging_Filters_ImageTuning, setSaturationFactor)(JNIEnv * jenv, jobject, jlong hInstance, jfloat v) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Filters::ImageTuning, filter, hInstance);
    filter->setSaturationFactor((float)v);
}


JNIMETHOD(void, setValueFactor, Java_Beatmup_Imaging_Filters_ImageTuning, setValueFactor)(JNIEnv * jenv, jobject, jlong hInstance, jfloat v) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Filters::ImageTuning, filter, hInstance);
    filter->setValueFactor((float)v);
}


JNIMETHOD(void, setBrightness, Java_Beatmup_Imaging_Filters_ImageTuning, setBrightness)(JNIEnv * jenv, jobject, jlong hInstance, jfloat v) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Filters::ImageTuning, filter, hInstance);
    filter->setBrightness((float)v);
}


JNIMETHOD(void, setContrast, Java_Beatmup_Imaging_Filters_ImageTuning, setContrast)(JNIEnv * jenv, jobject, jlong hInstance, jfloat v) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Filters::ImageTuning, filter, hInstance);
    filter->setContrast((float)v);
}


JNIMETHOD(jfloat, getHueOffset, Java_Beatmup_Imaging_Filters_ImageTuning, getHueOffset)(JNIEnv * jenv, jobject, jlong hInstance) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Filters::ImageTuning, filter, hInstance);
    return filter->getHueOffset();
}


JNIMETHOD(jfloat, getSaturationFactor, Java_Beatmup_Imaging_Filters_ImageTuning, getSaturationFactor)(JNIEnv * jenv, jobject, jlong hInstance) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Filters::ImageTuning, filter, hInstance);
    return filter->getSaturationFactor();
}


JNIMETHOD(jfloat, getValueFactor, Java_Beatmup_Imaging_Filters_ImageTuning, getValueFactor)(JNIEnv * jenv, jobject, jlong hInstance) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Filters::ImageTuning, filter, hInstance);
    return filter->getValueFactor();
}


JNIMETHOD(jfloat, getBrightness, Java_Beatmup_Imaging_Filters_ImageTuning, getBrightness)(JNIEnv * jenv, jobject, jlong hInstance) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Filters::ImageTuning, filter, hInstance);
    return filter->getBrightness();
}


JNIMETHOD(jfloat, getContrast, Java_Beatmup_Imaging_Filters_ImageTuning, getContrast)(JNIEnv * jenv, jobject, jlong hInstance) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Filters::ImageTuning, filter, hInstance);
    return filter->getContrast();
}


JNIMETHOD(void, setBitmaps, Java_Beatmup_Imaging_Filters_ImageTuning, setBitmaps)(JNIEnv * jenv, jobject, jlong hInstance, jobject jInputBitmap, jobject jOutputBitmap) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Filters::ImageTuning, filter, hInstance);
    BEATMUP_OBJ_OR_NULL(Beatmup::AbstractBitmap, input, jInputBitmap);
    BEATMUP_OBJ_OR_NULL(Beatmup::AbstractBitmap, output, jOutputBitmap);
    return filter->setBitmaps(input, output);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//                                      RESAMPLER
/////////////////////////////////////////////////////////////////////////////////////////////

JNIMETHOD(jlong, newResampler, Java_Beatmup_Imaging_Filters_Resampler, newResampler)(JNIEnv * jenv, jclass) {
    BEATMUP_ENTER;
    return (jlong) new Beatmup::BitmapResampler();
}


JNIMETHOD(void, setBitmaps, Java_Beatmup_Imaging_Filters_Resampler, setBitmaps)
    (JNIEnv * jenv, jobject, jlong hInstance, jobject jInput, jobject jOutput)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::BitmapResampler, resampler, hInstance);
    BEATMUP_OBJ_OR_NULL(Beatmup::AbstractBitmap, input, jInput);
    BEATMUP_OBJ_OR_NULL(Beatmup::AbstractBitmap, output, jOutput);
    resampler->setBitmaps(input, output);
}


JNIMETHOD(void, setMode, Java_Beatmup_Imaging_Filters_Resampler, setMode)
    (JNIEnv * jenv, jobject, jlong hInstance, jint mode)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::BitmapResampler, resampler, hInstance);
    resampler->setMode((Beatmup::BitmapResampler::Mode)mode);
}


JNIMETHOD(jint, getMode, Java_Beatmup_Imaging_Filters_Resampler, getMode)
    (JNIEnv * jenv, jobject, jlong hInstance)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::BitmapResampler, resampler, hInstance);
    return (jint)resampler->getMode();
}


/////////////////////////////////////////////////////////////////////////////////////////////
//                                      COLOR MATRIX
/////////////////////////////////////////////////////////////////////////////////////////////

JNIMETHOD(jlong, newColorMatrix, Java_Beatmup_Imaging_ColorMatrix, newColorMatrix)(JNIEnv *jenv, jclass) {
    BEATMUP_ENTER;
    return (jlong) new Beatmup::Color::Matrix();
}


JNIMETHOD(void, assign, Java_Beatmup_Imaging_ColorMatrix, assign)(JNIEnv *jenv, jclass, jlong hInstTo, jlong hInstFrom) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Color::Matrix, matFrom, hInstFrom);
    BEATMUP_OBJ(Beatmup::Color::Matrix, matTo, hInstTo);
    *matTo = *matFrom;
}


JNIMETHOD(void, multiply, Java_Beatmup_Imaging_ColorMatrix, multiply)(JNIEnv *jenv, jclass, jlong hLeft, jlong hRight, jlong hResult) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Color::Matrix, matLeft, hLeft);
    BEATMUP_OBJ(Beatmup::Color::Matrix, matRight, hRight);
    BEATMUP_OBJ(Beatmup::Color::Matrix, matResult, hResult);
    *matResult = (*matLeft) * (*matRight);
}


JNIMETHOD(void, setHSVCorrection, Java_Beatmup_Imaging_ColorMatrix, setHSVCorrection)(JNIEnv *jenv, jobject, jlong hInst, jfloat h, jfloat s, jfloat v) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Color::Matrix, mat, hInst);
    *mat = Beatmup::Color::Matrix::getHSVCorrection(h, s, v);
}


JNIMETHOD(void, setColorInversion, Java_Beatmup_Imaging_ColorMatrix, setColorInversion)(JNIEnv *jenv, jobject, jlong hInst, jfloat r, jfloat g, jfloat b, jfloat s, jfloat v) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Color::Matrix, mat, hInst);
    *mat = Beatmup::Color::Matrix::getColorInversion(
            Beatmup::color3f{ r, g, b },
            s, v
    );
}


JNIMETHOD(jfloat, getElement, Java_Beatmup_Imaging_ColorMatrix, getElement)(JNIEnv *jenv, jobject, jlong hInst, jint x, jint y) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Color::Matrix, mat, hInst);
#ifdef BEATMUP_DEBUG
    Beatmup::DebugAssertion::check(x >= 0 && x < 4 && y >= 0 && y < 4, "Matrix element index out of range: %d %d", x, y);
#endif
    return (jfloat) mat->elem[x][y];
}


JNIMETHOD(void, setElement, Java_Beatmup_Imaging_ColorMatrix, setElement)(JNIEnv *jenv, jobject, jlong hInst, jint x, jint y, jfloat v) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Color::Matrix, mat, hInst);
#ifdef BEATMUP_DEBUG
    Beatmup::DebugAssertion::check(x >= 0 && x < 4 && y >= 0 && y < 4, "Matrix element index out of range: %d %d", x, y);
#endif
    mat->elem[x][y] = v;
}


/////////////////////////////////////////////////////////////////////////////////////////////
//                                          SHADER
/////////////////////////////////////////////////////////////////////////////////////////////

JNIMETHOD(jlong, newShader, Java_Beatmup_Shading_Shader, newShader)(JNIEnv * jenv, jclass, jobject jCtx) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Context, ctx, jCtx);
    return (jlong) new Beatmup::ImageShader(*ctx);
}


JNIMETHOD(void, setSourceCode, Java_Beatmup_Shading_Shader, setSourceCode)(JNIEnv * jenv, jobject, jlong hInstance, jstring src) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::ImageShader, shader, hInstance);
    const char* javaChar = jenv->GetStringUTFChars(src, 0);
    shader->setSourceCode(javaChar);
    jenv->ReleaseStringUTFChars(src, javaChar);
}


JNIMETHOD(jstring, getInputImageId, Java_Beatmup_Shading_Shader, getInputImageId)
    (JNIEnv * jenv, jclass)
{
    BEATMUP_ENTER;
    return jenv->NewStringUTF(Beatmup::ImageShader::INPUT_IMAGE_ID.c_str());
}


JNIMETHOD(jstring, getInputImageDeclType, Java_Beatmup_Shading_Shader, getInputImageDeclType)
    (JNIEnv * jenv, jclass)
{
    BEATMUP_ENTER;
    return jenv->NewStringUTF(Beatmup::ImageShader::INPUT_IMAGE_DECL_TYPE.c_str());
}

/////////////////////////////////////////////////////////////////////////////////////////////
//                                      SHADER APPLICATOR
/////////////////////////////////////////////////////////////////////////////////////////////

JNIMETHOD(jlong, newShaderApplicator, Java_Beatmup_Shading_ShaderApplicator, newShaderApplicator)(JNIEnv * jenv, jclass) {
    BEATMUP_ENTER;
    return (jlong) new Beatmup::ShaderApplicator();
}


JNIMETHOD(void, addSampler, Java_Beatmup_Shading_ShaderApplicator, addSampler)
    (JNIEnv * jenv, jobject, jlong hInst, jobject jBitmap, jstring uniformName)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::ShaderApplicator, applicator, hInst);
    BEATMUP_OBJ_OR_NULL(Beatmup::AbstractBitmap, bitmap, jBitmap);
    BEATMUP_STRING(uniformName);
    applicator->addSampler(bitmap, uniformNameStr);
}


JNIMETHOD(void, setOutput, Java_Beatmup_Shading_ShaderApplicator, setOutput)(JNIEnv * jenv, jobject, jlong hInst, jobject jBitmap) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::ShaderApplicator, applicator, hInst);
    BEATMUP_OBJ_OR_NULL(Beatmup::AbstractBitmap, bitmap, jBitmap);
    applicator->setOutputBitmap(bitmap);
}


JNIMETHOD(void, setShader, Java_Beatmup_Shading_ShaderApplicator, setShader)(JNIEnv * jenv, jobject, jlong hInst, jobject jShader) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::ShaderApplicator, applicator, hInst);
    BEATMUP_OBJ_OR_NULL(Beatmup::ImageShader, shader, jShader);
    applicator->setShader(shader);
}
