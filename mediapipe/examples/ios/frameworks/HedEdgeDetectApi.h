//
//  HedEdgeDetect.h
//  _idx_HedEdgeDetectionAppLibrary_361ED0BE_ios_min10.0
//
//  Created by chenlong on 8/17/20.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIImage.h>
#include <opencv2/core/mat.hpp>

NS_ASSUME_NONNULL_BEGIN

@interface HedEdgeDetectApi : NSObject

- (instancetype)initWithModelPath:(NSString *) modelPath;

- (void)run:(UIImage *) image points:(std::vector<cv::Point> &) points;

@end

NS_ASSUME_NONNULL_END
