//
//  teViewController_iOS.h
//  evo_iPhone
//
//  Created by Igor Leontiev on 10/27/11.
//  Copyright (c) 2011 Tatem Games. All rights reserved.
//

#ifndef TE_TEVIEWCONTROLLER_IOS_H
#define TE_TEVIEWCONTROLLER_IOS_H

#include "teApplicationManager.h"

#import <UIKit/UIKit.h>

@interface teViewController_iOS : UIViewController
{
	float scaleFactor;
}

@property (nonatomic) float scaleFactor;

@end

#endif