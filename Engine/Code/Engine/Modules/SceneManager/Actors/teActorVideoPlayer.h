/*
 *  teActorVideoPlayer.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 01/5/13.
 *  Copyright 2013 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEACTORVIDEOPLAYER_H
#define TE_TEACTORVIDEOPLAYER_H

#include "teActor.h"
#include "tePlatform.h"

#ifdef TE_PLATFORM_IPHONE
	#include "tePlatform_iOS.h"
#endif

namespace te
{
	namespace scene
	{
		class teActorVideoPlayer
		{
		public:
			TE_INLINE teActorVideoPlayer(teFastScene * setScene)
				:scene(setScene)
			{
			}

			TE_INLINE ~teActorVideoPlayer()
			{
			}

			TE_INLINE void OnUpdate()
			{
			}

			TE_INLINE void OpenVideo(f32 index)
			{
				#ifdef TE_PLATFORM_IPHONE
					static_cast<core::tePlatform_iOS*>(core::GetPlatform()->GetCurrentDevicePlatform())->PlayVideo((u8)index, (void*)this);
				#else
					OnFinished();
				#endif
			}

			TE_ACTOR_SIGNAL(0, OnFinished);

		protected:

			teFastScene * scene;
		};

		TE_ACTOR_PROXY(teActorVideoPlayer);
		TE_ACTOR_SLOT_1(teActorVideoPlayer, OpenVideo);

		TE_FUNC void RegisterVideoPlayer(teActorsTI * ti)
		{
			ti->AddClass(sizeof(teActorVideoPlayer), "videoPlayer", TE_ACTOR_PROXY_NAMES(teActorVideoPlayer));
			ti->AddSignal("OnFinished");
			ti->AddSlot("OpenVideo", TE_ACTOR_SLOT_PROXY(teActorVideoPlayer, OpenVideo));
		}
	}
}

#endif