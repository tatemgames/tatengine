//
//  teViewController_iOS.m
//  evo_iPhone
//
//  Created by Igor Leontiev on 10/27/11.
//  Copyright (c) 2011 Tatem Games. All rights reserved.
//

#import "teViewController_iOS.h"

#include "teApplicationDelegate_iOS.h"

#include "tePlatform.h"

@implementation teViewController_iOS

@synthesize scaleFactor;

- (void)viewDidLoad
{
	[super viewDidLoad];

	[[(ApplicationDelegate*)[[UIApplication sharedApplication] delegate] View] setFrame:[[UIScreen mainScreen] bounds]];
	[self.view setFrame:[[UIScreen mainScreen] bounds]];
	
	scaleFactor = 1.0f;
	
	if([[[UIDevice currentDevice] systemVersion] doubleValue] >= 4.0)
	{
		#ifdef TE_LAUNCHER_IOS_SUPPORT_IPHONE_RETINA
		if([UIScreen mainScreen].currentMode.size.width == 640)
			scaleFactor = 2.0f;
		#endif
		#ifdef TE_LAUNCHER_IOS_SUPPORT_IPAD_RETINA
		 if(([UIScreen mainScreen].currentMode.size.width == 2048) || ([UIScreen mainScreen].currentMode.size.width == 1536))
			scaleFactor = 2.0f;
		#endif
	}
}

- (NSUInteger)supportedInterfaceOrientations
{
	return TE_LAUNCHER_IOS_START_ORIENTATION;

	// TODO platform doesnt exist when this code executes
	/*
	if(!te::core::GetPlatform())
		return UIInterfaceOrientationMaskPortrait;
	
	NSUInteger supportedOrientationsMask = 0;

	te::u32 DeviceOrientationMask = te::core::GetPlatform()->GetDeviceOrientationMask();

	if(DeviceOrientationMask & te::core::DO_NORMAL)
		supportedOrientationsMask |= UIInterfaceOrientationMaskPortrait;
	if(DeviceOrientationMask & te::core::DO_DOWN)
		supportedOrientationsMask |= UIInterfaceOrientationPortraitUpsideDown;
	if(DeviceOrientationMask & te::core::DO_LEFT)
		supportedOrientationsMask |= UIInterfaceOrientationLandscapeLeft;
	if(DeviceOrientationMask & te::core::DO_RIGHT)
		supportedOrientationsMask |= UIInterfaceOrientationLandscapeRight;

	return supportedOrientationsMask;
	*/
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
{
	te::u32 DeviceOrientationMask = te::core::DO_NORMAL;
	
	if( te::core::GetPlatform() )
		DeviceOrientationMask = te::core::GetPlatform()->GetDeviceOrientationMask();
	
	te::core::EDeviceOrientation DeviceOrientation;
	
	switch (toInterfaceOrientation) {
		case UIInterfaceOrientationPortrait:
			DeviceOrientation = te::core::DO_NORMAL;
			break;
		case UIInterfaceOrientationPortraitUpsideDown:
			DeviceOrientation = te::core::DO_DOWN;
			break;
		case UIInterfaceOrientationLandscapeLeft:
			DeviceOrientation = te::core::DO_LEFT;
			break;
		case UIInterfaceOrientationLandscapeRight:
			DeviceOrientation = te::core::DO_RIGHT;
			break;
		default:
			return NO;
			break;
	}
	
	if( DeviceOrientationMask & DeviceOrientation )
	{
		return YES;
	}
	
	return NO;
}

- (void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)fromInterfaceOrientation
{
	[super didRotateFromInterfaceOrientation:fromInterfaceOrientation];

	UIInterfaceOrientation DeviceOrientation = [[UIApplication sharedApplication] statusBarOrientation];
	
	te::core::EDeviceOrientation teDeviceOrientation;
	
	switch (DeviceOrientation) {
		case UIInterfaceOrientationPortraitUpsideDown:
			teDeviceOrientation = te::core::DO_DOWN;
			break;
		case UIInterfaceOrientationLandscapeLeft:
			teDeviceOrientation = te::core::DO_LEFT;
			break;
		case UIInterfaceOrientationLandscapeRight:
			teDeviceOrientation = te::core::DO_RIGHT;
			break;
		default:
			teDeviceOrientation = te::core::DO_NORMAL;
			break;
	}
	
	te::core::GetPlatform()->SetDeviceOrientation(teDeviceOrientation);
	
	te::f32 screenWidth = [[UIScreen mainScreen] bounds].size.width;
	te::f32 screenHeight = [[UIScreen mainScreen] bounds].size.height;
	
	if( UIInterfaceOrientationIsPortrait(DeviceOrientation) )
	{
		[[(ApplicationDelegate*)[[UIApplication sharedApplication] delegate] View] setFrame:CGRectMake(0.0f, 0.0f, screenWidth, screenHeight)];
		te::app::GetApplicationManager()->OnResize(te::teVector2df(screenWidth * scaleFactor, screenHeight * scaleFactor));
	}
	else if( UIInterfaceOrientationIsLandscape(DeviceOrientation) )
	{
		[[(ApplicationDelegate*)[[UIApplication sharedApplication] delegate] View] setFrame:CGRectMake(0.0f, 0.0f, screenHeight, screenWidth)];
		te::app::GetApplicationManager()->OnResize(te::teVector2df(screenHeight * scaleFactor, screenWidth * scaleFactor));
	}
}

- (void)willRotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation duration:(NSTimeInterval)duration
{
	[super willRotateToInterfaceOrientation:toInterfaceOrientation duration:0.0f];
}

@end
