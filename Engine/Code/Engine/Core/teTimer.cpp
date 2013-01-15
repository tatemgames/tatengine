/*
 *  teTimer.cpp
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 11/24/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#include "teTimer.h"
#include "teDebug.h"

#if defined(TE_PLATFORM_IPHONE) || defined(TE_PLATFORM_LINUX_MOBILE) || defined(TE_PLATFORM_ANDROID) || defined(TE_PLATFORM_MAC) || defined(TE_PLATFORM_LINUX)
#include <sys/time.h>
#endif

#ifdef TE_PLATFORM_WIN
#endif

namespace te
{
	namespace core
	{
		//! Current Timer
		teTimer * CurrentTimer = NULL;
		
		//! Constructor
		teTimer::teTimer()
			:CurrentTimeCorrection(0.0f),CurrentRunTime(0.0f),StartGlobalTime(0)
		{
			CurrentTimer = this;

			#ifdef TE_PLATFORM_WIN
				LARGE_INTEGER BaseFrequency;
				QueryPerformanceFrequency(&BaseFrequency);
				CurrentFrequency = BaseFrequency.QuadPart / 10000000.0;

				LARGE_INTEGER PerformanceTime;
				QueryPerformanceCounter(&PerformanceTime);
				CurrentStartTickCount = PerformanceTime;
			#endif
			
			CurrentRunTime = GetTime();
			
			StartGlobalTime = GetTimeGlobal();
		}
			
		//! Destructor
		teTimer::~teTimer()
		{
			CurrentTimer = NULL;
		}
		
		//! Get Time (in millisecs)
		teTime teTimer::GetTime()
		{
			#if defined(TE_PLATFORM_IPHONE) || defined(TE_PLATFORM_LINUX_MOBILE) || defined(TE_PLATFORM_ANDROID) || defined(TE_PLATFORM_MAC) || defined(TE_PLATFORM_LINUX)
				struct timeval CurrentTimeOfDay;
				gettimeofday(&CurrentTimeOfDay, NULL);
				
				f64 CurrentTime = (f64)CurrentTimeOfDay.tv_sec * 1000.0 + (f64)CurrentTimeOfDay.tv_usec * 0.001;
				return (teTime)CurrentTime + CurrentTimeCorrection - CurrentRunTime;
			#endif
			
			#ifdef TE_PLATFORM_WIN
				LARGE_INTEGER PerformanceTime;
				QueryPerformanceCounter(&PerformanceTime);
				return (PerformanceTime.QuadPart - CurrentStartTickCount.QuadPart) * 1000.0 / (CurrentFrequency * 10000000.0);
			#endif

			TE_ASSERT_NODEBUG(0);
			return 0;
		}

		//! Set Time Correction
		void teTimer::SetTimeCorrection(teTime Correction)
		{
			CurrentTimeCorrection = Correction;
		}

		//! Get Time Correction
		teTime teTimer::GetTimeCorrection()
		{
			return CurrentTimeCorrection;
		}

		teTime2 teTimer::GetTimeGlobal()
		{
			#if defined(TE_PLATFORM_IPHONE) || defined(TE_PLATFORM_LINUX_MOBILE) || defined(TE_PLATFORM_ANDROID) || defined(TE_PLATFORM_MAC) || defined(TE_PLATFORM_LINUX)
				struct timeval timeOfDay;
				gettimeofday(&timeOfDay, NULL);

				s64 units = (s64)timeOfDay.tv_sec * TE_TIME2_SCALE + (s64)timeOfDay.tv_usec * (TE_TIME2_SCALE / 1000000);
				return teTime2(units);
			#endif

			#ifdef TE_PLATFORM_WIN
				const s64 DELTA_EPOCH_IN_MICROSECS = 11644473600000000;

				FILETIME ft;
				s64 tmpres = 0;
				TIME_ZONE_INFORMATION tzWinapi;
				int rez = 0;

				memset(&ft, 0, sizeof(ft));
				memset(&tzWinapi, 0, sizeof(tzWinapi));

				GetSystemTimeAsFileTime(&ft);

				tmpres = ft.dwHighDateTime;
				tmpres <<= 32;
				tmpres |= ft.dwLowDateTime;

				// converting file time to unix epoch
				tmpres /= 10; // convert into microseconds
				tmpres -= DELTA_EPOCH_IN_MICROSECS;

				return teTime2(tmpres);
			#endif
		}

		//! Get Timer
		teTimer * GetTimer()
		{
			return CurrentTimer;
		}
	}
}
