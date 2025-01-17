// Copyright 2019 The MediaPipe Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

package com.google.mediapipe.apps.hededgedetectiongpu;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.PointF;
import android.graphics.SurfaceTexture;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.util.Size;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;

import com.google.mediapipe.components.CameraHelper;
import com.google.mediapipe.components.CameraXMultipleUseCaseHelper;
import com.google.mediapipe.components.ExternalTextureConverter;
import com.google.mediapipe.components.FrameProcessor;
import com.google.mediapipe.components.PermissionHelper;
import com.google.mediapipe.framework.AndroidAssetUtil;
import com.google.mediapipe.framework.Packet;
import com.google.mediapipe.framework.PacketCallback;
import com.google.mediapipe.framework.HedEdgeDetection;
import com.google.mediapipe.framework.PacketGetter;
import com.google.mediapipe.glutil.EglManager;

import java.io.File;
import java.util.List;

import androidx.appcompat.app.AppCompatActivity;
import androidx.camera.core.ImageCapture;

/** Bare-bones main activity. */
public class MainActivity extends AppCompatActivity {
  private static final String TAG = MainActivity.class.getSimpleName();
  private static final String BINARY_GRAPH_NAME = "hed_edge_detection.binarypb";
  private static final String INPUT_VIDEO_STREAM_NAME = "input_video";
  private static final String OUTPUT_VIDEO_STREAM_NAME = "output_video";
  private static final CameraHelper.CameraFacing CAMERA_FACING = CameraHelper.CameraFacing.BACK;

  // Flips the camera-preview frames vertically before sending them into FrameProcessor to be
  // processed in a MediaPipe graph, and flips the processed frames back when they are displayed.
  // This is needed because OpenGL represents images assuming the image origin is at the bottom-left
  // corner, whereas MediaPipe in general assumes the image origin is at top-left.
  private static final boolean FLIP_FRAMES_VERTICALLY = true;

  static {
    // Load all native libraries needed by the app.
    System.loadLibrary("mediapipe_jni");
    System.loadLibrary("opencv_java3");
  }

  // {@link SurfaceTexture} where the camera-preview frames can be accessed.
  private SurfaceTexture previewFrameTexture;
  // Sends camera-preview frames into a MediaPipe graph for processing, and displays the processed
  // frames onto a {@link Surface}.
  private FrameProcessor processor;
  // {@link SurfaceView} that displays the camera-preview frames processed by a MediaPipe graph.
  private SurfaceView previewDisplayView;

  // Creates and manages an {@link EGLContext}.
  private EglManager eglManager;
  // Converts the GL_TEXTURE_EXTERNAL_OES texture from Android camera into a regular texture to be
  // consumed by {@link FrameProcessor} and the underlying MediaPipe graph.
  private ExternalTextureConverter converter;

  // Handles camera access via the {@link CameraX} Jetpack support library.
  private CameraXMultipleUseCaseHelper cameraHelper;

  private long mHedRef;

  @Override
  protected void onDestroy() {
    super.onDestroy();
    HedEdgeDetection.uninit(mHedRef);
  }

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_main);

    previewDisplayView = new SurfaceView(this);
    setupPreviewDisplayView();

    // Initialize asset manager so that MediaPipe native libraries can access the app assets, e.g.,
    // binary graphs.
    AndroidAssetUtil.initializeNativeAssetManager(this);

    this.mHedRef = HedEdgeDetection.init("mediapipe/models/hed_graph.tflite");
    eglManager = new EglManager(null);
    processor =
        new FrameProcessor(
            this,
            eglManager.getNativeContext(),
            BINARY_GRAPH_NAME,
            INPUT_VIDEO_STREAM_NAME,
            OUTPUT_VIDEO_STREAM_NAME);
    processor.getVideoSurfaceOutput().setFlipY(FLIP_FRAMES_VERTICALLY);
    processor.addPacketCallback("capture_img", new PacketCallback() {
      @Override
      public void process(Packet packet) {
        int contourStatus = PacketGetter.getInt32(packet);
        if (contourStatus == 2) {
          takePic();
        }
        Log.w(TAG, "contourStatus=>"+contourStatus);
      }
    });

    PermissionHelper.checkAndRequestCameraPermissions(this);

    Button takePicBtn = findViewById(R.id.take_pic);
    takePicBtn.setOnClickListener(new View.OnClickListener() {
      @Override
      public void onClick(View v) {
        takePic();
      }
    });
  }

  private void takePic() {
    if (cameraHelper != null) {
      File saveImgFile = new File(MainActivity.this.getExternalFilesDir(Environment.DIRECTORY_PICTURES), System.currentTimeMillis() + ".jpg");
      cameraHelper.takePicture(MainActivity.this, saveImgFile, new ImageCapture.OnImageSavedListener() {
        @Override
        public void onImageSaved(File file) {
          Bitmap bitmap = BitmapFactory.decodeFile(file.getPath());
          List<PointF> points = HedEdgeDetection.run(bitmap, mHedRef);
          bitmap.recycle();
          Log.i(TAG, "points=>"+points.size() + " thread=>"+Thread.currentThread().getName());

        }

        @Override
        public void onError(ImageCapture.ImageCaptureError imageCaptureError, String s, Throwable throwable) {
          Log.i(TAG, "take picture failed");
        }
      });
    }
  }

  @Override
  protected void onResume() {
    super.onResume();
    converter = new ExternalTextureConverter(eglManager.getContext());
    converter.setFlipY(FLIP_FRAMES_VERTICALLY);
    converter.setConsumer(processor);
    if (PermissionHelper.cameraPermissionsGranted(this)) {
      startCamera();
    }
  }

  @Override
  protected void onPause() {
    super.onPause();
    previewDisplayView.setVisibility(View.GONE);
    converter.close();
  }

  @Override
  public void onRequestPermissionsResult(
      int requestCode, String[] permissions, int[] grantResults) {
    super.onRequestPermissionsResult(requestCode, permissions, grantResults);
    PermissionHelper.onRequestPermissionsResult(requestCode, permissions, grantResults);
  }

  public void startCamera() {
    cameraHelper = new CameraXMultipleUseCaseHelper();
    cameraHelper.setOnCameraStartedListener(
        surfaceTexture -> {
          previewFrameTexture = surfaceTexture;
          // Make the display view visible to start showing the preview. This triggers the
          // SurfaceHolder.Callback added to (the holder of) previewDisplayView.
          previewDisplayView.setVisibility(View.VISIBLE);
        });
    cameraHelper.startCamera(this, CAMERA_FACING, /*surfaceTexture=*/ null);
  }

  private void setupPreviewDisplayView() {
    previewDisplayView.setVisibility(View.GONE);
    ViewGroup viewGroup = findViewById(R.id.preview_display_layout);
    viewGroup.addView(previewDisplayView);

    previewDisplayView
        .getHolder()
        .addCallback(
            new SurfaceHolder.Callback() {
              @Override
              public void surfaceCreated(SurfaceHolder holder) {
                processor.getVideoSurfaceOutput().setSurface(holder.getSurface());
              }

              @Override
              public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
                // (Re-)Compute the ideal size of the camera-preview display (the area that the
                // camera-preview frames get rendered onto, potentially with scaling and rotation)
                // based on the size of the SurfaceView that contains the display.
                Size viewSize = new Size(width, height);
                Size displaySize = cameraHelper.computeDisplaySizeFromViewSize(viewSize);
                boolean isCameraRotated = cameraHelper.isCameraRotated();

                // Connect the converter to the camera-preview frames as its input (via
                // previewFrameTexture), and configure the output width and height as the computed
                // display size.
                converter.setSurfaceTextureAndAttachToGLContext(
                    previewFrameTexture,
                    isCameraRotated ? displaySize.getHeight() : displaySize.getWidth(),
                    isCameraRotated ? displaySize.getWidth() : displaySize.getHeight());
              }

              @Override
              public void surfaceDestroyed(SurfaceHolder holder) {
                processor.getVideoSurfaceOutput().setSurface(null);
              }
            });
  }
}
