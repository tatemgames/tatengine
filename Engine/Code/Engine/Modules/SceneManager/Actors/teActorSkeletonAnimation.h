/*
 *  teActorSkeletonAnimation.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 05/26/12.
 *  Copyright 2012 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEACTORSKELETONANIMATION_H
#define TE_TEACTORSKELETONANIMATION_H

#include "teActor.h"
#include "teFpsCounter.h"
#include "teFastScene.h"

namespace te
{
	namespace scene
	{
		class teActorSkeletonAnimation
		{
		public:
			TE_INLINE teActorSkeletonAnimation(teFastScene * setScene)
				:scene(setScene)
			{
				SetLayer(0.0f);
			}

			TE_INLINE ~teActorSkeletonAnimation()
			{
			}

			TE_INLINE void OnUpdate()
			{
				if((!surface) || (surface->surfaceIndex == u32Max))
					return;
				
				video::teSurfaceData * data = reinterpret_cast<video::teSurfaceData*>(scene->GetContentPack().surfaceData.At(surface->surfaceIndex));

				u32 skeletonIndex = data->skeletonIndex;

				if(skeletonIndex == u32Max)
					return;

				scene::teSkeleton * skeleton = reinterpret_cast<scene::teSkeleton*>(scene->GetContentPack().skeletonData.At(skeletonIndex));

				if(enableAnimation)
				{
					f64 fps = skeleton->frameDeltaTime * 1000.0;

					core::teTime2 time2;
					time2.FromSeconds(TE_TIME_32);

					core::teTime2 time3 = time2 - time;

					u32 fr_t = (u32)(time3.ToSeconds() / fps);
					//u32 fr_t = (u32)(TE_TIME_32 / fps);

					u32 fr = fr_t % (skeleton->GetFramesCount(layer) - 1);
					u32 fr2 = (fr_t + 1) % (skeleton->GetFramesCount(layer) - 1);
					u32 fr3 = (fr_t + 2) % (skeleton->GetFramesCount(layer) - 1);

					if(!fr3)
						OnLoop();

					surface->skeletonFrame[0] = fr;
					surface->skeletonFrame[1] = fr2;

					surface->skeletonLayer[0] = layer;
					surface->skeletonLayer[1] = layer;

					f32 delta = (f32)((time3.ToSeconds() - (f64)fr_t * fps) / fps);
					//f32 delta = (TE_TIME_32 - (f64)fr_t * fps) / fps;
					surface->skeletonDelta = teClamp(delta, 0.0f, 1.0f);
				}
				else
				{
					surface->skeletonFrame[0] = 0;
					surface->skeletonFrame[1] = 0;
					surface->skeletonLayer[0] = 0;
					surface->skeletonLayer[1] = 0;
					surface->skeletonDelta = 0.0f;
				}
			}

			TE_INLINE void SetLayer(f32 setLayer = 0.0f, f32 setFrame = 0.0f)
			{
				if((((u16)setLayer) != layer) || (!enableAnimation))
				{
					enableAnimation = true;
					
					layer = (u16)setLayer;
					frame = (u16)setFrame;

					time.FromSeconds(TE_TIME_32);
				}
			}

			TE_INLINE void Stop()
			{
				enableAnimation = false;
			}

			TE_INLINE u16 GetLayer()
			{
				return layer;
			}

			TE_INLINE u16 GetFrame()
			{
				return frame;
			}

			TE_ACTOR_SIGNAL(0, OnLoop);

		protected:
			teAssetSurface * surface;

			teFastScene * scene;
			core::teTime2 time;
			u16 layer;
			u16 frame;
			u1 enableAnimation;
		};

		TE_ACTOR_PROXY(teActorSkeletonAnimation);
		TE_ACTOR_SLOT_2(teActorSkeletonAnimation, SetLayer);
		TE_ACTOR_SLOT_0(teActorSkeletonAnimation, Stop);

		TE_FUNC void RegisterSkeletonAnimation(teActorsTI * ti)
		{
			ti->AddClass(sizeof(teActorSkeletonAnimation), "skeletonAnimation", TE_ACTOR_PROXY_NAMES(teActorSkeletonAnimation));
			ti->AddLink("surface");
			ti->AddSignal("OnLoop");
			ti->AddSlot("SetLayer", TE_ACTOR_SLOT_PROXY(teActorSkeletonAnimation, SetLayer));
			ti->AddSlot("Stop", TE_ACTOR_SLOT_PROXY(teActorSkeletonAnimation, Stop));
		}
	}
}

#endif