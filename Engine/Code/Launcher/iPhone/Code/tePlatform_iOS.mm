//
//  tePlatform_iOS.mm
//  TatEngine
//
//  Created by Igor Leontiev on 05/13/11.
//  Copyright Tatem Games 2011. All rights reserved.
//

#include "tePlatform_iOS.h"
#include "teApplicationDelegate_iOS.h"

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

#ifdef TE_PUBLISHING_CHARTBOOST
	#include "tePublishingManager.h"
	#import "ChartBoost.h"
#endif

#ifdef TE_MODULE_SCENEMANAGER
	#include "teActorVideoPlayer.h"
#endif

#include <sys/sysctl.h>
#include <sys/types.h>

@interface teAlertViewDelegate : NSObject<UIAlertViewDelegate>

typedef void (^AlertViewCompletionBlock)(NSInteger buttonIndex);
@property (strong,nonatomic) AlertViewCompletionBlock callback;

+ (void)showAlertView:(UIAlertView *)alertView withCallback:(AlertViewCompletionBlock)callback;

@end

@implementation teAlertViewDelegate
@synthesize callback;

- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex {
	callback(buttonIndex);
}

+ (void)showAlertView:(UIAlertView *)alertView  withCallback:(AlertViewCompletionBlock)callback
{
	__block teAlertViewDelegate *delegate = [[teAlertViewDelegate alloc] init];
	alertView.delegate = delegate;
	delegate.callback = ^(NSInteger buttonIndex)
	{
		callback(buttonIndex);
		alertView.delegate = nil;
		delegate = nil;
	};
	[alertView show];
}

@end

namespace te
{
	namespace core
	{
		//! Constructor
		tePlatform_iOS::tePlatform_iOS()
			:videoCallbackActor(NULL)
		{
		}
			
		//! Destructor
		tePlatform_iOS::~tePlatform_iOS()
		{
		}
			
		//! Get Device UDID
		const te::c8 * tePlatform_iOS::GetDeviceUDID()
		{
			return [[[UIDevice currentDevice] uniqueIdentifier] cStringUsingEncoding:NSUTF8StringEncoding];
		}
		
		/*
		//! Open Link In Safari
		void tePlatform_iOS::OpenLink(teStringc link)
		{
			#ifdef TE_MODULE_PUBLISHING
				if(!pbl::GetPublishingManager()->IsReachable())
					[[[UIApplication sharedApplication] delegate] ShowAlert:@"No internet connection"];
				else
					[[UIApplication sharedApplication] openURL:[NSURL URLWithString:[NSString stringWithCString:link.c_str() encoding:NSUTF8StringEncoding]]];
			#endif
		}
		 */
		
		//! Show Chartboost More Apps Page
		void tePlatform_iOS::ShowChartbosstMoreApps()
		{
			#ifdef TE_PUBLISHING_CHARTBOOST
				[[ChartBoost sharedChartBoost] loadMoreApps];
			#endif
		}
		
		EDeviceType tePlatform_iOS::DefinePlatform()
		{
			size_t size;
			sysctlbyname("hw.machine", NULL, &size, NULL, 0);
			c8 machine[128];
			sysctlbyname("hw.machine", machine, &size, NULL, 0);

			if(strstr(machine, "iPhone1,1")) return DT_IOS_IPHONE_2G;
			if(strstr(machine, "iPhone1,2")) return DT_IOS_IPHONE_3G;
			if(strstr(machine, "iPhone2")) return DT_IOS_IPHONE_3GS;
			if(strstr(machine, "iPhone3")) return DT_IOS_IPHONE_4;
			if(strstr(machine, "iPhone4")) return DT_IOS_IPHONE_4S;
			if(strstr(machine, "iPhone5")) return DT_IOS_IPHONE_5;

			if(strstr(machine, "iPod1")) return DT_IOS_IPOD_1;
			if(strstr(machine, "iPod2")) return DT_IOS_IPOD_2;
			if(strstr(machine, "iPod3")) return DT_IOS_IPOD_3;
			if(strstr(machine, "iPod4")) return DT_IOS_IPOD_4;
			if(strstr(machine, "iPod5")) return DT_IOS_IPOD_5;

			if(strstr(machine, "iPad2,5")) return DT_IOS_IPAD_MINI_1;
			if(strstr(machine, "iPad2,6")) return DT_IOS_IPAD_MINI_1;
			if(strstr(machine, "iPad2,7")) return DT_IOS_IPAD_MINI_1;

			if(strstr(machine, "iPad1")) return DT_IOS_IPAD_1;
			if(strstr(machine, "iPad2")) return DT_IOS_IPAD_2;
			if(strstr(machine, "iPad3")) return DT_IOS_IPAD_3;
			if(strstr(machine, "iPad4")) return DT_IOS_IPAD_4;

			return DT_IOS_UNKNOWN;
		}
		
		void tePlatform_iOS::GetUserInputText(teGetUserInputCallback callback, te::teptr_t userData)
		{
			UIAlertView * alert = [[UIAlertView alloc] initWithTitle:@"Dream Gym"
															 message:@"         " 
															delegate:[[UIApplication sharedApplication] delegate]
												   cancelButtonTitle:@"Cancel" 
												   otherButtonTitles:@"Sign in", nil];
			
			UITextField *inputTextField = [[UITextField alloc] initWithFrame:CGRectMake(12.0, 45.0, 260.0, 25.0)];
			[inputTextField setBackgroundColor:[UIColor whiteColor]];
			[alert addSubview:inputTextField];
			
			[teAlertViewDelegate showAlertView:alert withCallback:^(NSInteger buttonIndex)
			{
				if(buttonIndex == 1)
				{
					callback((c8*)[inputTextField.text cStringUsingEncoding:NSASCIIStringEncoding], userData);
				}
			}];

		}
		
		void tePlatform_iOS::AskUserQuestion(teString & question, teGetUserInputCallback callback, te::teptr_t userData)
		{
			UIAlertView * alert = [[UIAlertView alloc] initWithTitle:@"Dream Gym"
															 message:[NSString stringWithUTF8String:question.c_str()]
															delegate:[[UIApplication sharedApplication] delegate]
												   cancelButtonTitle:@"NO" 
												   otherButtonTitles:@"YES", nil];
			
			[teAlertViewDelegate showAlertView:alert withCallback:^(NSInteger buttonIndex)
			 {
				 if(buttonIndex == 1)
				 {
					 callback((c8*)"yes", userData);
				 }
				 else
				 {
					 callback((c8*)"no", userData);
				 }
			 }];
		}
		
		f32 tePlatform_iOS::GetBrightness()
		{
			if([[[UIDevice currentDevice] systemVersion] floatValue] >= 5.0f)
				return [UIScreen mainScreen].brightness;
			else
				return 0.5f;
				
		}
		
		void tePlatform_iOS::PlayVideo(u8 index, void * callbackActor)
		{
			videoCallbackActor = callbackActor;
			ApplicationDelegate * delg = (ApplicationDelegate*)[UIApplication sharedApplication].delegate;

			[delg.movieController.view setHidden:NO];
			[delg.movieController setContentURL:[NSURL fileURLWithPath:[[NSBundle mainBundle] pathForResource:@"0" ofType:@"mov"]]];
			[delg.movieController play];
		}
		
		void tePlatform_iOS::OnVideoFinished()
		{
			#ifdef TE_MODULE_SCENEMANAGER
			if(!videoCallbackActor) return;
			static_cast<scene::teActorVideoPlayer*>(videoCallbackActor)->OnFinished();
			#endif
		}
	}
}