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

#include "include/Beatmup_NNets_AbstractOperation.h"
#include "include/Beatmup_NNets_DeserializedModel.h"
#include "include/Beatmup_NNets_InferenceTask.h"
#include "include/Beatmup_NNets_ImageSampler.h"
#include "include/Beatmup_NNets_Model.h"
#include "include/Beatmup_NNets_Softmax.h"

#include "android/context.h"

#include <core/nnets/deserialized_model.h>
#include <core/nnets/inference_task.h>

#include <iostream>
#include <sstream>

/////////////////////////////////////////////////////////////////////////////////////////////
//                                       MODEL
/////////////////////////////////////////////////////////////////////////////////////////////

JNIMETHOD(jstring, serializeToString, Java_Beatmup_NNets_Model, serializeToString)
    (JNIEnv * jenv, jobject, jlong handle)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::NNets::Model, model, handle);
    return jenv->NewStringUTF(model->serializeToString().c_str());
}


JNIMETHOD(jlong, countMultiplyAdds, Java_Beatmup_NNets_Model, countMultiplyAdds)
    (JNIEnv * jenv, jobject, jlong handle)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::NNets::Model, model, handle);
    return (jlong)model->countMultiplyAdds();
}


JNIMETHOD(jlong, getMemorySize, Java_Beatmup_NNets_Model, getMemorySize)
    (JNIEnv * jenv, jobject, jlong handle)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::NNets::Model, model, handle);
    return (jlong)model->getMemorySize();
}


JNIMETHOD(jlong, getNumberOfOperations, Java_Beatmup_NNets_Model, getNumberOfOperations)
    (JNIEnv * jenv, jobject, jlong handle)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::NNets::Model, model, handle);
    return (jlong)model->getNumberOfOperations();
}

/////////////////////////////////////////////////////////////////////////////////////////////
//                                     DESERIALIZED MODEL
/////////////////////////////////////////////////////////////////////////////////////////////

JNIMETHOD(jlong, newDeserializedModel, Java_Beatmup_NNets_DeserializedModel, newDeserializedModel)
    (JNIEnv * jenv, jclass, jobject jCtx, jstring str)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Android::Context, ctx, jCtx);
    BEATMUP_STRING(str);
    BEATMUP_CATCH({
        auto model = new Beatmup::NNets::DeserializedModel(*ctx, strStr);
        $pool.addJavaReference(jenv, jCtx, model);      // model needs context in destruction
        return (jlong)model;
    });
    return BeatmupJavaObjectPool::INVALID_HANDLE;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//                                     ABSTRACT OPERATION
/////////////////////////////////////////////////////////////////////////////////////////////

JNIMETHOD(jstring, getName, Java_Beatmup_NNets_AbstractOperation, getName)
    (JNIEnv * jenv, jobject, jlong handle)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::NNets::AbstractOperation, op, handle);
    return jenv->NewStringUTF(op->getName().c_str());
}


JNIMETHOD(jlong, countMultiplyAdds, Java_Beatmup_NNets_AbstractOperation, countMultiplyAdds)
    (JNIEnv * jenv, jobject, jlong handle)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::NNets::AbstractOperation, op, handle);
    return (jlong)op->countMultiplyAdds();
}


JNIMETHOD(jlong, getOperationFromModel, Java_Beatmup_NNets_AbstractOperation, getOperationFromModel)
    (JNIEnv * jenv, jclass, jlong handle, jstring name)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::NNets::Model, model, handle);
    BEATMUP_STRING(name);
    try {
        return (jlong)&model->getOperation(nameStr);
    }
    catch (Beatmup::InvalidArgument& ex) {
        $pool.throwToJava(jenv, "java/lang/IllegalArgumentException", ex.what());
    }
    return BeatmupJavaObjectPool::INVALID_HANDLE;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//                                          SOFTMAX
/////////////////////////////////////////////////////////////////////////////////////////////

JNIMETHOD(jfloatArray, getProbabilities, Java_Beatmup_NNets_Softmax, getProbabilities)
    (JNIEnv * jenv, jclass, jlong handle)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::NNets::Softmax, softmax, handle);
    const auto& proba = softmax->getProbabilities();
    jfloatArray result = jenv->NewFloatArray(proba.size());
    jfloat *outptr = jenv->GetFloatArrayElements(result, nullptr);
    for (size_t i = 0; i < proba.size(); ++i)
        outptr[i] = proba[i];
    jenv->ReleaseFloatArrayElements(result, outptr, 0);
    return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//                                       IMAGE SAMPLER
/////////////////////////////////////////////////////////////////////////////////////////////

JNIMETHOD(void, setRotation, Java_Beatmup_NNets_ImageSampler, setRotation)
    (JNIEnv * jenv, jclass, jlong handle, jint rotation)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::NNets::ImageSampler, imageSampler, handle);
    imageSampler->setRotation((int)rotation);
}


JNIMETHOD(jint, getRotation, Java_Beatmup_NNets_ImageSampler, getRotation)
    (JNIEnv * jenv, jclass, jlong handle)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::NNets::ImageSampler, imageSampler, handle);
    return (jint)imageSampler->getRotation();
}

/////////////////////////////////////////////////////////////////////////////////////////////
//                                       INFERENCE TASK
/////////////////////////////////////////////////////////////////////////////////////////////

JNIMETHOD(jlong, newInferenceTask, Java_Beatmup_NNets_InferenceTask, newInferenceTask)
    (JNIEnv * jenv, jclass, jobject jModel, jobject jData)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::NNets::Model, model, jModel);
    BEATMUP_OBJ(Beatmup::ChunkCollection, data, jData);
    Beatmup::NNets::InferenceTask* task = new Beatmup::NNets::InferenceTask(*model, *data);
    $pool.addJavaReference(jenv, jModel, task);     // inference task needs model
    $pool.addJavaReference(jenv, jData, task);      // inference task needs data
    return (jlong)task;
}


JNIMETHOD(void, connectByName, Java_Beatmup_NNets_InferenceTask, connectByName)
    (JNIEnv * jenv, jobject, jlong handle, jobject jBitmap, jstring opName, jint index)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::NNets::InferenceTask, task, handle);
    BEATMUP_OBJ(Beatmup::AbstractBitmap, bitmap, jBitmap);
    BEATMUP_STRING(opName);
    try {
        task->connect(*bitmap, opNameStr, (int)index);
    }
    catch (Beatmup::InvalidArgument& ex) {
        $pool.throwToJava(jenv, "java/lang/IllegalArgumentException", ex.what());
    }
}


JNIMETHOD(void, connectByHandle, Java_Beatmup_NNets_InferenceTask, connectByHandle)
    (JNIEnv * jenv, jobject, jlong handle, jobject jBitmap, jlong hOp, jint index)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::NNets::InferenceTask, task, handle);
    BEATMUP_OBJ(Beatmup::AbstractBitmap, bitmap, jBitmap);
    BEATMUP_OBJ(Beatmup::NNets::AbstractOperation, op, hOp);
    task->connect(*bitmap, *op, (int)index);
}
