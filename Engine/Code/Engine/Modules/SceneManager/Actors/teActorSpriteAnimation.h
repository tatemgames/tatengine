/*
 *  teActorSpriteAnimation.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 02/8/12.
 *  Copyright 2011 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEACTORSPRITEANIMATION_H
#define TE_TEACTORSPRITEANIMATION_H

#include "teActor.h"
#include "teFpsCounter.h"

namespace te
{
	namespace scene
	{
		enum ESpriteAnimationFlags
		{
			SAF_LOOPED			= 0x1,
			SAF_AUTOSTART		= 0x2
		};

		class teActorSpriteAnimation
		{
		public:
			TE_INLINE teActorSpriteAnimation(teFastScene * setScene)
				:scene(setScene), enabled(false)
			{
				if(((u32)flags) & SAF_AUTOSTART)
					Start();
			}

			TE_INLINE ~teActorSpriteAnimation()
			{
			}

			TE_INLINE void OnUpdate()
			{
				if(enabled && frames && sprite)
				{
					core::teTime2 time = TE_TIME2;
					core::teTime2 delta = time - startTime;

					u32 frame = (u32)(delta.ToSeconds() * fps);
					SetFrame((f32)frame);

					if(frame >= frames->count)
					{
						if(((u32)flags) & SAF_LOOPED)
						{
							OnLoop();
							startTime = time;
						}
						else
						{
							enabled = false;
							OnStopped();
						}
					}
				}
			}

			TE_INLINE void SetFrames(teAssetArray * setFrames)
			{
				frames = setFrames;
			}

			TE_INLINE teAssetArray * GetFrames()
			{
				return frames;
			}

			TE_INLINE void Start(f32 force = 0.0f)
			{
				if((!enabled) || (force > 0.0f))
				{
					enabled = true;
					startTime = TE_TIME2;
				}
			}

			TE_INLINE void Stop()
			{
				enabled = false;
			}

			TE_INLINE void SetFrame(f32 frameIndex)
			{
				if(sprite && frames)
					sprite->renderAsset.materialIndex = (u32)scene->GetContentPack().materials.GetIndexInArray(reinterpret_cast<video::teMaterial**>(frames->data)[((u32)frameIndex) % frames->count]);
			}

			TE_ACTOR_SIGNAL(0, OnStopped);
			TE_ACTOR_SIGNAL(1, OnLoop);

			void OnReset() {}
		protected:
			f32 fps;
			f32 flags;

			teAssetSprite * sprite;
			teAssetArray * frames;

			teFastScene * scene;
			core::teTime2 startTime;
			u1 enabled;
		};

		TE_ACTOR_PROXY(teActorSpriteAnimation);
		TE_ACTOR_SLOT_1(teActorSpriteAnimation, Start);
		TE_ACTOR_SLOT_0(teActorSpriteAnimation, Stop);
		TE_ACTOR_SLOT_1(teActorSpriteAnimation, SetFrame);

		TE_FUNC void RegisterSpriteAnimation(teActorsTI * ti)
		{
			ti->AddClass(sizeof(teActorSpriteAnimation), "spriteAnimation", TE_ACTOR_PROXY_NAMES(teActorSpriteAnimation));
			ti->AddData("fps");
			ti->AddData("flags");
			ti->AddLink("sprite");
			ti->AddLink("frames");
			ti->AddSignal("OnStopped");
			ti->AddSignal("OnLoop");
			ti->AddSlot("Start", TE_ACTOR_SLOT_PROXY(teActorSpriteAnimation, Start));
			ti->AddSlot("Stop", TE_ACTOR_SLOT_PROXY(teActorSpriteAnimation, Stop));
			ti->AddSlot("SetFrame", TE_ACTOR_SLOT_PROXY(teActorSpriteAnimation, SetFrame));
		}
	}
}

#endif