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

#ifdef TE_MODULE_PUBLISHING
	#include "tePublishingManager.h"
#endif

#ifdef TE_PUBLISHING_CHARTBOOST
	#import "ChartBoost.h"
#endif

#ifdef TE_MODULE_SCENEMANAGER
	#include "teActorVideoPlayer.h"
#endif

#include <sys/sysctl.h>
#include <sys/types.h>

NSMutableArray * alertdelegates = nil;

@interface teAlertViewDelegate : NSObject<UIAlertViewDelegate>

typedef void (^AlertViewCompletionBlock)(NSInteger buttonIndex);
@property (strong,nonatomic) AlertViewCompletionBlock callback;
@property (strong,nonatomic) UIAlertView * view;

+ (void)showAlertView:(UIAlertView *)alertView withCallback:(AlertViewCompletionBlock)callback;

- (void)dismiss;

@end

@implementation teAlertViewDelegate
@synthesize callback;

- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex {
	callback(buttonIndex);
}

- (void)dismiss
{
	[self.view dismissWithClickedButtonIndex:[self.view cancelButtonIndex] animated:NO];
}

+ (void)showAlertView:(UIAlertView *)alertView  withCallback:(AlertViewCompletionBlock)callback
{
	__block teAlertViewDelegate *delegate = [[teAlertViewDelegate alloc] init];
	alertView.delegate = delegate;
	delegate.view = alertView;
	
	if(alertdelegates == nil)
		alertdelegates = [[NSMutableArray alloc] init];
	[alertdelegates addObject:delegate];
	
	delegate.callback = ^(NSInteger buttonIndex)
	{
		[alertdelegates removeObject:delegate];
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
			//if([[[UIDevice currentDevice] systemVersion] floatValue] >= 6.0f)
			static c8 temp[128];
			strcpy(temp, [[[[UIDevice currentDevice] identifierForVendor] UUIDString] cStringUsingEncoding:NSUTF8StringEncoding]);
			return temp;
			//else
			//	return [[[UIDevice currentDevice] uniqueIdentifier] cStringUsingEncoding:NSUTF8StringEncoding];
		}
		
		//! Open Link In Safari
		void tePlatform_iOS::OpenLink(teString link)
		{
			#ifdef TE_MODULE_PUBLISHING
				if(!pbl::GetPublishingManager()->IsReachable())
					[[[UIApplication sharedApplication] delegate] ShowAlert:@"No internet connection"];
				else
					[[UIApplication sharedApplication] openURL:[NSURL URLWithString:[NSString stringWithCString:link.c_str() encoding:NSUTF8StringEncoding]]];
			#endif
		}
		
		void tePlatform_iOS::SendMail(teString url)
		{
			[[[UIApplication sharedApplication] delegate] SendMail:[NSString stringWithCString:url.c_str() encoding:NSUTF8StringEncoding]];
		}
		
		//! Show Chartboost More Apps Page
		void tePlatform_iOS::ShowChartbosstMoreApps()
		{
			#ifdef TE_PUBLISHING_CHARTBOOST
				[[ChartBoost sharedChartBoost] loadMoreApps];
			#endif
		}
		

		void tePlatform_iOS::DismissAllAlertView()
		{
			if(alertdelegates != nil)
				for(teAlertViewDelegate * delg in alertdelegates)
					[delg dismiss];
		}
		
		EDeviceType tePlatform_iOS::DefinePlatform()
		{
			size_t size;
			sysctlbyname("hw.machine", NULL, &size, NULL, 0);
			c8 machine[128];
			sysctlbyname("hw.machine", machine, &size, NULL, 0);

			if(strstr(machine, "iPhone5,3")) return DT_IOS_IPHONE_5C;
			if(strstr(machine, "iPhone5,4")) return DT_IOS_IPHONE_5C;

			if(strstr(machine, "iPhone1,1")) return DT_IOS_IPHONE_2G;
			if(strstr(machine, "iPhone1,2")) return DT_IOS_IPHONE_3G;
			if(strstr(machine, "iPhone2")) return DT_IOS_IPHONE_3GS;
			if(strstr(machine, "iPhone3")) return DT_IOS_IPHONE_4;
			if(strstr(machine, "iPhone4")) return DT_IOS_IPHONE_4S;
			if(strstr(machine, "iPhone5")) return DT_IOS_IPHONE_5;
			if(strstr(machine, "iPhone6")) return DT_IOS_IPHONE_5S;

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
		
		void tePlatform_iOS::GetUserInputText(const teString & title, const teString & question, const teString & yesBtn, const teString & noBtn, teGetUserInputCallback callback, te::teptr_t userData)
		{
			UIAlertView * alert = nil;

			alert = [[UIAlertView alloc] initWithTitle:[NSString stringWithUTF8String:title.c_str()]
												   message:[NSString stringWithUTF8String:question.c_str()]
												  delegate:[[UIApplication sharedApplication] delegate]
										 cancelButtonTitle:[NSString stringWithUTF8String:noBtn.c_str()]
										 otherButtonTitles:[NSString stringWithUTF8String:yesBtn.c_str()], nil];
			
			UITextField *inputTextField = [[UITextField alloc] initWithFrame:CGRectMake(12.0, 45.0, 260.0, 25.0)];
			[inputTextField setBackgroundColor:[UIColor whiteColor]];
			[alert addSubview:inputTextField];
			
			[teAlertViewDelegate showAlertView:alert withCallback:^(NSInteger buttonIndex)
			{
				if(buttonIndex == 1)
				{
					callback((const c8*)[inputTextField.text cStringUsingEncoding:NSASCIIStringEncoding], userData);
				}
			}];

		}
		
		void tePlatform_iOS::AskUserQuestion(const teString & title, const teString & question, const teString & yesBtn, const teString & noBtn, teGetUserInputCallback callback, te::teptr_t userData)
		{
			UIAlertView * alert = nil;
			
			if(noBtn.c_str() == NULL)
			{
				alert = [[UIAlertView alloc] initWithTitle:[NSString stringWithUTF8String:title.c_str()]
												   message:[NSString stringWithUTF8String:question.c_str()]
												  delegate:[[UIApplication sharedApplication] delegate]
										 cancelButtonTitle:nil
										 otherButtonTitles:[NSString stringWithUTF8String:yesBtn.c_str()], nil];
			}
			else
			{
				alert = [[UIAlertView alloc] initWithTitle:[NSString stringWithUTF8String:title.c_str()]
															 message:[NSString stringWithUTF8String:question.c_str()]
															delegate:[[UIApplication sharedApplication] delegate]
												   cancelButtonTitle:[NSString stringWithUTF8String:noBtn.c_str()] 
												   otherButtonTitles:[NSString stringWithUTF8String:yesBtn.c_str()], nil];
			}
			
			[teAlertViewDelegate showAlertView:alert withCallback:^(NSInteger buttonIndex)
			 {
				 if(buttonIndex == 1)
				 {
					 callback("yes", userData);
				 }
				 else
				 {
					 callback("no", userData);
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
			[delg.movieController setContentURL:[NSURL fileURLWithPath:[[NSBundle mainBundle] pathForResource:[[NSNumber numberWithInt:(int)index] stringValue] ofType:@"mov"]]];
			[delg.movieController play];
		}
		
		void tePlatform_iOS::OnVideoFinished()
		{
			#ifdef TE_MODULE_SCENEMANAGER
			if(!videoCallbackActor) return;
			static_cast<scene::teActorVideoPlayer*>(videoCallbackActor)->OnFinished();
			#endif
		}
		
		void tePlatform_iOS::ReadUserPref(const c8 * name, c8 * returnArray, u32 returnArraySize, u32 * resultSize)
		{
			NSUserDefaults * prefs = [NSUserDefaults standardUserDefaults];
			NSString * data = [prefs stringForKey:[NSString stringWithUTF8String:name]];
			
			u32 res = 0;
			
			if(data != nil)
			{
				const c8 * resData = [data UTF8String];
				res = teUTF8GetSize(resData);
				if(res)
				{
					memset(returnArray, 0, returnArraySize);
					memcpy(returnArray, resData, teMin(res, returnArraySize));
				}
				
			}
			
			if(resultSize)
				*resultSize = res;
		}
		
		const c8 * tePlatform_iOS::GetDeviceLocale()
		{
			static c8 temp[128];
			NSString * str = [[[NSBundle mainBundle] preferredLocalizations] objectAtIndex:0];
			strcpy(temp, [str UTF8String]);
			return temp;
		}
	}
}