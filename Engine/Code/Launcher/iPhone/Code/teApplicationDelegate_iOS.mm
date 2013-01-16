//
//  teApplicationDelegateiPhone.mm
//  TatEngine
//
//  Created by Dmitrii Ivanov on 11/23/09.
//  Copyright 2009 Tatem Games. All rights reserved.
//

#import "teApplicationDelegate_iOS.h"
#include "teEngineController.h"
#include "teInputManager.h"
#include "tePlatform.h"
//#include "teHTTPNetwork.h"

#ifdef TE_PLATFORM_IPHONE
	#import <Foundation/Foundation.h>
	#include "tePlatform_iOS.h"

	#ifdef TE_MODULE_PUBLISHING
		#import <Parse/Parse.h>
		#import "Facebook.h"
	#endif
#endif

//! iOS File System
class CiOSFileSystem : public te::core::IFileSystem
{
public:
	//! Get Path
	te::teString GetPath(te::core::EFilePathType Type)
	{
		switch(Type)
		{
		case te::core::FPT_USER_RESOURCES:
			{
				NSAutoreleasePool * autoreleasePool = [[NSAutoreleasePool alloc] init];
				NSArray * paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);

				char * test;

				if([paths count])
				{
					const char * sa = [[paths objectAtIndex:0] UTF8String];

					test = (char*)malloc(strlen(sa) + 2);

					memcpy(test, sa, strlen(sa));
					test[strlen(sa)] = '/';
					test[strlen(sa) + 1] = '\0';
				}
				else
				{
					test = (char*)malloc(1);
				}

				[autoreleasePool release];
				return te::teString(test); // TODO FIX THIS SHIT
			}
		case te::core::FPT_RESOURCES_UNPACKED:
			{
				NSAutoreleasePool * autoreleasePool = [[NSAutoreleasePool alloc] init];
				NSArray * paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);

				char * test;

				if([paths count])
				{
					const char * sa = [[[paths objectAtIndex:0] stringByAppendingPathComponent:@"Unpacked"] UTF8String];

					test = (char*)malloc(strlen(sa) + 2);

					memcpy(test, sa, strlen(sa));
					test[strlen(sa)] = '/';
					test[strlen(sa) + 1] = '\0';
				}
				else
				{
					test = (char*)malloc(1);
				}

				[autoreleasePool release];
				return te::teString(test); // TODO FIX THIS SHIT
			}
		case te::core::FPT_DEFAULT:
		case te::core::FPT_RESOURCES:
		case te::core::FPT_CONTENT:
		default:
			{
				char * test;

				const char * sa = [[[NSBundle mainBundle] resourcePath] UTF8String];
				
				test = (char*)malloc(strlen(sa) + 2);
				
				memcpy(test, sa, strlen(sa));
				test[strlen(sa)] = '/';
				test[strlen(sa) + 1] = '\0';
				
				NSAutoreleasePool * autoreleasePool = [[NSAutoreleasePool alloc] init];
				//te::teStringw path = te::teStringw([[[NSBundle mainBundle] resourcePath] UTF8String]) + te::teStringw("/");
				[autoreleasePool release];
				return te::teString(test);
			}
		}
	}
};

//! Application Delegate
@implementation ApplicationDelegate

@synthesize Window;
@synthesize View;
@synthesize movieController = _movieController;

- (BOOL)application:(UIApplication *)application willFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
	[[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
	
	loadingView = nil;
	
	UIAccelerometer * Accelerometer = [UIAccelerometer sharedAccelerometer];
	Accelerometer.delegate = self;
	Accelerometer.updateInterval = 0.05;
	
	[Window setFrame:[[UIScreen mainScreen] bounds]];
	
	te::core::InitEngine();

	te::core::IFileSystem * iOSFileSystem = NULL;
	TE_NEW(iOSFileSystem, CiOSFileSystem);
	te::core::GetPlatform()->SetFileSystem(iOSFileSystem);
	iOSFileSystem->Drop();

	te::core::tePlatform_iOS * PlatformiPhone;
	TE_NEW(PlatformiPhone, te::core::tePlatform_iOS)
	te::core::GetPlatform()->SetCurrentDevicePlatform(PlatformiPhone);
	PlatformiPhone->Drop();
	
	self.movieController = [[MPMoviePlayerController alloc] init];//initWithContentURL:[NSURL fileURLWithPath:[[NSBundle mainBundle] pathForResource:@"intro" ofType:@"mov"]]];
	
	//self.movieController.controlStyle = MPMovieControlStyleNone;
	self.movieController.shouldAutoplay = NO;
	self.movieController.view.frame = CGRectMake(0, 0, 1024, 768);
	
	[self.View addSubview:self.movieController.view];
	[self.movieController.view setHidden:YES];
	
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(moviePlayBackDidFinish:) name:MPMoviePlayerPlaybackDidFinishNotification object:self.movieController];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(moviePlayBackDidFinish:) name:MPMoviePlayerPlaybackDidFinishReasonUserInfoKey object:self.movieController];
	
	//[self.movieController setFullscreen:YES animated:YES];
	//[self.movieController prepareToPlay];
	
	//UIControl * overlay = [[[UIControl alloc] initWithFrame:CGRectMake(0, 0, 1024, 768)] autorelease];
	//[overlay addTarget:self action:@selector(movieWindowTouched:) forControlEvents:UIControlEventTouchDown];
	//[self.movieController.view addSubview:overlay];
	

	TE_NEW_S(te::app::teApplicationManager(false))
	[View layoutSubviews];
	te::app::GetApplicationManager()->InitApplication();
}

-(void)moviePlayBackDidFinish:(NSNotification*)notification
{
	//if(self.movieController.view.hidden == YES)
	//	return;
	
	[self.movieController.view setHidden:YES];
	
	((te::core::tePlatform_iOS*)te::core::GetPlatform()->GetCurrentDevicePlatform())->OnVideoFinished();
}

//! On finish launching
- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary*)launchOptions
{
	//[[UIApplication sharedApplication] setStatusBarHidden:YES animated:YES];
	//[Application setStatusBarOrientation:UIInterfaceOrientationLandscapeRight animated:YES];
	
	if([[[UIDevice currentDevice] systemVersion] doubleValue] < 6.0)
	{
		[self application:application willFinishLaunchingWithOptions:launchOptions];
	}
	
	#ifdef TE_MODULE_PUBLISHING
		[application registerForRemoteNotificationTypes:UIRemoteNotificationTypeBadge|UIRemoteNotificationTypeAlert|UIRemoteNotificationTypeSound];

		UILocalNotification * localNotification = [launchOptions objectForKey:UIApplicationLaunchOptionsLocalNotificationKey];

		if(localNotification)
			[application setApplicationIconBadgeNumber:localNotification.applicationIconBadgeNumber - 1];

		[Parse setApplicationId:[[[[NSBundle mainBundle] infoDictionary] objectForKey:@"Social"] objectForKey:@"ParseAppID"] clientKey:[[[[NSBundle mainBundle] infoDictionary] objectForKey:@"Social"] objectForKey:@"ParseAppKey"]];
	#endif

	[[Window rootViewController] didRotateFromInterfaceOrientation:[[UIApplication sharedApplication] statusBarOrientation]];
	
	[View StartAnimation];
}

//! Pause applicaton
- (void)applicationWillResignActive:(UIApplication *)Application
{
	((te::core::tePlatform_iOS*)te::core::GetPlatform()->GetCurrentDevicePlatform())->OnWillResignActive();
	[View StopAnimation];
}

//! Enter background
- (void)applicationDidEnterBackground:(UIApplication *)application
{
	((te::core::tePlatform_iOS*)te::core::GetPlatform()->GetCurrentDevicePlatform())->OnDidEnterBackground();
}

//! Enter foreground
- (void)applicationWillEnterForeground:(UIApplication *)application
{
	((te::core::tePlatform_iOS*)te::core::GetPlatform()->GetCurrentDevicePlatform())->OnWillEnterForeground();
}

//! Restore application
- (void)applicationDidBecomeActive:(UIApplication *)Application
{
	[View StartAnimation];
	((te::core::tePlatform_iOS*)te::core::GetPlatform()->GetCurrentDevicePlatform())->OnBecomeActive();
}

//! Terminate application
- (void)applicationWillTerminate:(UIApplication *)Application
{
	((te::core::tePlatform_iOS*)te::core::GetPlatform()->GetCurrentDevicePlatform())->OnWillTerminate();

	te::app::GetApplicationManager()->Drop();
	te::core::CloseEngine();
	
	[View StopAnimation];
}

//! On memory warning
- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application
{
	te::app::GetApplicationManager()->OnMemoryWarning();
}

- (void)application:(UIApplication *)application didRegisterForRemoteNotificationsWithDeviceToken:(NSData *)newDeviceToken
{
	#ifdef TE_MODULE_PUBLISHING
		[PFPush storeDeviceToken:newDeviceToken];
		[PFPush subscribeToChannelInBackground:@"" target:self selector:@selector(subscribeFinished:error:)];
	#endif
}

- (void)application:(UIApplication *)application didFailToRegisterForRemoteNotificationsWithError:(NSError *)error
{
	NSLog(@"application:didFailToRegisterForRemoteNotificationsWithError: %@", error);
}

- (void)subscribeFinished:(NSNumber *)result error:(NSError *)error
{
    if ([result boolValue])
	{
        NSLog(@"DreamGym successfully subscribed to push notifications on the broadcast channel.");
    }
	else
	{
        NSLog(@"DreamGym failed to subscribe to push notifications on the broadcast channel.");
    }
}

- (void)application:(UIApplication *)application didReceiveRemoteNotification:(NSDictionary *)userInfo
{
	#ifdef TE_MODULE_PUBLISHING
		[PFPush handlePush:userInfo];
	#endif
}

- (void)application:(UIApplication *)application didReceiveLocalNotification:(UILocalNotification *)notification
{
	[application setApplicationIconBadgeNumber:notification.applicationIconBadgeNumber - 1];
}

- (NSUInteger)application:(UIApplication *)application supportedInterfaceOrientationsForWindow:(UIWindow *)window
{
	return UIInterfaceOrientationMaskAll;
}

//! On accelerometer
- (void)accelerometer:(UIAccelerometer*)accelerometer didAccelerate:(UIAcceleration*)Acceleration
{
	te::input::GetInputManager()->SetAccelerometer(te::teVector3df(Acceleration.x, Acceleration.y, Acceleration.z));
}

- (void)ShowLoadingView
{
	if(!loadingView)
	{
		loadingView = [[UIView alloc] initWithFrame:Window.frame];
		[loadingView setBackgroundColor:[UIColor blackColor]];
		[loadingView setAlpha:0.75f];

		UIActivityIndicatorView * activityView = [[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleWhiteLarge];
		[activityView setCenter:loadingView.center];
		[activityView startAnimating];
		[loadingView addSubview:activityView];
		[activityView release];

		[Window addSubview:loadingView];
		[loadingView release];
	}
}

- (void)HideLoadingView
{
	if(loadingView)
	{
		[loadingView removeFromSuperview];
		loadingView = nil;
	}
}

- (void)ShowGameCenterDisabledAlert
{
	UIAlertView * alert = [[UIAlertView alloc] initWithTitle:@"Game Center Disabled"
													 message:@"Sign in with the Game Center application to enable." 
													delegate:self 
										   cancelButtonTitle:@"Cancel" 
										   otherButtonTitles:@"Sign in", nil];
	[alert setTag:101];
	[alert show];
	[alert release];
}

- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex
{
	if(([alertView tag] == 101) && (buttonIndex == 1))
	{
		[[UIApplication sharedApplication] openURL:[NSURL URLWithString:@"gamecenter:"]];
	}
}

- (void)leaderboardViewControllerDidFinish:(GKLeaderboardViewController *)viewController
{
	((te::core::tePlatform_iOS*)te::core::GetPlatform())->GetUserInputText(NULL, NULL);
	
	[Window.rootViewController dismissModalViewControllerAnimated:NO];
	[viewController release];
	[View StartAnimation];
}

- (void)achievementViewControllerDidFinish:(GKAchievementViewController *)viewController
{
	[Window.rootViewController dismissModalViewControllerAnimated:NO];
	[viewController release];
	[View StartAnimation];
}

- (void)ShowAlert:(NSString*)text
{
	UIAlertView * alert = [[UIAlertView alloc] initWithTitle:nil
													 message:text 
													delegate:nil 
										   cancelButtonTitle:@"OK" 
										   otherButtonTitles:nil];
	[alert show];
	[alert release];
}

- (BOOL)application:(UIApplication *)application handleOpenURL:(NSURL *)url
{
	#ifdef TE_MODULE_PUBLISHING
		return [FBSession.activeSession handleOpenURL:url];
	#endif
	return NO;
}

- (BOOL)application:(UIApplication *)application openURL:(NSURL *)url sourceApplication:(NSString *)sourceApplication annotation:(id)annotation
{
	#ifdef TE_MODULE_PUBLISHING
		return [FBSession.activeSession handleOpenURL:url];
	#endif
	return NO;
}

//! Dealloc
- (void) dealloc
{
	[Window release];
	[View release];
	
	[super dealloc];
}

@end
