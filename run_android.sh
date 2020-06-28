set -e # Will be exited execution if subsequent cmd caused an error

# bazel build -c dbg --strip=never --spawn_strategy=local --host_crosstool_top=@bazel_tools//tools/cpp:toolchain --fat_apk_cpu=arm64-v8a //mediapipe/examples/android/src/java/com/google/mediapipe/apps/hededgedetectiongpu:hededgedetectiongpu --distdir=local_cache --repository_cache=local_cache --verbose_failures

bazel build -c dbg --strip=never --spawn_strategy=local --host_crosstool_top=@bazel_tools//tools/cpp:toolchain --fat_apk_cpu=arm64-v8a //mediapipe/examples/android/src/java/com/google/mediapipe/apps/hededgedetectiongpu:hededgedetectiongpu --verbose_failures

adb install -r bazel-bin/mediapipe/examples/android/src/java/com/google/mediapipe/apps/hededgedetectiongpu/hededgedetectiongpu.apk

adb shell am start -n com.google.mediapipe.apps.hededgedetectiongpu/com.google.mediapipe.apps.hededgedetectiongpu.MainActivity