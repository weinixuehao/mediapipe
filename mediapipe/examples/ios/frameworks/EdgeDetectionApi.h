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

- (void)initWithLiveView:(UIView *) liveView;

@end

NS_ASSUME_NONNULL_END
