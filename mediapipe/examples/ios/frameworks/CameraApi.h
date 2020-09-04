//
//  edge_detection_api.h
//  _idx_HedEdgeDetectionAppLibrary_089A35A0_ios_min10.0
//
//  Created by chenlong on 6/30/20.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIView.h>

NS_ASSUME_NONNULL_BEGIN

@interface CameraApi : NSObject

- (void)startDetection;

- (void)takePic;

- (instancetype)initWithLiveView:(UIView *) liveView;

typedef void(^DetectStatusBlock)(int status);
@property (atomic, copy) DetectStatusBlock detectStatusBlock;
typedef void(^TakePicBlock)(UIImage *pic);
@property (atomic, copy) TakePicBlock takePicBlock;
@property (atomic, assign) BOOL autoTakePic;

@end

NS_ASSUME_NONNULL_END
