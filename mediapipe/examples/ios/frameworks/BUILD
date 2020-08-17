MIN_IOS_VERSION = "10.0"

load("@build_bazel_rules_apple//apple:ios.bzl", "ios_framework")

objc_library(
    name = "HedEdgeDetectionLibrary",
    visibility = ["//visibility:public"],
    srcs = ["EdgeDetectionApi.mm", "HedEdgeDetectApi.mm", "MMOpenCVHelper.mm"],
    hdrs = ["EdgeDetectionApi.h", "HedEdgeDetectApi.h", "MMOpenCVHelper.h"],
    data = [
        "//mediapipe/graphs/hed_edge_detection:mobile_gpu_binary_graph",
        "//mediapipe/models:hed_graph.tflite",
    ],
    sdk_frameworks = [
        "AVFoundation",
        "CoreGraphics",
        "CoreMedia",
        "UIKit",
    ],
    deps = [
        "//mediapipe/objc:mediapipe_framework_ios",
        "//mediapipe/objc:mediapipe_input_sources_ios",
        "//mediapipe/objc:mediapipe_layer_renderer",
    ] + select({
        "//mediapipe:ios_i386": [],
        "//mediapipe:ios_x86_64": [],
        "//conditions:default": [
            "//mediapipe/graphs/hed_edge_detection:mobile_calculators",
        ],
    }),
)

# bazel build --config=ios_fat --copt=-fembed-bitcode mediapipe/examples/ios/frameworks:HedEdgeDetection_Framework --linkopt="-s"
ios_framework(
    name = "HedEdgeDetection_Framework",
    # If `hdrs` isn't set Headers and Modules folders won't get generated
    hdrs = ["EdgeDetectionApi.h", "HedEdgeDetectApi.h"],
    families = [
        "iphone",
        "ipad",
    ],
    infoplists = ["info.plist"],
    bundle_id = "com.ctsmed.edgedetection",
    minimum_os_version = MIN_IOS_VERSION,
    deps = [
        "@ios_opencv//:OpencvFramework",
        "HedEdgeDetectionLibrary"
    ]
)
