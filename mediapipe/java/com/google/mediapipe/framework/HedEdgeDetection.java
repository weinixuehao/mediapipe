package com.google.mediapipe.framework;

import android.graphics.Bitmap;
import android.graphics.PointF;

import java.util.List;

/**
 * created by chenlong on 2020.6.16
 */
public class HedEdgeDetection {
    public static native void init(String modelPath);
    public static native List<PointF> run(Bitmap bitmap);
    public static native void uninit();
}
