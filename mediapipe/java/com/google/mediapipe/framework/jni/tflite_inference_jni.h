//
// Created by chenlong on 7/16/20.
//

#ifndef MEDIAPIPE_TFLITE_INFERENCE_JNI_H
#define MEDIAPIPE_TFLITE_INFERENCE_JNI_H

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TFLITE_INFERENCE_METHOD(METHOD_NAME) \
  Java_com_google_mediapipe_framework_TfliteInference_##METHOD_NAME

JNIEXPORT void JNICALL TFLITE_INFERENCE_METHOD(init)(JNIEnv* env, jclass clz, jstring _model_path);

JNIEXPORT jboolean JNICALL TFLITE_INFERENCE_METHOD(run)(JNIEnv* env, jclass clz, jstring img_path);

JNIEXPORT void JNICALL TFLITE_INFERENCE_METHOD(uninit)(JNIEnv* env, jclass clz);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
#endif //MEDIAPIPE_TFLITE_INFERENCE_JNI_H
