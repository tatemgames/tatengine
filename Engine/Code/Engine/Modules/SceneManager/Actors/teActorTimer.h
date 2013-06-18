/*
 *  teActorTimer.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 12/11/11.
 *  Copyright 2011 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEACTORTIMER_H
#define TE_TEACTORTIMER_H

#include "teActor.h"
#include "teTimer.h"

namespace te
{
	namespace scene
	{
		class teActorTimer
		{
		public:
			TE_INLINE teActorTimer(teFastScene * setScene)
			{
			}

			TE_INLINE ~teActorTimer()
			{
			}

			TE_INLINE void OnUpdate()
			{
				if(timeToTick < 0.0f)
					return;

				core::teTime2 currentTime = core::GetTimer()->GetTime2();
				if((currentTime - lastTime).ToSeconds() > timeToTick)
				{
					OnTick();
					lastTime = currentTime;
				}
			}

			TE_INLINE void SetTime(f32 setTime)
			{
				timeToTick = setTime;
			}

			TE_INLINE void ForceTick()
			{
				OnTick();
			}

			TE_ACTOR_SIGNAL(0, OnTick)

		protected:
			f32 timeToTick;

			core::teTime2 lastTime;
		};

		TE_ACTOR_PROXY(teActorTimer)
		TE_ACTOR_SLOT_1(teActorTimer, SetTime)
		TE_ACTOR_SLOT_0(teActorTimer, ForceTick)

		TE_FUNC void RegisterTimer(teActorsTI * ti)
		{
			ti->AddClass(sizeof(teActorTimer), "timer", TE_ACTOR_PROXY_NAMES(teActorTimer));
			ti->AddData("timeToTick");
			ti->AddSignal("OnTick");
			ti->AddSlot("SetTime", TE_ACTOR_SLOT_PROXY(teActorTimer, SetTime));
			ti->AddSlot("ForceTick", TE_ACTOR_SLOT_PROXY(teActorTimer, ForceTick));
		}
	}
}

#endif
