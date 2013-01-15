/*
 *  teFpsCounter.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 11/24/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEFPSCOUNTER_H
#define TE_TEFPSCOUNTER_H

#include "teTypes.h"
#include "teTimer.h"

namespace te
{
	namespace core
	{
		class teFpsCounter
		{
		public:
			teFpsCounter();
			~teFpsCounter();

			TE_INLINE f32 GetFps() {return 1.0f / (f32)frameTimes[0].ToSeconds();}
			TE_INLINE teTime2 GetFrameTime() {return frameTimes[0];} // frame time in seconds

			void SetDeltaTime(u1 enabled, teTime2 setDefaultTime = teTime2().FromSeconds(1.0f / 60.0f));
			TE_INLINE f32 GetDeltaTime() {return (f32)(deltaTimeEnabled ? filteredTime.ToSeconds() : defaultTime.ToSeconds());}

			void OnTick();

		protected:
			u1 deltaTimeEnabled;
			teTime2 defaultTime;
			teTime2 frameTimes[TE_FPS_COUNTER_FILTER_SIZE];
			teTime2 filteredTime;
			teTime2 lastTickTime;
		};
		
		teFpsCounter * GetFpsCounter();
	}
}

// delta timing helpers
#define TE_DT_64 te::core::GetFpsCounter()->GetDeltaTime()
#define TE_DT_32 ((f32)TE_DT_64)
#define TE_DT TE_DT_32
#define TE_DT2 core::teTime2().FromSeconds(TE_DT_32)

#endif
