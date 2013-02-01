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
				layerOld = 0;
				frameOld = 0;
				blendTime = 0;
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

					u32 fro = fr_t % (skeleton->GetFramesCount(layerOld) - 1);
					u32 fro2 = (fr_t + 1) % (skeleton->GetFramesCount(layerOld) - 1);
					u32 fro3 = (fr_t + 2) % (skeleton->GetFramesCount(layerOld) - 1);

					if(!fr3)
						OnLoop();

					if(blendTime)
					{
						blendTime--;

						surface->blendPairs[0].SetXY(layerOld, fro);
						surface->blendPairs[1].SetXY(layerOld, fro2);
						surface->blendPairs[2].SetXY(layer, fr);
						surface->blendPairs[3].SetXY(layer, fr2);

						f32 delta1 = (f32)((time3.ToSeconds() - (f64)fr_t * fps) / fps);
						f32 delta2 = (((f32)blendTime) / 15.0f);
						surface->blendTimes[0] = teClamp(delta1, 0.0f, 1.0f);
						surface->blendTimes[1] = teClamp(delta1, 0.0f, 1.0f);
						surface->blendTimes[2] = teClamp(1.0f - delta2, 0.0f, 1.0f);
						surface->blendMode = 0x1 + 0x2 + 0x4 + 0x8;
					}
					else
					{
						surface->blendPairs[0].SetXY(layer, fr);
						surface->blendPairs[1].SetXY(layer, fr2);

						f32 delta = (f32)((time3.ToSeconds() - (f64)fr_t * fps) / fps);
						surface->blendTimes[0] = teClamp(delta, 0.0f, 1.0f);
						surface->blendMode = 0x1 + 0x2;
					}
				}
				else
				{
					surface->blendMode = 0x0;
				}
			}

			TE_INLINE void SetLayer(f32 setLayer = 0.0f, f32 setFrame = 0.0f, f32 enableBlend = 0.0f)
			{
				if((((u16)setLayer) != layer) || (!enableAnimation))
				{
					enableAnimation = true;

					if(((u8)enableBlend) == 1)
					{
						layerOld = layer;
						frameOld = frame;
						blendTime = 15;
					}
					else
					{
						layerOld = (u16)setLayer;
						frameOld = (u16)setFrame;
						blendTime = 0;
					}
					
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

			u16 layerOld, frameOld, blendTime;
			u1 enableAnimation;
		};

		TE_ACTOR_PROXY(teActorSkeletonAnimation);
		TE_ACTOR_SLOT_3(teActorSkeletonAnimation, SetLayer);
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