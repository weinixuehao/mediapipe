package com.google.mediapipe.framework;

import android.graphics.Bitmap;
import android.graphics.PointF;

import java.util.List;

/**
 * created by chenlong on 2020.6.16
 */
public class HedEdgeDetection {
    public static native long init(String modelPath);
    public static native List<PointF> run(Bitmap bitmap, long pInference);
    public static native void uninit(long pInference);
}
