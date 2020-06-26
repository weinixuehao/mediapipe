# image lookup -vn <SomeFunctionInTheLibrary>
# settings set target.source-map /Users/user/Ctsmed/mediapipe/external/org_tensorflow/tensorflow/lite/delegates/gpu/metal/compute_task.mm /private/var/tmp/_bazel_chenlong/46cfe12d82525b8b2b43cf38d2da2eb7/external/org_tensorflow/tensorflow/lite/delegates/gpu/metal/compute_task.mm

bazel build -c opt --config=ios_arm64 //mediapipe/examples/ios/hededgedetectiongpu:HedEdgeDetectionApp --verbose_failures
