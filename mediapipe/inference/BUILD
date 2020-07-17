cc_library(
    name = "tflite_inference_lib",
    srcs = [
        "tflite_inference.cpp"
    ],
    hdrs = [
        "tflite_inference.h"
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
        "//third_party:glog",
        "//mediapipe/framework/port:status",
        "//mediapipe/framework/port:statusor",
        "//mediapipe/framework/port:ret_check",
        "//mediapipe/util:resource_util",
    ] + select({
        "//mediapipe:ios": [],
        "//conditions:default": [
            "@org_tensorflow//tensorflow/lite/delegates/gpu:gl_delegate",
        ],
    }),
    alwayslink = 1,
)