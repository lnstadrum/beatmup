/*
    Beatmup image and signal processing library
    Copyright (C) 2019, lnstadrum

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "wrapper.h"

#include "include/Beatmup_Object.h"
#include "include/Beatmup_Visual_Android_BasicDisplay.h"
#include "include/Beatmup_Context.h"
#include "include/Beatmup_Android_Context.h"
#include "include/Beatmup_Object.h"
#include "include/Beatmup_Pipelining_CustomPipeline.h"
#include "include/Beatmup_Pipelining_Multitask.h"
#include "include/Beatmup_Pipelining_TaskHolder.h"
#include "include/Beatmup_Utils_Callback.h"
#include "include/Beatmup_Utils_ChunkAsset.h"
#include "include/Beatmup_Utils_ChunkCollection.h"
#include "include/Beatmup_Utils_ChunkFile.h"
#include "include/Beatmup_Utils_VariablesBundle.h"
#include "include/Beatmup_Sequence.h"

#include "android/context.h"
#include "context_event_listener.h"
#include "callback_task.h"

#include <android/asset_manager_jni.h>
#include <android/native_window_jni.h>

#include <core/bitmap/tools.h>
#include <core/pipelining/custom_pipeline.h>
#include <core/pipelining/multitask.h>
#include <core/fragments/sequence.h>
#include <core/gpu/variables_bundle.h>
#include <core/gpu/swapper.h>
#include <core/gpu/display_switch.h>
#include <core/utils/android/asset.h>


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

    // retrieving the context
    Beatmup::Context * ctx = $pool.getObject<Beatmup::Context>(jenv, jCtx);

    // binding a new surface
    if (surface) {
        // bind the new one
        ANativeWindow *wnd = ANativeWindow_fromSurface(jenv, surface);
        if (!wnd) {
            LOG_E("Empty surface window got when switching GL display.");
            return JNI_FALSE;
        }

        BEATMUP_CATCH({
            const bool result = Beatmup::DisplaySwitch::run(*ctx, wnd);
            ANativeWindow_release(wnd);
            return result ? JNI_TRUE : JNI_FALSE;
        });
    }

    // just removing the old one
    else {
        BEATMUP_CATCH({
            return Beatmup::DisplaySwitch::run(*ctx, nullptr) ? JNI_TRUE : JNI_FALSE;
        });
    }

    return JNI_FALSE;   // never happens
}

/////////////////////////////////////////////////////////////////////////////////////////////
//                                          CONTEXT
/////////////////////////////////////////////////////////////////////////////////////////////

JNIMETHOD(jlong, attachEventListener, Java_Beatmup_Context, attachEventListener)(JNIEnv * jenv, jclass, jlong hCtx) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Context, ctx, hCtx);
    Beatmup::Context::EventListener* listener = ctx->getEventListener();
    if (listener)
        delete listener;
    listener = new Beatmup::ContextEventListener(jenv);
    ctx->setEventListener(listener);
    return (jlong) (listener);
}

JNIMETHOD(void, detachEventListener, Java_Beatmup_Context, detachEventListener)(JNIEnv * jenv, jclass, jlong h) {
    BEATMUP_ENTER;
    Beatmup::ContextEventListener* listener = $pool.getObject<Beatmup::ContextEventListener>(jenv, h);
    delete listener;
}

JNIMETHOD(jlong, getTotalRam, Java_Beatmup_Context, getTotalRam)(JNIEnv * jenv, jclass) {
    BEATMUP_ENTER;
    return (jlong) Beatmup::AlignedMemory::total();
}

JNIMETHOD(jfloat, performTask, Java_Beatmup_Context, performTask)
    (JNIEnv * jenv, jobject, jlong hCtx, jint poolIdx, jobject jTask)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Android::Context, ctx, hCtx);
    BEATMUP_OBJ(Beatmup::AbstractTask, task, jTask);
    BEATMUP_CATCH({
        return ctx->performTask(*task, (Beatmup::PoolIndex)poolIdx);
    });
    return -1;
}

JNIMETHOD(jint, submitTask, Java_Beatmup_Context, submitTask)
    (JNIEnv * jenv, jobject, jlong hCtx, jint poolIdx, jobject jTask)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Android::Context, ctx, hCtx);
    BEATMUP_OBJ(Beatmup::AbstractTask, task, jTask);
    BEATMUP_CATCH({
        return (jint)ctx->submitTask(*task, (Beatmup::PoolIndex)poolIdx);
    });
    return 0;
}

JNIMETHOD(jint, submitPersistentTask, Java_Beatmup_Context, submitPersistentTask)
    (JNIEnv * jenv, jobject, jlong hCtx, jint poolIdx, jobject jTask)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Android::Context, ctx, hCtx);
    BEATMUP_OBJ(Beatmup::AbstractTask, task, jTask);
    BEATMUP_CATCH({
        return ctx->submitPersistentTask(*task, (Beatmup::PoolIndex) poolIdx);
    });
    return 0;
}

JNIMETHOD(void, repeatTask, Java_Beatmup_Context, repeatTask)
    (JNIEnv * jenv, jobject, jlong hCtx, jint poolIdx, jobject jTask, jboolean abort)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Android::Context, ctx, hCtx);
    BEATMUP_OBJ(Beatmup::AbstractTask, task, jTask);
    BEATMUP_CATCH({
        ctx->repeatTask(*task, abort == JNI_TRUE, (Beatmup::PoolIndex)poolIdx);
    });
}

JNIMETHOD(void, waitForJob, Java_Beatmup_Context, waitForJob)
    (JNIEnv * jenv, jobject, jlong hCtx, jint poolIdx, jint job)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Android::Context, ctx, hCtx);
    BEATMUP_CATCH({
        ctx->waitForJob(job, (Beatmup::PoolIndex)poolIdx);
    });
}

JNIMETHOD(jboolean, abortJob, Java_Beatmup_Context, abortJob)
    (JNIEnv * jenv, jobject, jlong hCtx, jint poolIdx, jint job)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Android::Context, ctx, hCtx);
    BEATMUP_CATCH({
        return ctx->abortJob(job, (Beatmup::PoolIndex)poolIdx) ? JNI_TRUE : JNI_FALSE;
    });
    return JNI_FALSE;
}

JNIMETHOD(void, waitForAllJobs, Java_Beatmup_Context, waitForAllJobs)
    (JNIEnv * jenv, jobject, jlong hCtx, jint poolIdx)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Android::Context, ctx, hCtx);
    BEATMUP_CATCH({
        ctx->wait((Beatmup::PoolIndex)poolIdx);
    });
}

JNIMETHOD(jboolean, busy, Java_Beatmup_Context, busy)
    (JNIEnv * jenv, jobject, jlong hCtx, jint poolIdx)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Android::Context, ctx, hCtx);
    BEATMUP_CATCH({
        return ctx->busy((Beatmup::PoolIndex)poolIdx) ? JNI_TRUE : JNI_FALSE;
    });
    return JNI_FALSE;
}

JNIMETHOD(void, check, Java_Beatmup_Context, check)
    (JNIEnv * jenv, jobject, jlong hCtx, jint poolIdx)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Android::Context, ctx, hCtx);
    BEATMUP_CATCH({
        ctx->check((Beatmup::PoolIndex)poolIdx);
    });
}


JNIMETHOD(jlong, renderChessboard, Java_Beatmup_Context, renderChessboard)(JNIEnv * jenv, jobject, jlong hCtx,
    jint width, jint height, jint cell, jint pixelFormat)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Android::Context, ctx, hCtx);
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
    (JNIEnv * jenv, jobject, jlong hCtx, jint poolIdx)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Android::Context, ctx, hCtx);
    return ctx->maxAllowedWorkerCount((Beatmup::PoolIndex) poolIdx);
}


JNIMETHOD(void, limitWorkerCount, Java_Beatmup_Context, limitWorkerCount)
    (JNIEnv * jenv, jobject, jlong hCtx, jint poolIdx, jint count)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Android::Context, ctx, hCtx);
    ctx->limitWorkerCount(Beatmup::AbstractTask::validThreadCount(count), (Beatmup::PoolIndex) poolIdx );
}


JNIMETHOD(jboolean, isGpuQueried, Java_Beatmup_Context, isGpuQueried)(JNIEnv * jenv, jobject, jlong hCtx) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Android::Context, ctx, hCtx);
    return (jboolean) ctx->isGpuQueried();
}


JNIMETHOD(jboolean, isGpuReady, Java_Beatmup_Context, isGpuReady)(JNIEnv * jenv, jobject, jlong hCtx) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Android::Context, ctx, hCtx);
    return (jboolean) ctx->isGpuReady();
}


JNIMETHOD(void, recycleGPUGarbage, Java_Beatmup_Context, recycleGPUGarbage)(JNIEnv * jenv, jobject, jlong hCtx) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Android::Context, ctx, hCtx);
    ctx->getGpuRecycleBin()->emptyBin();
}


/////////////////////////////////////////////////////////////////////////////////////////////
//                                      ANDROID CONTEXT
/////////////////////////////////////////////////////////////////////////////////////////////

JNIMETHOD(jlong, newContext, Java_Beatmup_Android_Context, newContext)(JNIEnv * jenv, jclass, jint poolCount)
{
    LOG_I("Beatmup is starting up...");
    BEATMUP_ENTER;
    return (jlong) new Beatmup::Android::Context(jenv, (Beatmup::PoolIndex) poolCount);
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
    return 0;
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

/////////////////////////////////////////////////////////////////////////////////////////////
//                                      CHUNK COLLECTION
/////////////////////////////////////////////////////////////////////////////////////////////

JNIMETHOD(void, open, Java_Beatmup_Utils_ChunkCollection, open)
    (JNIEnv * jenv, jobject, jlong hCollection)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::ChunkCollection, collection, hCollection);
    try {
        collection->open();
    }
    catch (Beatmup::Exception& ex) { $pool.throwToJava(jenv, "java/io/IOError", ex.what()); }
}


JNIMETHOD(void, close, Java_Beatmup_Utils_ChunkCollection, close)
    (JNIEnv * jenv, jobject, jlong hCollection)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::ChunkCollection, collection, hCollection);
    collection->close();
}


JNIMETHOD(jlong, size, Java_Beatmup_Utils_ChunkCollection, size)
    (JNIEnv * jenv, jobject, jlong hCollection)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::ChunkCollection, collection, hCollection);
    return (jlong)collection->size();
}


JNIMETHOD(jboolean, chunkExists, Java_Beatmup_Utils_ChunkCollection, chunkExists)
    (JNIEnv * jenv, jobject, jlong hCollection, jstring id)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::ChunkCollection, collection, hCollection);
    BEATMUP_STRING(id);
    return collection->chunkExists(idStr) ? JNI_TRUE : JNI_FALSE;
}


JNIMETHOD(jlong, chunkSize, Java_Beatmup_Utils_ChunkCollection, chunkSize)
    (JNIEnv * jenv, jobject, jlong hCollection, jstring id)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::ChunkCollection, collection, hCollection);
    BEATMUP_STRING(id);
    return (jlong)collection->chunkSize(idStr);
}


JNIMETHOD(void, save, Java_Beatmup_Utils_ChunkCollection, save)
    (JNIEnv * jenv, jobject, jlong hCollection, jstring filename, jboolean append)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::ChunkCollection, collection, hCollection);
    BEATMUP_STRING(filename);
    collection->save(filenameStr, append == JNI_TRUE);
}


JNIMETHOD(jstring, read, Java_Beatmup_Utils_ChunkCollection, read)
    (JNIEnv * jenv, jobject, jlong hCollection, jstring id)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::ChunkCollection, collection, hCollection);
    BEATMUP_STRING(id);
    std::string content = collection->template read<std::string>(idStr);
    return jenv->NewStringUTF(content.c_str());
}

/////////////////////////////////////////////////////////////////////////////////////////////
//                                         CHUNK ASSET
/////////////////////////////////////////////////////////////////////////////////////////////

JNIMETHOD(jlong, newChunkAsset, Java_Beatmup_Utils_ChunkAsset, newChunkAsset)
    (JNIEnv * jenv, jclass, jobject jAssetManager, jstring filename)
{
    AAssetManager *assetManager = AAssetManager_fromJava(jenv, jAssetManager);
    BEATMUP_ENTER;
    BEATMUP_STRING(filename);
    try {
        Beatmup::Android::ChunkAsset* instance = new Beatmup::Android::ChunkAsset(assetManager, filenameStr);
        BEATMUP_REFERENCE(jAssetManager, instance);
        return (jlong)instance;
    }
    catch (Beatmup::Exception& ex) { $pool.throwToJava(jenv, "java/io/IOError", ex.what()); }
    return BeatmupJavaObjectPool::INVALID_HANDLE;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//                                         CHUNK FILE
/////////////////////////////////////////////////////////////////////////////////////////////

JNIMETHOD(jlong, newChunkfile, Java_Beatmup_Utils_ChunkFile, newChunkfile)
    (JNIEnv * jenv, jclass, jstring filename, jboolean openNow)
{
    BEATMUP_ENTER;
    BEATMUP_STRING(filename);
    try {
        return (jlong)new Beatmup::ChunkFile(filenameStr, openNow == JNI_TRUE);
    }
    catch (Beatmup::Exception& ex) { $pool.throwToJava(jenv, "java/io/IOError", ex.what()); }
    return BeatmupJavaObjectPool::INVALID_HANDLE;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//                                          CALLBACK
/////////////////////////////////////////////////////////////////////////////////////////////

JNIMETHOD(jlong, newCallbackTask, Java_Beatmup_Utils_Callback, newCallbackTask)
    (JNIEnv * jenv, jclass)
{
    BEATMUP_ENTER;
    return (jlong)new Beatmup::CallbackTask(jenv);
}

JNIMETHOD(void, updateCallback, Java_Beatmup_Utils_Callback, updateCallback)
    (JNIEnv * jenv, jobject jTask)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::CallbackTask, task, jTask);
    task->setCallbackObject(jenv, jTask);
}


// A handy regexp to transform generated JNI declarations into dummy definitions:
//    search for        (/\*\n(\s\*.*\n)+\s\*/\n)?JNIEXPORT ([^\s]+) JNICALL ([^\s]+)_([^\s]+)\n\s+\(JNIEnv \*,([^\)]+)\);
//    replace with      JNIMETHOD(\3, \5, \4, \5)\n    (JNIEnv * jenv,\6)\n{\n    BEATMUP_ENTER;\n}\n
