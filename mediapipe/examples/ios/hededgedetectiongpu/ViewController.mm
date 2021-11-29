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

#import "ViewController.h"
#import "mediapipe/examples/ios/frameworks/CameraApi.h"
#import "mediapipe/examples/ios/frameworks/UIImage+PictureOrientation.h"
#import "mediapipe/examples/ios/frameworks/HedEdgeDetectApi.h"

//#import "mediapipe/objc/MPPGraph.h"
//#import "mediapipe/objc/MPPCameraInputSource.h"
//#import "mediapipe/objc/MPPLayerRenderer.h"
//
//static NSString* const kGraphName = @"mobile_gpu";
//
//static const char* kInputStream = "input_video";
//static const char* kOutputStream = "output_video";
//static const char* kVideoQueueLabel = "com.google.mediapipe.example.videoQueue";

//@interface ViewController () <MPPGraphDelegate, MPPInputSourceDelegate>

// The MediaPipe graph currently in use. Initialized in viewDidLoad, started in viewWillAppear: and
// sent video frames on _videoQueue.
//@property(nonatomic) MPPGraph* mediapipeGraph;

//@end

@implementation ViewController {
  /// Handles camera access via AVCaptureSession library.
//  MPPCameraInputSource* _cameraSource;
//
//  /// Inform the user when camera is unavailable.
  IBOutlet UILabel* _noCameraLabel;
//  /// Display the camera preview frames.
  IBOutlet UIView* _liveView;
    
  __unsafe_unretained IBOutlet UIButton *takingPicBtn;
    //  /// Render frames in a layer.
//  MPPLayerRenderer* _renderer;
//
//  /// Process camera frames on this queue.
//  dispatch_queue_t _videoQueue;
    CameraApi *_cameraApi;
    __unsafe_unretained IBOutlet UIButton *backBtn;
    HedEdgeDetectApi *_hedEdgeDetectApi;
    __unsafe_unretained IBOutlet UIButton *modeBtn;
}

- (IBAction)takingPicEvent:(id)sender {
#ifdef DEBUG
    NSLog(@"taking picture event!");
#endif
    [_cameraApi takePic];
}

- (IBAction)backAction:(id)sender {
    [self dismissViewControllerAnimated:false completion:nil];
}

- (IBAction)autoTakePicEvent:(id)sender {
    #ifdef DEBUG
        NSLog(@"Auto take picture event!");
    #endif
//    _cameraApi.autoTakePic = true;
}

- (void)viewDidLoad {
    _hedEdgeDetectApi = [[HedEdgeDetectApi alloc] initWithModelPath: @"mediapipe/models/hed_graph.tflite"];
    _cameraApi = [[CameraApi alloc] initWithLiveView:_liveView];
    _cameraApi.detectStatusBlock = ^(int status) {
        NSLog(@"status=%d", status);
    };
    _cameraApi.takePicBlock = ^(UIImage * _Nonnull pic) {
        UIImage *image = [pic normalizedImage];
        std::vector<cv::Point> points;
        [_hedEdgeDetectApi run: image points:points];
//        NSLog(@"Taked an pic!");
    };
}

- (void)viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];
    [_cameraApi startDetection];
}

- (void)viewWillDisappear:(BOOL)animated {
    NSLog(@"viewWillDisappear");
    _cameraApi = nil; //Warning!!! This will trigger the dealloc method of the CameraApi to release resource. It is import !!
}

@end
