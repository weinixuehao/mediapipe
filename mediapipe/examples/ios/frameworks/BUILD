MIN_IOS_VERSION = "10.0"

load("@build_bazel_rules_apple//apple:ios.bzl", "ios_static_framework")

objc_library(
    name="HedEdgeDetectionLibrary",
    data = [
        "//mediapipe/graphs/hed_edge_detection:mobile_gpu_binary_graph",
        "//mediapipe/models:hed_graph.tflite",
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

# bazel build --config=ios_fat mediapipe/examples/ios/frameworks:HedEdgeDetection_Framework
ios_static_framework(
    name = "HedEdgeDetection_Framework",
    # If `hdrs` isn't set Headers and Modules folders won't get generated
    hdrs = [
        "mediapipe/objc/MPPCameraInputSource.h",
        # "//mediapipe/objc:MPPGraph.h",
        # "//mediapipe/objc:MPPLayerRenderer.h",
        # "//mediapipe/objc:MPPInputSource.h",
    ],
    includes=".",
    families = [
        "iphone",
        "ipad",
    ],
    minimum_os_version = MIN_IOS_VERSION,
    deps = [
        "@ios_opencv//:OpencvFramework",
        "HedEdgeDetectionLibrary"
    ]
)
