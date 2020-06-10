cc_library(
    name = "hed_edge_detection_calculator",
    srcs = [
        "hed_edge_detection_calculator.cc",
        "fm_ocr_scanner.cpp"
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//mediapipe/framework:calculator_framework",
        "@org_tensorflow//tensorflow/lite:framework",
        "@org_tensorflow//tensorflow/lite/delegates/gpu:gl_delegate",
        "//mediapipe/gpu:gl_calculator_helper"
    ],
    alwayslink = 1
)