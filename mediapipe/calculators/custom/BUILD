cc_library(
    name = "hed_edge_detection_calculator",
    srcs = [
        "fm_ocr_scanner.cc",
        "hed_edge_detection_calculator.cc",
    ],
    hdrs = [
        "fm_ocr_scanner.h",
    ],
    copts = select({
        "//mediapipe:ios": [
            "-x objective-c++",
            "-fobjc-arc",  # enable reference-counting
        ],
        "//conditions:default": [],
    }),
    visibility = ["//visibility:public"],
    deps = [
        "@org_tensorflow//tensorflow/lite:framework",
        "//mediapipe/framework/port:opencv_core",
        "//mediapipe/framework/port:opencv_imgproc",
        "//mediapipe/gpu:gl_calculator_helper",
        "//mediapipe/gpu:gl_simple_calculator",
        "//mediapipe/gpu:gl_simple_shaders",
        "//mediapipe/gpu:shader_util",
    ] + select({
        "//mediapipe:ios": [],
        "//conditions:default": [
            "@org_tensorflow//tensorflow/lite/delegates/gpu:gl_delegate",
        ],
    }),
    alwayslink = 1,
)
