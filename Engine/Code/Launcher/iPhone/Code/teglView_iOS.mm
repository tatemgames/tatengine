//
//  teglView_iOS.m
//  TatEngine
//
//  Created by Dmitrii Ivanov on 11/23/09.
//  Copyright Tatem Games 2009. All rights reserved.
//

#import "teglView_iOS.h"
#import <OpenGLES/EAGL.h>
#import <OpenGLES/EAGLDrawable.h>
#include "teApplicationManager.h"
#include "teRenderGL.h"
#include "teInputManager.h"
#include "teTimer.h"
#include "teFpsCounter.h"
#include "teRenderGLExtensions.h"

//! glView
@implementation glView

@synthesize CurrentAnimating;
@dynamic CurrentAnimationFrameInterval;

+(Class)layerClass
{
	return [CAEAGLLayer class];
}

//! Init
-(id)initWithCoder:(NSCoder*)coder
{
	if((self = [super initWithCoder:coder]))
	{
		CAEAGLLayer * EAGLLayer = (CAEAGLLayer *)self.layer;
		EAGLLayer.opaque = true;
		EAGLLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];

		CurrentAnimating = false;
		CurrentDisplayLinkSupported = false;
		
		CurrentAnimationFrameInterval = 1;
		
		CurrentDisplayLink = nil;
		CurrentAnimationTimer = nil;

		NSString * RequiedSystemVersionForDisplayLink = @"3.1";
		NSString * CurrSystemVersion = [[UIDevice currentDevice] systemVersion];
		
		if([CurrSystemVersion compare:RequiedSystemVersionForDisplayLink options:NSNumericSearch] != NSOrderedAscending)
			CurrentDisplayLinkSupported = true;

		if([[[UIDevice currentDevice] systemVersion] doubleValue] >= 4.0)
		{
			CurrentScaleFactorEnabled = YES;
			
			// TODO find better solution
			#ifdef TE_LAUNCHER_IOS_SUPPORT_IPHONE_RETINA
			if([UIScreen mainScreen].currentMode.size.width == 640)
				self.contentScaleFactor = 2.0f;
			#endif
			#ifdef TE_LAUNCHER_IOS_SUPPORT_IPAD_RETINA
			if([UIScreen mainScreen].currentMode.size.width == 2048)
				self.contentScaleFactor = 2.0f;
			#endif
		}
	}
	return self;
}

//! Start Animation
-(void)StartAnimation
{
	if(!CurrentAnimating)
	{
		if(CurrentDisplayLinkSupported)
		{
			CurrentDisplayLink = [NSClassFromString(@"CADisplayLink") displayLinkWithTarget:self selector:@selector(OnTick:)];
			[CurrentDisplayLink setFrameInterval:CurrentAnimationFrameInterval];
			[CurrentDisplayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
		}
		else
			CurrentAnimationTimer = [NSTimer scheduledTimerWithTimeInterval:(NSTimeInterval)((1.0 / 60.0) * CurrentAnimationFrameInterval) target:self selector:@selector(OnTick:) userInfo:nil repeats:TRUE];
		
		CurrentAnimating = TRUE;
	}
}

//! Stop Animation
-(void)StopAnimation
{
	if(CurrentAnimating)
	{
		if(CurrentDisplayLinkSupported)
		{
			[CurrentDisplayLink invalidate];
			CurrentDisplayLink = nil;
		}
		else
		{
			[CurrentAnimationTimer invalidate];
			CurrentAnimationTimer = nil;
		}
		
		CurrentAnimating = FALSE;
	}
}

//! On Tick
-(void)OnTick:(id)sender
{
	te::app::GetApplicationManager()->OnTick();
}

//! On resize
-(void)layoutSubviews
{
	te::app::GetApplicationManager()->GetFrameBuffer()->textures[te::video::FBL_COLOR_0]->Bind();
	[(EAGLContext*)te::video::GetRender()->GetContext()->GetGLContext() renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer*)self.layer];
	
	te::app::GetApplicationManager()->GetFrameBuffer()->textures[te::video::FBL_COLOR_0]->UpdateSize();

	if(te::app::GetApplicationManager()->GetFrameBuffer()->textures[te::video::FBL_DEPTH])
	{
		const te::teVector2di & Size = te::app::GetApplicationManager()->GetFrameBuffer()->textures[te::video::FBL_COLOR_0]->size;
		te::app::GetApplicationManager()->GetFrameBuffer()->textures[te::video::FBL_DEPTH]->Bind();
		te::video::tglRenderbufferStorage(GL_DEPTH_COMPONENT24_OES, Size.x, Size.y);
	}

	te::app::GetApplicationManager()->OnResize();
	
	te::app::GetApplicationManager()->GetFrameBuffer()->Bind();

    [self OnTick:nil];
}

//! Current Animation Frame Interval property
-(NSInteger)CurrentAnimationFrameInterval
{
	return CurrentAnimationFrameInterval;
}

//! Set Animation Frame Interval
-(void)setAnimationFrameInterval:(NSInteger)FrameInterval
{
	if(FrameInterval >= 1)
	{
		CurrentAnimationFrameInterval = FrameInterval;
		
		if(CurrentAnimating)
		{
			[self StopAnimation];
			[self StartAnimation];
		}
	}
}

//! On Touch
-(void)OnTouch:(NSSet*)Touches:(te::input::ETouchEventType)Type
{
	te::input::teTouchEvent * Array = te::input::GetInputManager()->LockTouch();
	
	te::u32 i = 0;
	for(UITouch * Touch in Touches)
	{
		CGPoint Point = [Touch locationInView:self];
		
		if(CurrentScaleFactorEnabled)
			Array[i].Position.SetXY(Point.x * self.contentScaleFactor, Point.y * self.contentScaleFactor);
		else
			Array[i].Position.SetXY(Point.x, Point.y);
		
		Array[i].TouchIdentifier = Touch;
		
		++i;
		
		if(i >= TE_INPUT_TOUCHES_MAX)
			break;
	}
	
	te::input::GetInputManager()->UnlockTouch(Type, i);
}

//! Touch began
-(void)touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event
{
	[self OnTouch:touches:te::input::TET_BEGIN];
}

//! Touch end
-(void)touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event
{
	[self OnTouch:touches:te::input::TET_END];
}

//! Touch cancelled
-(void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
	[self OnTouch:touches:te::input::TET_CANCELLED];
}

//! Touch moved
-(void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
	[self OnTouch:touches:te::input::TET_MOVE];
}

//! Dealloc
-(void)dealloc
{
    [super dealloc];
}

@end
