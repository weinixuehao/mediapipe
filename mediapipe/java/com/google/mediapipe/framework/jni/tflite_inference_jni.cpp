//
// Created by chenlong on 7/16/20.
//

#include "tflite_inference_jni.h"
#include "mediapipe/inference/tflite_inference.h"

TfliteInference *pInference;
JNIEXPORT void JNICALL TFLITE_INFERENCE_METHOD(init)(JNIEnv* env, jclass clz, jstring _model_path) {
    const char* utf = env->GetStringUTFChars(_model_path, 0);
    std::string model_path(utf);
    pInference = new TfliteInference(model_path);
    env->ReleaseStringUTFChars(_model_path, utf);
}

JNIEXPORT jboolean JNICALL TFLITE_INFERENCE_METHOD(unit)(JNIEnv* env, jclass clz){
    delete pInference;
    pInference = nullptr;
}

JNIEXPORT jboolean JNICALL TFLITE_INFERENCE_METHOD(run)(JNIEnv* env, jclass clz, jstring img_path) {

}
