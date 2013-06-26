/*
 *  teMath.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 11/24/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEMATH_H
#define TE_TEMATH_H

#include "teTypes.h"
#include "teDebug.h"
#include <math.h>

namespace te
{
	const f32 tePi = 3.14159265359f;
	const f32 teHalfPi = tePi / 2.0f;
	const f32 teQuartfPi = tePi / 4.0f;

	const f64 tePi64 = 3.1415926535897932384626433832795028841971693993751;
	const f64 teHalfPi64 = tePi64 / 2.0;
	const f32 teQuartfPi64 = tePi64 / 4.0f;

	const f32 teDegToRad = tePi / 180.0f;
	const f32 teRadToDeg = 180.0f / tePi;
	
	const f32 teRoundingError32 = 0.000001f;
	
	TE_FUNC f32 teCos(f32 angle) {return cosf(angle);}
	TE_FUNC f32 teSin(f32 angle) {return sinf(angle);}
	TE_FUNC f32 teTan(f32 angle) {return tanf(angle);}
	TE_FUNC f32 teCot(f32 angle) {return 1.0f / teTan(angle);}
	
	TE_FUNC void teSinCos(f32 angle, f32 & sinValue, f32 & cosValue)
	{
		sinValue = teSin(angle);
		cosValue = teCos(angle);
	}
	
	TE_FUNC f32 teSqrt(f32 value) {return sqrtf(value);}
	TE_FUNC f32 teInvSqrt(f32 Value) {return 1.0f / sqrtf(Value);}

	template <class T>
	TE_INLINE T teAbs(T value) {return (value < 0) ? -value : value;}

	template <class T>
	TE_INLINE T teSqr(T Value) {return Value * Value;}

	TE_FUNC f32 teSign(const f32 & v) {return (v < 0.0f) ? -1.0f : ((v > 0.0f) ? 1.0f : 0.0f);}
	//TE_FUNC s32 teSign(const f32 & v) {return (v < 0) ? -1 : ((v > 0) ? 1 : 0);}

	template <class T>
	TE_INLINE const T & teMax(const T & a, const T & b) {return a < b ? b : a;}
	
	template <class T>
	TE_INLINE const T & teMax(const T & a, const T & b, const T & c) {return a < b ? teMax(b, c) : teMax(a, c);}
	
	template <class T>
	TE_INLINE const T & teMin(const T & a, const T & b) {return a < b ? a : b;}
	
	template <class T>
	TE_INLINE const T & teMin(const T & a, const T & b, const T & c) {return a < b ? teMin(a, c) : teMin(b, c);}
	
	// return value that >= min and <= max
	template <class T>
	TE_INLINE const T & teClamp(const T & v, const T & min, const T & max)
	{
		return teMin(teMax(v, min), max);
	}

	// move value to target with speed, snap to value whet distance < speed
	template <class T>
	TE_INLINE T teAnimate(const T & v, const T & target, const T & speed)
	{
		T d = target - v;
		T r = v;

		if(teAbs(d) > speed)
			r += teSign(d) * speed;
		else
			r = target;

		return r;
	}

	template <class T, class B>
	TE_INLINE T teLinearInterpolation(const T & a, const T & b, const B & param)
	{
		return a * (1.0f - param) + b * param;
	}

	TE_FUNC f32 teCeil(const f32 & v) {return ceilf(v);}
	TE_FUNC f32 teFloor(const f32 & v) {return floorf(v);}
	TE_FUNC f32 teFractional(const f32 & v) {return v - (s32)v;}

	TE_FUNC f32 teRound(const f32 & v)
	{
		#ifdef TE_PLATFORM_WIN
			return teFloor(v + 0.5f);
		#else
			return roundf(v);
		#endif
	}

	TE_FUNC f32 teTrunc(const f32 & v)
	{
		#ifdef TE_PLATFORM_WIN
			return (v < 0) ? -teFloor(-v) : teFloor(v);
		#else
			return truncf(v);
		#endif
	}

	//! Is Nan (it's IEEE standard)
	TE_FUNC u1 teIsNaN(const f32 & v) {return v != v;}

	#define TE_GET_BIT(__var, __num) (((__var) >> (__num))&1)
	#define TE_SET_BIT_1(__var, __num) (__var) |= 1 << (__num)
	#define TE_SET_BIT_0(__var, __num) (__var) &= ~( 1 << (__num))
	#define TE_SET_BIT(__var, __num, __val) if(__val){ (__var) = (__var) | (1 << (__num)); }else{ (__var) = (__var) & ~(1 << (__num));}

	#define TE_GET_FLAG(__var, __flag) ((__var) & (__flag))
	#define TE_SET_FLAG(__var, __flag, __val) {if(__val) (__var) |= (__flag); else (__var) &= ~(__flag);}

	#define TE_RAND (((f32)rand()) / ((f32)RAND_MAX))

	enum ERectangleOriginCorner
	{
		ROC_LEFT_UP,
		ROC_LEFT_MIDDLE,
		ROC_LEFT_DOWN,
		ROC_RIGHT_UP,
		ROC_RIGHT_MIDDLE,
		ROC_RIGHT_DOWN,
		ROC_TOP_MIDDLE,
		ROC_BOTTOM_MIDDLE,
		ROC_CENTER,

		ROC_TOP_LEFT = ROC_LEFT_UP,
		ROC_TOP_RIGHT = ROC_RIGHT_UP,
		ROC_BOTTOM_LEFT = ROC_LEFT_DOWN,
		ROC_BOTTOM_RIGHT = ROC_RIGHT_DOWN
	};

	TE_FUNC void GetRectangleProportion(ERectangleOriginCorner corner, f32 & x, f32 & y)
	{
		switch(corner)
		{
		case ROC_LEFT_UP:			x = 0.0f; y = 0.0f; break;
		case ROC_LEFT_MIDDLE:		x = 0.0f; y = 0.5f; break;
		case ROC_LEFT_DOWN:			x = 0.0f; y = 1.0f; break;
		case ROC_RIGHT_UP:			x = 1.0f; y = 0.0f; break;
		case ROC_RIGHT_MIDDLE:		x = 1.0f; y = 0.5f; break;
		case ROC_RIGHT_DOWN:		x = 1.0f; y = 1.0f; break;
		case ROC_TOP_MIDDLE:		x = 0.5f; y = 0.0f; break;
		case ROC_BOTTOM_MIDDLE:		x = 0.5f; y = 1.0f; break;
		case ROC_CENTER:			x = 0.5f; y = 0.5f; break;
		default: TE_ASSERT(0); break;
		}
	}
}

#endif
