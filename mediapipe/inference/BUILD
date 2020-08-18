cc_library(
    name = "hed_edge_detection",
    srcs = [
        "hed_edge_detection.cpp"
    ],
    hdrs = [
        "hed_edge_detection.h"
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
        "@org_tensorflow//tensorflow/lite/kernels:builtin_ops",
        "//third_party:opencv",
        "//third_party:glog",
        "//mediapipe/framework/port:status",
        "//mediapipe/framework/port:statusor",
        "//mediapipe/framework/port:ret_check",
        "//mediapipe/util:resource_util",
        "//mediapipe/calculators/custom:fm_ocr_scanner"
    ] + select({
        "//mediapipe:ios": [
            "//mediapipe/gpu:MPPMetalHelper",
            "//mediapipe/gpu:MPPMetalUtil",
            "//mediapipe/gpu:gpu_buffer",
            "//mediapipe/objc:mediapipe_framework_ios",
            "@org_tensorflow//tensorflow/lite/delegates/gpu/common:shape",
            "@org_tensorflow//tensorflow/lite/delegates/gpu/metal:buffer_convert",
            "@org_tensorflow//tensorflow/lite/delegates/gpu:metal_delegate",
            "@org_tensorflow//tensorflow/lite/delegates/gpu:metal_delegate_internal",
        ],
        "//conditions:default": [
            "@org_tensorflow//tensorflow/lite/delegates/gpu:gl_delegate",
        ],
    }),
    alwayslink = 1,
)
