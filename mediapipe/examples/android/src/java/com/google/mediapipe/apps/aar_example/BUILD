load("//mediapipe/java/com/google/mediapipe:mediapipe_aar.bzl", "mediapipe_aar")

mediapipe_aar(
    name = "hed_edge_aar",
    calculators = ["//mediapipe/graphs/hed_edge_detection:mobile_calculators"],
)

genrule(
    name = "binary_graph",
    srcs = ["//mediapipe/graphs/hed_edge_detection:mobile_gpu_binary_graph"],
    outs = ["hed_edge_detection.binarypb"],
    cmd = "cp $< $@",
)