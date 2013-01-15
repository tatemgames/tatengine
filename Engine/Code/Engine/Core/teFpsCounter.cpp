/*
 *  teFpsCounter.cpp
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 11/24/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#include "teFpsCounter.h"

namespace te
{
	namespace core
	{
		teFpsCounter * currentFpsCounter = NULL;

		teFpsCounter::teFpsCounter()
		{
			SetDeltaTime(true);
			currentFpsCounter = this;
		}

		teFpsCounter::~teFpsCounter()
		{
			currentFpsCounter = NULL;
		}

		void teFpsCounter::SetDeltaTime(u1 enabled, teTime2 setDefaultTime)
		{
			deltaTimeEnabled = enabled;
			defaultTime = setDefaultTime;

			for(u8 i = 0; i < TE_FPS_COUNTER_FILTER_SIZE; ++i)
				frameTimes[i] = defaultTime;
			lastTickTime.FromSeconds(TE_TIME / 1000.0);
			filteredTime = defaultTime;
		}

		void teFpsCounter::OnTick()
		{
			for(u8 i = TE_FPS_COUNTER_FILTER_SIZE - 1; i > 0; --i)
				frameTimes[i] = frameTimes[i - 1];

			teTime2 currentTime = TE_TIME2;
			frameTimes[0] = currentTime - lastTickTime;
			if(frameTimes[0] > TE_TIME2_SCALE / 10)
				frameTimes[0] = TE_TIME2_SCALE / 10;
			lastTickTime = currentTime;
			
			filteredTime = 0.0f;
			for(u8 i = 0; i < TE_FPS_COUNTER_FILTER_SIZE; ++i)
				filteredTime += frameTimes[i];
			filteredTime /= (teTime2)TE_FPS_COUNTER_FILTER_SIZE;
		}

		teFpsCounter * GetFpsCounter()
		{
			return currentFpsCounter;
		}
	}
}
