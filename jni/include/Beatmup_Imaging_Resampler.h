/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class Beatmup_Imaging_Resampler */

#ifndef _Included_Beatmup_Imaging_Resampler
#define _Included_Beatmup_Imaging_Resampler
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     Beatmup_Imaging_Resampler
 * Method:    newResampler
 * Signature: (LBeatmup/Context;)J
 */
JNIEXPORT jlong JNICALL Java_Beatmup_Imaging_Resampler_newResampler
  (JNIEnv *, jclass, jobject);

/*
 * Class:     Beatmup_Imaging_Resampler
 * Method:    setInput
 * Signature: (JLBeatmup/Bitmap;)V
 */
JNIEXPORT void JNICALL Java_Beatmup_Imaging_Resampler_setInput
  (JNIEnv *, jobject, jlong, jobject);

/*
 * Class:     Beatmup_Imaging_Resampler
 * Method:    setOutput
 * Signature: (JLBeatmup/Bitmap;)V
 */
JNIEXPORT void JNICALL Java_Beatmup_Imaging_Resampler_setOutput
  (JNIEnv *, jobject, jlong, jobject);

/*
 * Class:     Beatmup_Imaging_Resampler
 * Method:    setMode
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL Java_Beatmup_Imaging_Resampler_setMode
  (JNIEnv *, jobject, jlong, jint);

/*
 * Class:     Beatmup_Imaging_Resampler
 * Method:    getMode
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_Beatmup_Imaging_Resampler_getMode
  (JNIEnv *, jobject, jlong);

/*
 * Class:     Beatmup_Imaging_Resampler
 * Method:    setCubicParameter
 * Signature: (JF)V
 */
JNIEXPORT void JNICALL Java_Beatmup_Imaging_Resampler_setCubicParameter
  (JNIEnv *, jobject, jlong, jfloat);

/*
 * Class:     Beatmup_Imaging_Resampler
 * Method:    getCubicParameter
 * Signature: (J)F
 */
JNIEXPORT jfloat JNICALL Java_Beatmup_Imaging_Resampler_getCubicParameter
  (JNIEnv *, jobject, jlong);

#ifdef __cplusplus
}
#endif
#endif