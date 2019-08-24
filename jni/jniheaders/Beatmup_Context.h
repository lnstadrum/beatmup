/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class Beatmup_Context */

#ifndef _Included_Beatmup_Context
#define _Included_Beatmup_Context
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     Beatmup_Context
 * Method:    attachEventListener
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_Beatmup_Context_attachEventListener
  (JNIEnv *, jclass, jlong);

/*
 * Class:     Beatmup_Context
 * Method:    detachEventListener
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_Beatmup_Context_detachEventListener
  (JNIEnv *, jclass, jlong);

/*
 * Class:     Beatmup_Context
 * Method:    getTotalRAM
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_Beatmup_Context_getTotalRAM
  (JNIEnv *, jclass);

/*
 * Class:     Beatmup_Context
 * Method:    performTask
 * Signature: (JILBeatmup/Task;)F
 */
JNIEXPORT jfloat JNICALL Java_Beatmup_Context_performTask
  (JNIEnv *, jobject, jlong, jint, jobject);

/*
 * Class:     Beatmup_Context
 * Method:    submitTask
 * Signature: (JILBeatmup/Task;)I
 */
JNIEXPORT jint JNICALL Java_Beatmup_Context_submitTask
  (JNIEnv *, jobject, jlong, jint, jobject);

/*
 * Class:     Beatmup_Context
 * Method:    submitPersistentTask
 * Signature: (JILBeatmup/Task;)I
 */
JNIEXPORT jint JNICALL Java_Beatmup_Context_submitPersistentTask
  (JNIEnv *, jobject, jlong, jint, jobject);

/*
 * Class:     Beatmup_Context
 * Method:    repeatTask
 * Signature: (JILBeatmup/Task;Z)V
 */
JNIEXPORT void JNICALL Java_Beatmup_Context_repeatTask
  (JNIEnv *, jobject, jlong, jint, jobject, jboolean);

/*
 * Class:     Beatmup_Context
 * Method:    waitForJob
 * Signature: (JII)V
 */
JNIEXPORT void JNICALL Java_Beatmup_Context_waitForJob
  (JNIEnv *, jobject, jlong, jint, jint);

/*
 * Class:     Beatmup_Context
 * Method:    abortJob
 * Signature: (JII)Z
 */
JNIEXPORT jboolean JNICALL Java_Beatmup_Context_abortJob
  (JNIEnv *, jobject, jlong, jint, jint);

/*
 * Class:     Beatmup_Context
 * Method:    waitForAllJobs
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL Java_Beatmup_Context_waitForAllJobs
  (JNIEnv *, jobject, jlong, jint);

/*
 * Class:     Beatmup_Context
 * Method:    busy
 * Signature: (JI)Z
 */
JNIEXPORT jboolean JNICALL Java_Beatmup_Context_busy
  (JNIEnv *, jobject, jlong, jint);

/*
 * Class:     Beatmup_Context
 * Method:    renderChessboard
 * Signature: (JIIII)J
 */
JNIEXPORT jlong JNICALL Java_Beatmup_Context_renderChessboard
  (JNIEnv *, jobject, jlong, jint, jint, jint, jint);

/*
 * Class:     Beatmup_Context
 * Method:    copyBitmap
 * Signature: (LBeatmup/Bitmap;I)J
 */
JNIEXPORT jlong JNICALL Java_Beatmup_Context_copyBitmap
  (JNIEnv *, jobject, jobject, jint);

/*
 * Class:     Beatmup_Context
 * Method:    scanlineSearchInt
 * Signature: (JIIIIII)LBeatmup/Geometry/IntPoint;
 */
JNIEXPORT jobject JNICALL Java_Beatmup_Context_scanlineSearchInt
  (JNIEnv *, jobject, jlong, jint, jint, jint, jint, jint, jint);

/*
 * Class:     Beatmup_Context
 * Method:    scanlineSearchFloat
 * Signature: (JIIFFFF)LBeatmup/Geometry/IntPoint;
 */
JNIEXPORT jobject JNICALL Java_Beatmup_Context_scanlineSearchFloat
  (JNIEnv *, jobject, jlong, jint, jint, jfloat, jfloat, jfloat, jfloat);

/*
 * Class:     Beatmup_Context
 * Method:    maxAllowedWorkerCount
 * Signature: (JI)I
 */
JNIEXPORT jint JNICALL Java_Beatmup_Context_maxAllowedWorkerCount
  (JNIEnv *, jobject, jlong, jint);

/*
 * Class:     Beatmup_Context
 * Method:    limitWorkerCount
 * Signature: (JII)V
 */
JNIEXPORT void JNICALL Java_Beatmup_Context_limitWorkerCount
  (JNIEnv *, jobject, jlong, jint, jint);

/*
 * Class:     Beatmup_Context
 * Method:    swapOnDisk
 * Signature: (JJ)J
 */
JNIEXPORT jlong JNICALL Java_Beatmup_Context_swapOnDisk
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     Beatmup_Context
 * Method:    fetchPixelsFromGPU
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_Beatmup_Context_fetchPixelsFromGPU
  (JNIEnv *, jclass, jlong);

/*
 * Class:     Beatmup_Context
 * Method:    isGPUQueried
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_Beatmup_Context_isGPUQueried
  (JNIEnv *, jobject, jlong);

/*
 * Class:     Beatmup_Context
 * Method:    isGPUReady
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_Beatmup_Context_isGPUReady
  (JNIEnv *, jobject, jlong);

/*
 * Class:     Beatmup_Context
 * Method:    recycleGPUGarbage
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_Beatmup_Context_recycleGPUGarbage
  (JNIEnv *, jobject, jlong);

#ifdef __cplusplus
}
#endif
#endif
