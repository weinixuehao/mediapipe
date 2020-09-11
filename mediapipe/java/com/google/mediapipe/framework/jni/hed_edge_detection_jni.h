//
// Created by chenlong on 7/16/20.
//

#ifndef MEDIAPIPE_HED_EDGE_DETECTION_JNI_H
#define MEDIAPIPE_HED_EDGE_DETECTION_JNI_H

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TFLITE_INFERENCE_METHOD(METHOD_NAME) \
  Java_com_google_mediapipe_framework_HedEdgeDetection_##METHOD_NAME

JNIEXPORT jlong JNICALL TFLITE_INFERENCE_METHOD(init)(JNIEnv* env, jclass clz, jstring _model_path);

JNIEXPORT jobject JNICALL TFLITE_INFERENCE_METHOD(run)(JNIEnv* env, jclass clz, jobject bitmap, jlong inference_address);

JNIEXPORT void JNICALL TFLITE_INFERENCE_METHOD(uninit)(JNIEnv* env, jclass clz, jlong inference_address);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
#endif //MEDIAPIPE_HED_EDGE_DETECTION_JNI_H
