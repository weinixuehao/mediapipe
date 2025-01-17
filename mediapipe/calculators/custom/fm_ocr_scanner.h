//
//  fm_ocr_scanner.hpp
//  FMHEDNet
//
//  Created by fengjian on 2018/4/11.
//  Copyright © 2018年 fengjian. All rights reserved.
//

#ifndef fm_ocr_scanner_hpp
#define fm_ocr_scanner_hpp

#include <stdio.h>
#include <array>
#include <tuple>
#include "mediapipe/framework/port/opencv_core_inc.h"

std::tuple<bool, std::vector<cv::Point>, std::vector<cv::Mat>> ProcessEdgeImage(cv::Mat edge_image, cv::Mat color_image, bool strictMode, bool draw_debug_image);

#endif /* fm_ocr_scanner_hpp */
