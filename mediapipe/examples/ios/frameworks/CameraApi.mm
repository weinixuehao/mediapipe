//
//  edge_detection_api.m
//  _idx_HedEdgeDetectionAppLibrary_089A35A0_ios_min10.0
//
//  Created by chenlong on 6/30/20.
//

#import "CameraApi.h"
#import "mediapipe/objc/MPPGraph.h"
#import "mediapipe/objc/MPPCameraInputSource.h"
#import "mediapipe/objc/MPPLayerRenderer.h"

static NSString* const kGraphName = @"mobile_gpu";

static const char* kInputStream = "input_video";
static const char* kOutputStream = "output_video";
static const char* CAPTURE_IMG = "capture_img";
static const char* kVideoQueueLabel = "com.google.mediapipe.example.videoQueue";

@interface CameraApi () <MPPGraphDelegate, MPPInputSourceDelegate, AVCapturePhotoCaptureDelegate>

// The MediaPipe graph currently in use. Initialized in viewDidLoad, started in viewWillAppear: and
// sent video frames on _videoQueue.
@property(nonatomic) MPPGraph* mediapipeGraph;
@property(atomic, assign) BOOL takingPic;
@property(atomic, strong) NSLock *lock;

@end

@implementation CameraApi {
    /// Handles camera access via AVCaptureSession library.
    MPPCameraInputSource* _cameraSource;
    CVPixelBufferRef currentPixelBufRef;
    /// Inform the user when camera is unavailable.
//    IBOutlet UILabel* _noCameraLabel;
    /// Display the camera preview frames.
//    IBOutlet UIView* _liveView;
    /// Render frames in a layer.
    MPPLayerRenderer* _renderer;

    /// Process camera frames on this queue.
    dispatch_queue_t _videoQueue;
}

- (instancetype)initWithLiveView:(UIView *) liveView {
    self = [super init];
    if (self) {
        self.lock = [[NSLock alloc] init];
        _renderer = [[MPPLayerRenderer alloc] init];
        _renderer.layer.frame = liveView.layer.bounds;
        [liveView.layer addSublayer:_renderer.layer];
        _renderer.frameScaleMode = MPPFrameScaleModeFillAndCrop;
        
        dispatch_queue_attr_t qosAttribute = dispatch_queue_attr_make_with_qos_class(
                                                                                     DISPATCH_QUEUE_SERIAL, QOS_CLASS_USER_INTERACTIVE, /*relative_priority=*/0);
        _videoQueue = dispatch_queue_create(kVideoQueueLabel, qosAttribute);
        
        _cameraSource = [[MPPCameraInputSource alloc] init];
        [_cameraSource setDelegate:self queue:_videoQueue];
        _cameraSource.sessionPreset = AVCaptureSessionPresetPhoto;
        _cameraSource.cameraPosition = AVCaptureDevicePositionBack;
        // The frame's native format is rotated with respect to the portrait orientation.
        _cameraSource.orientation = AVCaptureVideoOrientationPortrait;
        
        self.mediapipeGraph = [[self class] loadGraphFromResource:kGraphName];
        self.mediapipeGraph.delegate = self;
        // Set maxFramesInFlight to a small value to avoid memory contention for real-time processing.
        self.mediapipeGraph.maxFramesInFlight = 2;
    }
    return self;
}

#pragma mark - MediaPipe graph methods

+ (MPPGraph*)loadGraphFromResource:(NSString*)resource {
  // Load the graph config resource.
  NSError* configLoadError = nil;
  NSBundle* bundle = [NSBundle bundleForClass:[self class]];
  if (!resource || resource.length == 0) {
    return nil;
  }
  NSURL* graphURL = [bundle URLForResource:resource withExtension:@"binarypb"];
  NSData* data = [NSData dataWithContentsOfURL:graphURL options:0 error:&configLoadError];
  if (!data) {
    NSLog(@"Failed to load MediaPipe graph config: %@", configLoadError);
    return nil;
  }

  // Parse the graph config resource into mediapipe::CalculatorGraphConfig proto object.
  mediapipe::CalculatorGraphConfig config;
  config.ParseFromArray(data.bytes, data.length);

  // Create MediaPipe graph with mediapipe::CalculatorGraphConfig proto object.
  MPPGraph* newGraph = [[MPPGraph alloc] initWithGraphConfig:config];
  [newGraph addFrameOutputStream:kOutputStream outputPacketType:MPPPacketTypePixelBuffer];
  [newGraph addFrameOutputStream:CAPTURE_IMG outputPacketType:MPPPacketTypeRaw];
  return newGraph;
}

- (void)startDetection {
    [_cameraSource requestCameraAccessWithCompletionHandler:^void(BOOL granted) {
      if (granted) {
        [self startGraphAndCamera];
      }
    }];
}

- (void)startGraphAndCamera {
  // Start running self.mediapipeGraph.
  NSError* error;
  if (![self.mediapipeGraph startWithError:&error]) {
    NSLog(@"Failed to start graph: %@", error);
  }

  // Start fetching frames from the camera.
  dispatch_async(_videoQueue, ^{
    [_cameraSource start];
  });
}

#pragma mark - MPPGraphDelegate methods

// Receives CVPixelBufferRef from the MediaPipe graph. Invoked on a MediaPipe worker thread.
- (void)mediapipeGraph:(MPPGraph*)graph
    didOutputPixelBuffer:(CVPixelBufferRef)pixelBuffer
              fromStream:(const std::string&)streamName {
  if (streamName == kOutputStream) {
    // Display the captured image on the screen.
      
      //这里需要加锁，确保在主线程处于比较忙碌状态，不触发CVPixelBufferRetain。因为主线程没有足够时间去消耗这些资源，从而造成内存溢出。
      [self.lock lock];
      if (currentPixelBufRef) {
          [self.lock unlock];
          return;
      }
      currentPixelBufRef = CVPixelBufferRetain(pixelBuffer);
      [self.lock unlock];
    dispatch_async(dispatch_get_main_queue(), ^{
        [self.lock lock];
        [_renderer renderPixelBuffer:currentPixelBufRef];
        CVPixelBufferRelease(currentPixelBufRef);
        currentPixelBufRef = nil;
        [self.lock unlock];
    });
  }
}

- (void)mediapipeGraph:(MPPGraph *)graph didOutputPacket:(const mediapipe::Packet &)packet fromStream:(const std::string &)streamName {
    if (streamName == CAPTURE_IMG && self.detectStatusBlock) {
        if (packet.IsEmpty()) {
            return;
        }
        auto status = packet.Get<int>();
        dispatch_async(dispatch_get_main_queue(), ^{
            if (!self.takingPic) {
                self.detectStatusBlock(status);
            }
        });
    }
}

- (void)takePic {
    if (!self.takingPic) {
        self.takingPic = YES;
        [_cameraSource takePic:self];
    }
}

- (void)captureOutput:(AVCapturePhotoOutput *)output didFinishProcessingPhotoSampleBuffer:(CMSampleBufferRef)photoSampleBuffer previewPhotoSampleBuffer:(CMSampleBufferRef)previewPhotoSampleBuffer resolvedSettings:(AVCaptureResolvedPhotoSettings *)resolvedSettings bracketSettings:(AVCaptureBracketedStillImageSettings *)bracketSettings error:(NSError *)error {
    if (photoSampleBuffer) {
        NSData *data = [AVCapturePhotoOutput JPEGPhotoDataRepresentationForJPEGSampleBuffer:photoSampleBuffer previewPhotoSampleBuffer:previewPhotoSampleBuffer];
        UIImage *image = [UIImage imageWithData:data];
        if (self.takePicBlock) {
            self.takePicBlock(image);
        }
    }
    self.takingPic = NO;
}

- (void)captureOutput:(AVCapturePhotoOutput *)output didFinishProcessingPhoto:(AVCapturePhoto *)photo error:(NSError *)error  API_AVAILABLE(ios(11.0)){
    NSData *imageData = [photo fileDataRepresentation];
    UIImage *image = [UIImage imageWithData:imageData];
    if (self.takePicBlock) {
        self.takePicBlock(image);
    }
    self.takingPic = NO;
}

#pragma mark - MPPInputSourceDelegate methods

// Must be invoked on _videoQueue.
- (void)processVideoFrame:(CVPixelBufferRef)imageBuffer
                timestamp:(CMTime)timestamp
               fromSource:(MPPInputSource*)source {
    //当拍照的时候会导致，Using more buffers than expected! This is a debug-only warning, "
    //"you can ignore it if your app works fine otherwise. 错误。这个错误会导致app崩溃.
    if (self.takingPic) {
        return;
    }
    if (source != _cameraSource) {
#ifdef DEBUG
    NSLog(@"Unknown source: %@", source);
#endif
      return;
    }
    [self.mediapipeGraph sendPixelBuffer:imageBuffer
                            intoStream:kInputStream
                            packetType:MPPPacketTypePixelBuffer];
}

#pragma mark - Cleanup methods
- (void)dealloc
{
    self.mediapipeGraph.delegate = nil;
    [self.mediapipeGraph cancel];
    // Ignore errors since we're cleaning up.
    [self.mediapipeGraph closeAllInputStreamsWithError:nil];
    if (_videoQueue) return;
    dispatch_async(_videoQueue, ^{
        [self.mediapipeGraph waitUntilDoneWithError:nil];
        if (currentPixelBufRef) {
#ifdef DEBUG
            NSLog(@"Clear the currentPixelBufRef as it not be used to display on the screen!");
#endif
            CVPixelBufferRelease(currentPixelBufRef);
        }
    });
}

@end
