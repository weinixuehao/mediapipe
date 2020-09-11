//
// Created by chenlong on 7/16/20.
//

#include "hed_edge_detection_jni.h"
#include "mediapipe/inference/hed_edge_detection.h"
#include <android/bitmap.h>
#include <opencv2/imgproc/imgproc.hpp>

void BitmapToMat2(JNIEnv *env, jobject &bitmap, cv::Mat &mat, jboolean needUnPremultiplyAlpha) {
    AndroidBitmapInfo info;
    void *pixels = 0;
    cv::Mat &dst = mat;

    try {
        CV_Assert(AndroidBitmap_getInfo(env, bitmap, &info) >= 0);
        CV_Assert(info.format == ANDROID_BITMAP_FORMAT_RGBA_8888 ||
                  info.format == ANDROID_BITMAP_FORMAT_RGB_565);
        CV_Assert(AndroidBitmap_lockPixels(env, bitmap, &pixels) >= 0);
        CV_Assert(pixels);
        dst.create(info.height, info.width, CV_8UC4);
        if (info.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
            cv::Mat tmp(info.height, info.width, CV_8UC4, pixels);
            if (needUnPremultiplyAlpha) cvtColor(tmp, dst, cv::COLOR_mRGBA2RGBA);
            else tmp.copyTo(dst);
        } else {
            cv::Mat tmp(info.height, info.width, CV_8UC2, pixels);
            cvtColor(tmp, dst, cv::COLOR_BGR5652RGBA);
        }
        AndroidBitmap_unlockPixels(env, bitmap);
        return;
    } catch (const cv::Exception &e) {
        AndroidBitmap_unlockPixels(env, bitmap);
        jclass je = env->FindClass("org/opencv/core/CvException");
        if (!je) je = env->FindClass("java/lang/Exception");
        env->ThrowNew(je, e.what());
        return;
    } catch (...) {
        AndroidBitmap_unlockPixels(env, bitmap);
        jclass je = env->FindClass("java/lang/Exception");
        env->ThrowNew(je, "Unknown exception in JNI code {nBitmapToMat}");
        return;
    }
}

JNIEXPORT jlong JNICALL TFLITE_INFERENCE_METHOD(init)(JNIEnv* env, jclass clz, jstring _model_path) {
    const char* utf = env->GetStringUTFChars(_model_path, 0);
    std::string model_path(utf);
    HedEdgeDetecion *pInference = new HedEdgeDetecion(model_path);
    env->ReleaseStringUTFChars(_model_path, utf);
    return (long)pInference;
}

JNIEXPORT void JNICALL TFLITE_INFERENCE_METHOD(uninit)(JNIEnv* env, jclass clz, jlong inference_address){
    HedEdgeDetecion *pInference = (HedEdgeDetecion *) inference_address;
    delete pInference;
    pInference = nullptr;
}

JNIEXPORT jobject JNICALL TFLITE_INFERENCE_METHOD(run)(JNIEnv* env, jclass clz, jobject bitmap, jlong inference_address) {
    HedEdgeDetecion *pInference = (HedEdgeDetecion *) inference_address;
    assert(pInference != nullptr);
    cv::Mat origImg;
    BitmapToMat2(env, bitmap, origImg, 0);
    std::vector<cv::Point> points;
    pInference->run(origImg, points);
    jclass cls_ArrayList = env->FindClass("java/util/ArrayList");
    jmethodID construct = env->GetMethodID(cls_ArrayList,"<init>","()V");
    jobject obj_ArrayList = env->NewObject(cls_ArrayList,construct,"");
    jmethodID arrayList_add = env->GetMethodID(cls_ArrayList,"add","(Ljava/lang/Object;)Z");
    jclass clazzOfPointF = env->FindClass("android/graphics/PointF");
    jfieldID fid_x = env->GetFieldID(clazzOfPointF, "x", "F");
    jfieldID fid_y = env->GetFieldID(clazzOfPointF, "y", "F");
    for (int i = 0, len = points.size(); i < len; i++) {
        jobject pointFRef = (*env).NewObject(clazzOfPointF,
                                             (*env).GetMethodID(clazzOfPointF, "<init>",
                                                                "()V"));
        env->SetFloatField(pointFRef, fid_x, points[i].x);
        env->SetFloatField(pointFRef, fid_y, points[i].y);
        env->CallBooleanMethod(obj_ArrayList, arrayList_add, pointFRef);
    }

    return obj_ArrayList;
}


