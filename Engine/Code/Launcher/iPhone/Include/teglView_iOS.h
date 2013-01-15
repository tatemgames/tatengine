//
//  teglView_iOS.h
//  TatEngine
//
//  Created by Dmitrii Ivanov on 11/23/09.
//  Copyright Tatem Games 2009. All rights reserved.
//

#ifndef TE_TEGLVIEW_IOS_H
#define TE_TEGLVIEW_IOS_H

#include "TatEngineCoreConfig.h"

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>

//! glView
@interface glView : UIView
{
@private
	//! Current animating state
	BOOL CurrentAnimating;

	//! Display link support
	BOOL CurrentDisplayLinkSupported;

	//! Animation frame interval
	NSInteger CurrentAnimationFrameInterval;

	//! Current display link
	id CurrentDisplayLink;

	//! Current animation timer
	NSTimer * CurrentAnimationTimer;

	//! Current scale factor enabled
	BOOL CurrentScaleFactorEnabled;
}

//! Is Animating property
@property (readonly, nonatomic, getter = IsAnimating) BOOL CurrentAnimating;

//! Current animation frame interval property
@property (nonatomic) NSInteger CurrentAnimationFrameInterval;

//! Start animation
-(void)StartAnimation;

//! Stop animation
-(void)StopAnimation;

//! On Tick
-(void)OnTick:(id)sender;

@end

#endif
