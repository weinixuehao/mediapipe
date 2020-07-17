package com.google.mediapipe.framework;

/**
 * created by chenlong on 2020.6.16
 */
public class TfliteInference {
    public static native void init(String modelPath);
    public static native void run(String imgPath);
    public static native void uninit();
}
