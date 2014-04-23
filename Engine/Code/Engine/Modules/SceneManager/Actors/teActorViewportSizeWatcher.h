/*
 *  teActorViewportSizeWatcher.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 02/26/12.
 *  Copyright 2012 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEACTORVIEWPORTSIZEWATCHER_H
#define TE_TEACTORVIEWPORTSIZEWATCHER_H

#include "teActor.h"

namespace te
{
	namespace scene
	{
		// emit signal with args : width, height, x, y
		class teActorViewportSizeWatcher
		{
		public:
			TE_INLINE teActorViewportSizeWatcher(teFastScene * setScene)
				:animationState(0)
			{
			}

			TE_INLINE ~teActorViewportSizeWatcher()
			{
			}

			TE_INLINE void OnUpdate()
			{
				//if((viewportPosition != camera->viewportPosition) || (viewportSize != camera->viewportSize))
				{
					viewportPosition = camera->viewportPosition;
					viewportSize = camera->viewportSize;
					
					if(firstFrame)
					{
						if(viewportSize.x < viewportSize.y)
						{
							s32 temp = viewportSize.x;
							viewportSize.x = viewportSize.y;
							viewportSize.y = temp;
						}
						
						firstFrame = false;
					}

					teVector3df vs((f32)viewportSize.x, (f32)viewportSize.y, 0.0f);
					vs /= 2.0f;

					core::teTime2 animationTime = core::GetTimer()->GetTime2() - animationStart;
					f32 animationDelta = teClamp((f32)animationTime.ToSeconds() * 4.0f, 0.0f, 1.0f);
					f32 moveDistance = 300.0f;
					
					if(animationState == 1)
						animationDelta = teLinearInterpolation(moveDistance, 0.0f, animationDelta);
					else if(animationState == 2)
						animationDelta = teLinearInterpolation(0.0f, moveDistance, animationDelta);

					//if(animationState && (animationTime.ToSeconds() > 1.0f))
					//	animationState = 0;

					if(leftTop)		leftTop->position = vs *		teVector3df(-1.0f, +1.0f, 0.0f) + teVector3df(0.0f,  animationDelta, 0.0f);
					if(leftMiddle)	leftMiddle->position = vs *		teVector3df(-1.0f, +0.0f, 0.0f) + teVector3df(-animationDelta, 0.0f, 0.0f);
					if(leftBottom)	leftBottom->position = vs *		teVector3df(-1.0f, -1.0f, 0.0f) + teVector3df(0.0f, -animationDelta, 0.0f);

					if(rightTop)	rightTop->position = vs *		teVector3df(+1.0f, +1.0f, 0.0f) + teVector3df(0.0f,  animationDelta, 0.0f);
					if(rightMiddle)	rightMiddle->position = vs *	teVector3df(+1.0f, +0.0f, 0.0f) + teVector3df( animationDelta, 0.0f, 0.0f);
					if(rightBottom)	rightBottom->position = vs *	teVector3df(+1.0f, -1.0f, 0.0f) + teVector3df(0.0f, -animationDelta, 0.0f);

					if(topMiddle)	topMiddle->position = vs *		teVector3df(+0.0f, +1.0f, 0.0f) + teVector3df(0.0f,  animationDelta, 0.0f);
					if(bottomMiddle)bottomMiddle->position = vs *	teVector3df(+0.0f, -1.0f, 0.0f) + teVector3df(0.0f, -animationDelta, 0.0f);

					if(center)
					{
						center->scale.SetXYZ((f32)viewportSize.x / basicUIWidth, (f32)viewportSize.y / basicUIHeight, 1.0f);

						f32 t2 = teClamp((f32)animationTime.ToSeconds() * 4.0f, 0.0f, 1.0f);
						//if(animationState == 1)
						//	center->scale *= teLinearInterpolation(10.0f, 1.0f, t2);
						//else if(animationState == 2)
						//	center->scale *= teLinearInterpolation(1.0f, 10.0f, t2);
					}
										
//					if(viewportSize.x == 2048)
//						sizeRootScale.SetXY(1.0f, 1.0f);
//					else if(viewportSize.x == 1136)
//						sizeRootScale.SetXY(0.42f, 0.42f);
//					else if(viewportSize.x == 1024)
//						sizeRootScale.SetXY(0.5f, 0.5f);
//					else if(viewportSize.x == 960)
//						sizeRootScale.SetXY(0.42f, 0.42f);
//					else if(viewportSize.x == 480)
//						sizeRootScale.SetXY(0.25f, 0.25f);
//					else
						sizeRootScale.SetXY((f32)viewportSize.x / basicUIWidth, (f32)viewportSize.y / basicUIHeight);
					
					//sizeRootScale *= 2.0f;
					
					if(sizeRoot)
							sizeRoot->scale.SetXYZ(sizeRootScale.x, sizeRootScale.y, 1.0f);

					RequestChange();
				}
			}

			TE_INLINE void RequestChange()
			{
				OnChanged((f32)viewportSize.x, (f32)viewportSize.y, (f32)viewportPosition.x, (f32)viewportPosition.y);
			}

			TE_INLINE void ShowChilds()
			{
				animationState = 1;
				animationStart = core::GetTimer()->GetTime2();
			}

			TE_INLINE void HideChilds()
			{
				animationState = 2;
				animationStart = core::GetTimer()->GetTime2();
			}

			TE_ACTOR_SIGNAL(0, OnChanged)
			
			TE_INLINE static const teVector2df & GetSizeRootScale() {return sizeRootScale;}

			void OnReset() {}

		protected:
			f32 basicUIWidth;
			f32 basicUIHeight;

			teAssetCamera * camera;
			teAssetTransform * leftTop;
			teAssetTransform * leftMiddle;
			teAssetTransform * leftBottom;
			teAssetTransform * rightTop;
			teAssetTransform * rightMiddle;
			teAssetTransform * rightBottom;
			teAssetTransform * topMiddle;
			teAssetTransform * bottomMiddle;
			teAssetTransform * center;
			teAssetTransform * sizeRoot;
			teVector2di viewportPosition, viewportSize;
			core::teTime2 animationStart;
			u8 animationState;
			
			static teVector2df sizeRootScale;
			static u1 firstFrame;
		};

		TE_ACTOR_PROXY(teActorViewportSizeWatcher)
		TE_ACTOR_SLOT(teActorViewportSizeWatcher, RequestChange);
		TE_ACTOR_SLOT(teActorViewportSizeWatcher, ShowChilds);
		TE_ACTOR_SLOT(teActorViewportSizeWatcher, HideChilds);

		TE_FUNC void RegisterViewportSizeWatcher(teActorsTI * ti)
		{
			ti->AddClass(sizeof(teActorViewportSizeWatcher), "viewportSizeWatcher", TE_ACTOR_PROXY_NAMES(teActorViewportSizeWatcher));
			ti->AddData("basicUIWidth");
			ti->AddData("basicUIHeight");
			ti->AddLink("camera");
			ti->AddLink("leftTop");
			ti->AddLink("leftMiddle");
			ti->AddLink("leftBottom");
			ti->AddLink("rightTop");
			ti->AddLink("rightMiddle");
			ti->AddLink("rightBottom");
			ti->AddLink("topMiddle");
			ti->AddLink("bottomMiddle");
			ti->AddLink("center");
			ti->AddLink("sizeRoot");
			ti->AddSignal("OnChanged");
			ti->AddSlot("RequestChange", TE_ACTOR_SLOT_PROXY(teActorViewportSizeWatcher, RequestChange));
			ti->AddSlot("ShowChilds", TE_ACTOR_SLOT_PROXY(teActorViewportSizeWatcher, ShowChilds));
			ti->AddSlot("HideChilds", TE_ACTOR_SLOT_PROXY(teActorViewportSizeWatcher, HideChilds));
		}
	}
}

#endif