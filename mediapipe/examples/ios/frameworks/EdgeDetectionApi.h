//
//  edge_detection_api.h
//  _idx_HedEdgeDetectionAppLibrary_089A35A0_ios_min10.0
//
//  Created by chenlong on 6/30/20.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIView.h>

NS_ASSUME_NONNULL_BEGIN

@interface EdgeDetectionApi : NSObject

- (void)startDetection;

- (void)takePic;

- (void)initWithLiveView:(UIView *) liveView;

typedef void(^DetectStatusBlock)(int status);
@property (atomic, copy) DetectStatusBlock detectStatusBlock;
@property (atomic, assign) BOOL autoTakePic;

@end

NS_ASSUME_NONNULL_END
