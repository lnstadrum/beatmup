/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class Beatmup_Shading_ShaderApplicator */

#ifndef _Included_Beatmup_Shading_ShaderApplicator
#define _Included_Beatmup_Shading_ShaderApplicator
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     Beatmup_Shading_ShaderApplicator
 * Method:    newShaderApplicator
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_Beatmup_Shading_ShaderApplicator_newShaderApplicator
  (JNIEnv *, jclass);

/*
 * Class:     Beatmup_Shading_ShaderApplicator
 * Method:    setInput
 * Signature: (JLBeatmup/Bitmap;)V
 */
JNIEXPORT void JNICALL Java_Beatmup_Shading_ShaderApplicator_setInput
  (JNIEnv *, jobject, jlong, jobject);

/*
 * Class:     Beatmup_Shading_ShaderApplicator
 * Method:    setOutput
 * Signature: (JLBeatmup/Bitmap;)V
 */
JNIEXPORT void JNICALL Java_Beatmup_Shading_ShaderApplicator_setOutput
  (JNIEnv *, jobject, jlong, jobject);

/*
 * Class:     Beatmup_Shading_ShaderApplicator
 * Method:    setLayerShader
 * Signature: (JLBeatmup/Shading/Shader;)V
 */
JNIEXPORT void JNICALL Java_Beatmup_Shading_ShaderApplicator_setLayerShader
  (JNIEnv *, jobject, jlong, jobject);

#ifdef __cplusplus
}
#endif
#endif
