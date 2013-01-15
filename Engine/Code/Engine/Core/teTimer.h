/*
 *  teTimer.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 11/24/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TETIMER_H
#define TE_TETIMER_H

#include "teTypes.h"

#ifdef TE_PLATFORM_WIN
#include <windows.h>
#endif

namespace te
{
	namespace core
	{
		//! Timer Base Type
		typedef f64 teTime;

		// ------------------------------------------------- wip
		#define TE_TIME2_SCALE 1000000

		class teTime2
		{
		public:
			s64 units; // time in units

			TE_INLINE teTime2(s64 setUnits = 0)
				:units(setUnits)
			{
			}

			TE_INLINE void Flush() {units = 0;}

			TE_INLINE teTime2 & FromSeconds(f32 seconds) {units = (s64)(seconds * (f64)TE_TIME2_SCALE); return *this;}
			TE_INLINE f64 ToSeconds() const {return ((f64)units) / ((f64)TE_TIME2_SCALE);}

			TE_INLINE teTime2 & FromMilliSeconds(f32 milliseconds) {units = (s64)(milliseconds * (f64)(TE_TIME2_SCALE / 1000)); return *this;}
			TE_INLINE f64 ToMilliSeconds() const {return ((f64)units) / ((f64)(TE_TIME2_SCALE / 1000));}

			TE_INLINE teTime2 & operator = (const teTime2 & other) {units = other.units; return *this;}

			TE_INLINE u1 operator == (const teTime2 & other) const {return units == other.units;}
			TE_INLINE u1 operator != (const teTime2 & other) const {return units != other.units;}
			TE_INLINE u1 operator > (const teTime2 & other) const {return units > other.units;}
			TE_INLINE u1 operator < (const teTime2 & other) const {return units < other.units;}
			TE_INLINE u1 operator >= (const teTime2 & other) const {return units >= other.units;}
			TE_INLINE u1 operator <= (const teTime2 & other) const {return units <= other.units;}

			TE_INLINE teTime2 & operator += (const teTime2 & other) {units += other.units; return *this;}
			TE_INLINE teTime2 & operator -= (const teTime2 & other) {units -= other.units; return *this;}
			TE_INLINE teTime2 & operator *= (const teTime2 & other) {units *= other.units; return *this;}
			TE_INLINE teTime2 & operator /= (const teTime2 & other) {units /= other.units; return *this;}

			TE_INLINE teTime2 operator + (const teTime2 & other) const {return teTime2(units + other.units);}
			TE_INLINE teTime2 operator - (const teTime2 & other) const {return teTime2(units - other.units);}
			TE_INLINE teTime2 operator * (const teTime2 & other) const {return teTime2(units * other.units);}
			TE_INLINE teTime2 operator / (const teTime2 & other) const {return teTime2(units / other.units);}
		};

		// ------------------------------------------------- wip
		
		//! Timer
		class teTimer
		{
		private:
			//! Current Time Correction
			teTime CurrentTimeCorrection;
			
			//! Current Run Time
			teTime CurrentRunTime;
			
			//! Start Global Time
			teTime2 StartGlobalTime;

			#ifdef TE_PLATFORM_WIN
				//! Current Start Tick Count
				LARGE_INTEGER CurrentStartTickCount;

				//! Current Frequency
				f64 CurrentFrequency;
			#endif
		public:
			//! Constructor
			teTimer();
			
			//! Destructor
			~teTimer();
			
			//! Get Time (in millisecs)
			teTime GetTime();
			
			teTime2 GetTime2()
			{
//				return teTime2().FromMilliSeconds(GetTime());
				return (GetTimeGlobal() - StartGlobalTime);
			}
			
			//! Set Time Correction
			void SetTimeCorrection(teTime Correction);

			//! Get Time Correction
			teTime GetTimeCorrection();

			teTime2 GetTimeGlobal();
		};

		//! Get Timer
		teTimer * GetTimer();

		#define TE_TIME te::core::GetTimer()->GetTime()
		#define TE_TIME2 te::core::GetTimer()->GetTime2()
		#define TE_TIME_32 (f32)te::core::GetTimer()->GetTime()

		#define TE_TIME_BEGIN(__var) te::core::teTime2 __var = TE_TIME2;
		#define TE_TIME_END(__var) __var = TE_TIME2 - __var;

		// return time limited to period, needed to workaround problem with float time precision
		#define TE_TIME_ANIM(__period_ms)   (te::core::teTime2(TE_TIME2.units % te::core::teTime2().FromMilliSeconds((f32)(__period_ms)).units).ToMilliSeconds())
		#define TE_TIME_ANIM_S(__period_s) (te::core::teTime2(TE_TIME2.units % te::core::teTime2().FromSeconds((f32)(__period_s)).units).ToSeconds())

		// return from 0 to 1 in period time
		#define TE_TIME_ANIM_T(__period_ms) ((f32)TE_TIME_ANIM(__period_ms) / ((f32)(__period_ms)))
		#define TE_TIME_ANIM_S_T(__period_s) ((f32)TE_TIME_ANIM_S(__period_s) / ((f32)(__period_s)))
	}
}

#endif
