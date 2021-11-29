bazel build -c dbg --strip=never --spawn_strategy=local --host_crosstool_top=@bazel_tools//tools/cpp:toolchain --fat_apk_cpu=arm64-v8a,armeabi-v7a //mediapipe/examples/android/src/java/com/google/mediapipe/apps/aar_example:hed_edge_aar --verbose_failures

bazel build -c opt --host_crosstool_top=@bazel_tools//tools/cpp:toolchain --fat_apk_cpu=arm64-v8a,armeabi-v7a //mediapipe/examples/android/src/java/com/google/mediapipe/apps/aar_example:hed_edge_aar --verbose_failures --linkopt="-s"


bazel build -c opt mediapipe/examples/android/src/java/com/google/mediapipe/apps/aar_example:binary_graph --verbose_failures

# If failed to download the dependences of this project try to with the two cmds --distdir=cache --repository_cache=cache 