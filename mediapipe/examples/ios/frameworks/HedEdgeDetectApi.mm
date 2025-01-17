//
//  HedEdgeDetect.m
//  _idx_HedEdgeDetectionAppLibrary_361ED0BE_ios_min10.0
//
//  Created by chenlong on 8/17/20.
//

#import "HedEdgeDetectApi.h"
#include "mediapipe/inference/hed_edge_detection.h"
#include "MMOpenCVHelper.h"

@implementation HedEdgeDetectApi {
    HedEdgeDetecion *_hedEdgeDetection;
}

- (instancetype)initWithModelPath:(NSString *) modelPath
{
    self = [super init];
    if (self) {
        std::string _modelPath = std::string([modelPath UTF8String]);
        self->_hedEdgeDetection = new HedEdgeDetecion(_modelPath);
    }
    return self;
}

- (void)run:(UIImage *) image points:(std::vector<cv::Point> &) points {
    cv::Mat inputImg = [MMOpenCVHelper cvMatFromUIImage:image];
    self->_hedEdgeDetection->run(inputImg, points);
}

- (void)dealloc
{
    delete _hedEdgeDetection;
}
@end
