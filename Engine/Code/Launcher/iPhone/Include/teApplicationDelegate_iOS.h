//
//  teApplicationDelegateiPhone.h
//  TatEngine
//
//  Created by Dmitrii Ivanov on 11/23/09.
//  Copyright 2009 Tatem Games. All rights reserved.
//

#ifndef TE_TEAPPLICATIONDELEGATEIPHONE_H
#define TE_TEAPPLICATIONDELEGATEIPHONE_H

#include "TatEngineCoreConfig.h"
#include "teApplicationManager.h"
#import "teglView_iOS.h"
#import "teViewController_iOS.h"

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <UIKit/UIAccelerometer.h>
#import <GameKit/GameKit.h>
#import <MediaPlayer/MPMoviePlayerController.h>
#import <MessageUI/MessageUI.h>

//! Application Delegate
@interface ApplicationDelegate : NSObject <UIApplicationDelegate, UIAccelerometerDelegate, GKLeaderboardViewControllerDelegate, GKAchievementViewControllerDelegate, UIAlertViewDelegate, MFMailComposeViewControllerDelegate>
{
@private
	//! Current Window
    UIWindow * Window;
	
	//! Current View
	glView * View;
	
	UIView * loadingView;
}

//! Window property
@property (nonatomic, retain) IBOutlet UIWindow * Window;

//! View property
@property (nonatomic, retain) IBOutlet glView * View;

@property (nonatomic, retain) IBOutlet teViewController_iOS * ViewController;

@property (nonatomic, retain) MPMoviePlayerController * movieController;

- (void)ShowLoadingView;
- (void)HideLoadingView;

- (void)ShowGameCenterDisabledAlert;
- (void)ShowAlert:(NSString*)text;

- (void)SendMail:(NSString*)url;

- (void)sgnbar;

@end

#endif
