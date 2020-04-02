#include "wrapper.h"

#include "jniheaders/Beatmup_Object.h"
#include "jniheaders/Beatmup_Visual_Android_BasicDisplay.h"
#include "jniheaders/Beatmup_Context.h"
#include "jniheaders/Beatmup_Android_Context.h"
#include "jniheaders/Beatmup_Object.h"
#include "jniheaders/Beatmup_Pipelining_CustomPipeline.h"
#include "jniheaders/Beatmup_Pipelining_Multitask.h"
#include "jniheaders/Beatmup_Pipelining_TaskHolder.h"
#include "jniheaders/Beatmup_Utils_VariablesBundle.h"
#include "jniheaders/Beatmup_Sequence.h"

#include "android/context.h"
#include "context_event_listener.h"

#include <android/native_window_jni.h>
#include <core/bitmap/tools.h>
#include <core/pipelining/custom_pipeline.h>
#include <core/pipelining/multitask.h>
#include <core/fragments/sequence.h>
#include <core/gpu/variables_bundle.h>
#include <core/gpu/swapper.h>
#include <core/gpu/display_switch.h>

// defining the pool
BeatmupJavaObjectPool $pool;


/////////////////////////////////////////////////////////////////////////////////////////////
//                                          OBJECT
/////////////////////////////////////////////////////////////////////////////////////////////

JNIMETHOD(void, disposeNative, Java_Beatmup_Object, disposeNative)(JNIEnv * jenv, jobject jObj) {
    Beatmup::Object* nativeObj = $pool.getObject<Beatmup::Object>(jenv, jObj);
#ifdef DEBUG_LOGGING
    LOG_I("Disposing object #%lld", (jlong)nativeObj);
#endif
    if (nativeObj) {
        $pool.nullifyHandle(jenv, jObj);
        $pool.removeAllJavaReferences(jenv, nativeObj);
        delete nativeObj;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
//                                          DISPLAY
/////////////////////////////////////////////////////////////////////////////////////////////

JNIMETHOD(jboolean, bindSurfaceToContext, Java_Beatmup_Visual_Android_BasicDisplay, bindSurfaceToContext)(JNIEnv * jenv, jobject, jobject jCtx, jobject surface) {
    BEATMUP_ENTER;
    jclass cls = jenv->FindClass("Beatmup/Android/Context");
    jfieldID surfaceFieldID = jenv->GetFieldID(cls, "glSurface", "Ljava/lang/Object;");
    jenv->DeleteLocalRef(cls);

    // retrieving the context
    Beatmup::Context * ctx = $pool.getObject<Beatmup::Context>(jenv, jCtx);

    // check if the context already has a surface
    jobject actualSurface = jenv->GetObjectField(jCtx, surfaceFieldID);
    if (actualSurface)
        jenv->DeleteLocalRef(actualSurface);

    // binding a new surface
    if (surface) {
        // bind the new one
        jenv->SetObjectField(jCtx, surfaceFieldID, surface);
        ANativeWindow *wnd = ANativeWindow_fromSurface(jenv, surface);
        if (!wnd) {
            LOG_E("Empty surface window got when switching GL display.");
            return JNI_FALSE;
        }
        const bool result = Beatmup::DisplaySwitch::run(*ctx, wnd);
        ANativeWindow_release(wnd);
        return result ? JNI_TRUE : JNI_FALSE;
    }
    // just removing an old one
    else {
        jenv->SetObjectField(jCtx, surfaceFieldID, NULL);
        return Beatmup::DisplaySwitch::run(*ctx, NULL) ? JNI_TRUE : JNI_FALSE;
    }

    return JNI_FALSE;   // never happens
}

/////////////////////////////////////////////////////////////////////////////////////////////
//                                          CONTEXT
/////////////////////////////////////////////////////////////////////////////////////////////

/**
    Initializes a context event listener and attaches it to the context
*/
JNIMETHOD(jlong, attachEventListener, Java_Beatmup_Context, attachEventListener)(JNIEnv * jenv, jclass, jlong hEnv) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Context, ctx, hEnv);
    Beatmup::Context::EventListener* listener = ctx->getEventListener();
    if (listener)
        delete listener;
    listener = new Beatmup::ContextEventListener(jenv);
    ctx->setEventListener(listener);
    return (jlong) (listener);
}


/**
    Removes a context event listener bind to the context
*/
JNIMETHOD(void, detachEventListener, Java_Beatmup_Context, detachEventListener)(JNIEnv * jenv, jclass, jlong h) {
    BEATMUP_ENTER;
    Beatmup::ContextEventListener* listener = $pool.getObject<Beatmup::ContextEventListener>(jenv, h);
    delete listener;
}

/**
    Returns total RAM size in bytes
*/
JNIMETHOD(jlong, getTotalRam, Java_Beatmup_Context, getTotalRam)(JNIEnv * jenv, jclass) {
    BEATMUP_ENTER;
    return (jlong) Beatmup::Context::getTotalRam();
}


/**
    Performs a task
*/
JNIMETHOD(jfloat, performTask, Java_Beatmup_Context, performTask)
    (JNIEnv * jenv, jobject, jlong hEnv, jint poolIdx, jobject jTask)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Android::Context, ctx, hEnv);
    BEATMUP_OBJ(Beatmup::AbstractTask, task, jTask);
    BEATMUP_CATCH({
        return ctx->performTask(*task, (Beatmup::PoolIndex)poolIdx);
    });
    return -1;
}

/**
    Submits a task
*/
JNIMETHOD(jint, submitTask, Java_Beatmup_Context, submitTask)
(JNIEnv * jenv, jobject, jlong hEnv, jint poolIdx, jobject jTask)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Android::Context, ctx, hEnv);
    BEATMUP_OBJ(Beatmup::AbstractTask, task, jTask);
    BEATMUP_CATCH({
        return (jint)ctx->submitTask(*task, (Beatmup::PoolIndex)poolIdx);
    });
    return 0;
}


JNIMETHOD(jint, submitPersistentTask, Java_Beatmup_Context, submitPersistentTask)
(JNIEnv * jenv, jobject, jlong hEnv, jint poolIdx, jobject jTask)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Android::Context, ctx, hEnv);
    BEATMUP_OBJ(Beatmup::AbstractTask, task, jTask);
    BEATMUP_CATCH({
        return ctx->submitPersistentTask(*task, (Beatmup::PoolIndex) poolIdx);
    });
    return 0;
}

/**
     Starts new task or asks for its repetition
 */
JNIMETHOD(void, repeatTask, Java_Beatmup_Context, repeatTask)
    (JNIEnv * jenv, jobject, jlong hEnv, jint poolIdx, jobject jTask, jboolean abort)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Android::Context, ctx, hEnv);
    BEATMUP_OBJ(Beatmup::AbstractTask, task, jTask);
    BEATMUP_CATCH({
        ctx->repeatTask(*task, abort == JNI_TRUE, (Beatmup::PoolIndex)poolIdx);
    });
}

JNIMETHOD(void, waitForJob, Java_Beatmup_Context, waitForJob)
    (JNIEnv * jenv, jobject, jlong hEnv, jint poolIdx, jint job)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Android::Context, ctx, hEnv);
    BEATMUP_CATCH({
        ctx->waitForJob(job, (Beatmup::PoolIndex) poolIdx);
    });
}

JNIMETHOD(jboolean, abortJob, Java_Beatmup_Context, abortJob)
(JNIEnv * jenv, jobject, jlong hEnv, jint poolIdx, jint job)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Android::Context, ctx, hEnv);
    BEATMUP_CATCH({
        return ctx->abortJob(job, (Beatmup::PoolIndex) poolIdx) ? JNI_TRUE : JNI_FALSE;
    });
    return JNI_FALSE;
}

JNIMETHOD(void, waitForAllJobs, Java_Beatmup_Context, waitForAllJobs)
(JNIEnv * jenv, jobject, jlong hEnv, jint poolIdx)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Android::Context, ctx, hEnv);
    BEATMUP_CATCH({
        ctx->wait((Beatmup::PoolIndex) poolIdx);
    });
}

JNIMETHOD(jboolean, busy, Java_Beatmup_Context, busy)
(JNIEnv * jenv, jobject, jlong hEnv, jint poolIdx)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Android::Context, ctx, hEnv);
    BEATMUP_CATCH({
        return ctx->busy((Beatmup::PoolIndex) poolIdx) ? JNI_TRUE : JNI_FALSE;
    });
    return JNI_FALSE;
}

JNIMETHOD(jlong, renderChessboard, Java_Beatmup_Context, renderChessboard)(JNIEnv * jenv, jobject, jlong hEnv,
    jint width, jint height, jint cell, jint pixelFormat)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Android::Context, ctx, hEnv);
    return (jlong) Beatmup::BitmapTools::chessboard(*ctx, (int)width, (int)height, (int)cell, (Beatmup::PixelFormat)pixelFormat);
}


JNIMETHOD(jlong, copyBitmap, Java_Beatmup_Context, copyBitmap)(JNIEnv * jenv, jobject jCtx, jobject jBitmap, jint pixelFormat) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::AbstractBitmap, bitmap, jBitmap);
    Beatmup::AbstractBitmap* copy = Beatmup::BitmapTools::makeCopy(*bitmap, (Beatmup::PixelFormat)pixelFormat);
    // ctx is used in internal bitmap destructor, so add an internal ref
    BEATMUP_REFERENCE(jCtx, copy);
    return (jlong) copy;
}


JNIMETHOD(jobject, scanlineSearchInt, Java_Beatmup_Context, scanlineSearchInt)(JNIEnv * jenv, jobject jCtx, jlong hBitmap, jint x, jint y, jint r, jint g, jint b, jint a) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::AbstractBitmap, bitmap, hBitmap);
    Beatmup::IntPoint result = Beatmup::BitmapTools::scanlineSearch(
            *bitmap,
            Beatmup::pixint4(r, g, b, a),
            Beatmup::IntPoint(x, y)
    );
    return $pool.factory.makeIntPoint(jenv, result);
}


JNIMETHOD(jobject, scanlineSearchFloat, Java_Beatmup_Context, scanlineSearchFloat)(JNIEnv * jenv, jobject jCtx, jlong hBitmap, jint x, jint y, jfloat r, jfloat g, jfloat b, jfloat a) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::AbstractBitmap, bitmap, hBitmap);
    Beatmup::IntPoint result = Beatmup::BitmapTools::scanlineSearch(
            *bitmap,
            Beatmup::pixfloat4(r, g, b, a),
            Beatmup::IntPoint(x, y)
    );
    return $pool.factory.makeIntPoint(jenv, result);
}


JNIMETHOD(jint, maxAllowedWorkerCount, Java_Beatmup_Context, maxAllowedWorkerCount)
    (JNIEnv * jenv, jobject, jlong hEnv, jint poolIdx)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Android::Context, ctx, hEnv);
    return ctx->maxAllowedWorkerCount((Beatmup::PoolIndex) poolIdx);
}


JNIMETHOD(void, limitWorkerCount, Java_Beatmup_Context, limitWorkerCount)
    (JNIEnv * jenv, jobject, jlong hEnv, jint poolIdx, jint count)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Android::Context, ctx, hEnv);
    ctx->limitWorkerCount(Beatmup::AbstractTask::validThreadCount(count), (Beatmup::PoolIndex) poolIdx );
}


JNIMETHOD(jlong, swapOnDisk, Java_Beatmup_Context, swapOnDisk)(JNIEnv * jenv, jobject, jlong hEnv, jlong howMuch) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Android::Context, ctx, hEnv);
    return (jlong) ctx->swapOnDisk((Beatmup::msize)howMuch);
}


JNIMETHOD(void, fetchPixelsFromGPU, Java_Beatmup_Context, fetchPixelsFromGPU)(JNIEnv * jenv, jclass, jlong hBitmap) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::AbstractBitmap, bitmap, hBitmap);
    Beatmup::Swapper::pullPixels(*bitmap);
}


JNIMETHOD(jboolean, isGpuQueried, Java_Beatmup_Context, isGpuQueried)(JNIEnv * jenv, jobject, jlong hEnv) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Android::Context, ctx, hEnv);
    return (jboolean) ctx->isGpuQueried();
}


JNIMETHOD(jboolean, isGpuReady, Java_Beatmup_Context, isGpuReady)(JNIEnv * jenv, jobject, jlong hEnv) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Android::Context, ctx, hEnv);
    return (jboolean) ctx->isGpuReady();
}


JNIMETHOD(void, recycleGPUGarbage, Java_Beatmup_Context, recycleGPUGarbage)(JNIEnv * jenv, jobject, jlong hEnv) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Android::Context, ctx, hEnv);
    ctx->getGpuRecycleBin()->emptyBin();
}


/////////////////////////////////////////////////////////////////////////////////////////////
//                                      ANDROID CONTEXT
/////////////////////////////////////////////////////////////////////////////////////////////

/**
    Creates new Beatmup context
*/
JNIMETHOD(jlong, newEnvironment, Java_Beatmup_Android_Context, newEnvironment)(JNIEnv * jenv, jclass, jint poolCount, jstring filesDir)
{
    LOG_I("Beatmup is starting up...");
    BEATMUP_ENTER;
    if (filesDir) {
        const char *javaChar = jenv->GetStringUTFChars(filesDir, 0);
        const std::string swapPrefix(javaChar);
        jenv->ReleaseStringUTFChars(filesDir, javaChar);
        if (swapPrefix.back() != '/') {
            $pool.throwToJava(jenv, "Slash-ended path is expected");
            return -1;
        }
        return (jlong) new Beatmup::Android::Context(
                jenv,
                (Beatmup::PoolIndex) poolCount,
                (swapPrefix + "swap").c_str(),
                ".tmp"
        );
    }

    else
        return (jlong) new Beatmup::Android::Context(
                jenv,
                (Beatmup::PoolIndex) poolCount,
                nullptr, nullptr
        );
}

/////////////////////////////////////////////////////////////////////////////////////////////
//                                      VARIABLES BUNDLE
/////////////////////////////////////////////////////////////////////////////////////////////

JNIMETHOD(void, setInteger1, Java_Beatmup_Utils_VariablesBundle, setInteger1)(JNIEnv * jenv, jobject, jlong hInstance, jstring name, jint val) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::GL::VariablesBundle, vars, hInstance);
    BEATMUP_STRING(name);
    vars->setInteger(nameStr, val);
}


JNIMETHOD(void, setInteger2, Java_Beatmup_Utils_VariablesBundle, setInteger2)(JNIEnv * jenv, jobject, jlong hInstance, jstring name, jint x, jint y) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::GL::VariablesBundle, vars, hInstance);
    BEATMUP_STRING(name);
    vars->setInteger(nameStr, x, y);
}


JNIMETHOD(void, setInteger3, Java_Beatmup_Utils_VariablesBundle, setInteger3)(JNIEnv * jenv, jobject, jlong hInstance, jstring name, jint x, jint y, jint z) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::GL::VariablesBundle, vars, hInstance);
    BEATMUP_STRING(name);
    vars->setInteger(nameStr, x, y, z);
}


JNIMETHOD(void, setInteger4, Java_Beatmup_Utils_VariablesBundle, setInteger4)(JNIEnv * jenv, jobject, jlong hInstance, jstring name, jint x, jint y, jint z, jint w) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::GL::VariablesBundle, vars, hInstance);
    BEATMUP_STRING(name);
    vars->setInteger(nameStr, x, y, z, w);
}


JNIMETHOD(void, setFloat1, Java_Beatmup_Utils_VariablesBundle, setFloat1)(JNIEnv * jenv, jobject, jlong hInstance, jstring name, jfloat val) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::GL::VariablesBundle, vars, hInstance);
    BEATMUP_STRING(name);
    vars->setFloat(nameStr, val);
}


JNIMETHOD(void, setFloat2, Java_Beatmup_Utils_VariablesBundle, setFloat2)(JNIEnv * jenv, jobject, jlong hInstance, jstring name, jfloat x, jfloat y) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::GL::VariablesBundle, vars, hInstance);
    BEATMUP_STRING(name);
    vars->setFloat(nameStr, x, y);
}


JNIMETHOD(void, setFloat3, Java_Beatmup_Utils_VariablesBundle, setFloat3)(JNIEnv * jenv, jobject, jlong hInstance, jstring name, jfloat x, jfloat y, jfloat z) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::GL::VariablesBundle, vars, hInstance);
    BEATMUP_STRING(name);
    vars->setFloat(nameStr, x, y, z);
}


JNIMETHOD(void, setFloat4, Java_Beatmup_Utils_VariablesBundle, setFloat4)(JNIEnv * jenv, jobject, jlong hInstance, jstring name, jfloat x, jfloat y, jfloat z, jfloat w) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::GL::VariablesBundle, vars, hInstance);
    BEATMUP_STRING(name);
    vars->setFloat(nameStr, x, y, z, w);
}


JNIMETHOD(void, setFloatMatrix, Java_Beatmup_Utils_VariablesBundle, setFloatMatrix)(JNIEnv * jenv, jobject, jlong hInstance, jstring name, jfloatArray array) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::GL::VariablesBundle, vars, hInstance);
    BEATMUP_STRING(name);
    jfloat* arrayData = jenv->GetFloatArrayElements(array, JNI_FALSE);
    int count = (int)jenv->GetArrayLength(array);
    switch (count) {
        case 9:
            vars->setFloatMatrix3(nameStr, arrayData);
            break;
        case 16:
            vars->setFloatMatrix4(nameStr, arrayData);
            break;
        default:
            $pool.throwToJava(jenv, "Invalid matrix size");
    }
    jenv->ReleaseFloatArrayElements(array, arrayData, JNI_ABORT);
}


JNIMETHOD(void, setFloatMatrixFromColorMatrix, Java_Beatmup_Utils_VariablesBundle, setFloatMatrixFromColorMatrix)(JNIEnv * jenv, jobject, jlong hInstance, jstring name, jobject jMat) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::GL::VariablesBundle, vars, hInstance);
    BEATMUP_OBJ(Beatmup::Color::Matrix, mat, jMat);
    BEATMUP_STRING(name);
    vars->setFloatMatrix4(nameStr, *mat);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//                                      CUSTOM PIPELINE
/////////////////////////////////////////////////////////////////////////////////////////////

JNIMETHOD(jint, getTaskCount, Java_Beatmup_Pipelining_CustomPipeline, getTaskCount)(JNIEnv * jenv, jobject, jlong hInst) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::CustomPipeline, pipeline, hInst);
    return (jint)pipeline->getTaskCount();
}


JNIMETHOD(jobject, getTask, Java_Beatmup_Pipelining_CustomPipeline, getTask)(JNIEnv * jenv, jobject, jlong hInst, jint index) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::CustomPipeline, pipeline, hInst);
    return $pool.getJavaReference(&pipeline->getTask(index));
}


JNIMETHOD(jint, getTaskIndex, Java_Beatmup_Pipelining_CustomPipeline, getTaskIndex)(JNIEnv * jenv, jobject, jlong hInst, jlong hHolder) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::CustomPipeline, pipeline, hInst);
    BEATMUP_OBJ(Beatmup::CustomPipeline::TaskHolder, taskHolder, hHolder);
    return pipeline->getTaskIndex(*taskHolder);
}


JNIMETHOD(jlong, addTask, Java_Beatmup_Pipelining_CustomPipeline, addTask)(JNIEnv * jenv, jobject, jlong hInst, jobject jHolder, jobject jTask) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::CustomPipeline, pipeline, hInst);
    BEATMUP_OBJ(Beatmup::AbstractTask, task, jTask);
    Beatmup::CustomPipeline::TaskHolder* newbie = &pipeline->addTask(*task);
    BEATMUP_REFERENCE(jHolder, newbie);
    return (jlong)(newbie);
}


JNIMETHOD(jlong, insertTask, Java_Beatmup_Pipelining_CustomPipeline, insertTask)
    (JNIEnv * jenv, jobject, jlong hInst, jobject jHolder, jobject jTask, jlong hWhere)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::CustomPipeline, pipeline, hInst);
    BEATMUP_OBJ(Beatmup::AbstractTask, task, jTask);
    BEATMUP_OBJ(Beatmup::CustomPipeline::TaskHolder, where, hWhere);
    Beatmup::CustomPipeline::TaskHolder* newbie = &pipeline->insertTask(*task, *where);
    BEATMUP_REFERENCE(jHolder, newbie);
    return (jlong)(newbie);
}


JNIMETHOD(jboolean, removeTask, Java_Beatmup_Pipelining_CustomPipeline, removeTask)(JNIEnv * jenv, jobject, jlong hInst, jlong hHolder) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::CustomPipeline, pipeline, hInst);
    BEATMUP_OBJ(Beatmup::CustomPipeline::TaskHolder, what, hHolder);
    bool result = pipeline->removeTask(*what);
    if (result)
        BEATMUP_DELETE_REFERENCE(what);
    return result;
}

JNIMETHOD(void, measure, Java_Beatmup_Pipelining_CustomPipeline, measure)(JNIEnv * jenv, jobject, jlong hInst) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::CustomPipeline, pipeline, hInst);
    pipeline->measure();
}

/////////////////////////////////////////////////////////////////////////////////////////////
//                                          MULTITASK
/////////////////////////////////////////////////////////////////////////////////////////////

JNIMETHOD(jlong, newMultitask, Java_Beatmup_Pipelining_Multitask, newMultitask)(JNIEnv * jenv, jclass, jobject jCtx) {
    BEATMUP_ENTER;
    return (jlong)(new Beatmup::Multitask());
}


JNIMETHOD(jint, getRepetitionPolicy, Java_Beatmup_Pipelining_Multitask, getRepetitionPolicy)(JNIEnv * jenv, jobject, jlong hInst, jlong hHolder) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Multitask, multitask, hInst);
    BEATMUP_OBJ(Beatmup::CustomPipeline::TaskHolder, taskHolder, hHolder);
    BEATMUP_CATCH({
        return static_cast<jint>(multitask->getRepetitionPolicy(*taskHolder));
    });
}


JNIMETHOD(void, setRepetitionPolicy, Java_Beatmup_Pipelining_Multitask, setRepetitionPolicy)(JNIEnv * jenv, jobject, jlong hInst, jlong hHolder, jint policy) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Multitask, multitask, hInst);
    BEATMUP_OBJ(Beatmup::CustomPipeline::TaskHolder, taskHolder, hHolder);
    BEATMUP_CATCH({
        multitask->setRepetitionPolicy(*taskHolder, static_cast<Beatmup::Multitask::RepetitionPolicy>(policy));
    });
}

/////////////////////////////////////////////////////////////////////////////////////////////
//                                         TASK HOLDER
/////////////////////////////////////////////////////////////////////////////////////////////

JNIMETHOD(jfloat, getRunTime, Java_Beatmup_Pipelining_TaskHolder, getRunTime) (JNIEnv * jenv, jobject, jlong hHolder) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::CustomPipeline::TaskHolder, taskHolder, hHolder);
    return (jfloat) taskHolder->getRunTime();
}

/////////////////////////////////////////////////////////////////////////////////////////////
//                                          SEQUENCE
/////////////////////////////////////////////////////////////////////////////////////////////

#if !defined(BEATMUP_PROFILE_NOAUDIO) || !defined(BEATMUP_PROFILE_NOVIDEO)

JNIMETHOD(jlong, copy, Java_Beatmup_Sequence, copy)(JNIEnv * jenv, jobject, jlong hSeq, jint start, jint end) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Fragments::Sequence, seq, hSeq);
    return (jlong) seq->copy(start, end);
}

JNIMETHOD(void, insert, Java_Beatmup_Sequence, insert)(JNIEnv * jenv, jobject, jlong hDestSeq, jlong hInsertSeq, jint time) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Fragments::Sequence, destSeq, hDestSeq);
    BEATMUP_OBJ(Beatmup::Fragments::Sequence, insSeq, hInsertSeq);
    destSeq->insert(*insSeq, time);
}

JNIMETHOD(void, remove, Java_Beatmup_Sequence, remove)(JNIEnv * jenv, jobject, jlong hSeq, jint start, jint end) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Fragments::Sequence, seq, hSeq);
    seq->remove(start, end);
}

JNIMETHOD(void, shrink, Java_Beatmup_Sequence, shrink)(JNIEnv * jenv, jobject, jlong hSeq, jint start, jint end) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Fragments::Sequence, seq, hSeq);
    seq->shrink(start, end);
}

#endif
