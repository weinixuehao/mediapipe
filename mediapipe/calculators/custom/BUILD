cc_library(
    name = "hed_edge_detection_calculator",
    srcs = [
        "hed_edge_detection_calculator.cc",
        "fm_ocr_scanner.cc"
    ],
    hdrs = [
        "fm_ocr_scanner.h"
    ],
    visibility = ["//visibility:public"],
    deps = [
        "@org_tensorflow//tensorflow/lite:framework",
        "@org_tensorflow//tensorflow/lite/delegates/gpu:gl_delegate",
        "//mediapipe/framework/port:opencv_core",
        "//mediapipe/framework/port:opencv_imgproc",
        "//mediapipe/gpu:gl_calculator_helper",
        "//mediapipe/gpu:gl_simple_calculator",
        "//mediapipe/gpu:gl_simple_shaders",
        "//mediapipe/gpu:shader_util",
    ],
    alwayslink = 1
)