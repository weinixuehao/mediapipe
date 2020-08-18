//
//  UIImage+PictureOrientation.m
//  DocEdgeCrop
//
//  Created by chenlong on 7/17/19.
//  Copyright © 2019 ctsmed.com. All rights reserved.
//

#import "UIImage+PictureOrientation.h"

@implementation UIImage (PictureOrientation)

- (UIImage *)normalizedImage {
    
    if (self.imageOrientation == UIImageOrientationUp) return self;
    
    UIGraphicsBeginImageContextWithOptions(self.size, NO, self.scale);
    [self drawInRect:(CGRect){0, 0, self.size}];
    UIImage *normalizedImage = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    
    return normalizedImage;
}

@end
