/*
 *  teColor.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 12/1/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TECOLOR_H
#define TE_TECOLOR_H

#include "teTypes.h"
#include "teMath.h"

namespace te
{
	// Actually we support only two argb color space representaions :
	// float 32 bit with values space [0, 1]
	// unsigned char 8 bit with values space [0, 255]
	// (and others with values space [0, X] where X <= maximal value of data type)

	#define __CONV_TYPE f32
	#define __CONV_CH(__Ch) ((T)(((__CONV_TYPE)__Ch/(__CONV_TYPE)OtChMaxVal)*(__CONV_TYPE)(ChMaxValue)))

	template<typename T, s32 ChMaxValue>
	class teColor4
	{
	public:
		T r, g, b, a;

		TE_FORCE_INLINE teColor4():r(0), g(0), b(0), a((T)ChMaxValue) {}
		TE_FORCE_INLINE teColor4(T setR, T setG, T setB, T setA):r(setR), g(setG), b(setB), a(setA) {Clamp();}
		TE_FORCE_INLINE teColor4(T setR, T setG, T setB):r(setR), g(setG), b(setB), a((T)ChMaxValue) {Clamp();}
		TE_FORCE_INLINE teColor4(T k):r(k), g(k), b(k), a(k) {Clamp();}

		template <typename OtType, s32 OtChMaxVal>
		TE_INLINE teColor4(const teColor4<OtType, OtChMaxVal> & other):r(__CONV_CH(other.r)), g(__CONV_CH(other.g)), b(__CONV_CH(other.b)), a(__CONV_CH(other.a)) {}

		TE_INLINE void Flush() {r = 0; g = 0; b = 0; a = 0;}

		TE_INLINE void SetRGBA(T setR, T setG, T setB, T setA) {r = setR; g = setG; b = setB; a = setA;}
		TE_INLINE void SetRGB(T setR, T setG, T setB) {SetRGBA(setR, setG, setB, ChMaxValue);}

		TE_INLINE void Clamp(T min, T max) {r = teClamp(r, min, max); g = teClamp(g, min, max); b = teClamp(b, min, max); a = teClamp(a, min, max);}
		TE_INLINE void Clamp() {Clamp(0, ChMaxValue);}

		TE_INLINE u1 IsEqual(const teColor4<T, ChMaxValue> & other) const {return (r == other.r) && (g == other.g) && (b == other.b) && (a == other.a);}

		template <typename B, s32 OtChMaxVal>
		TE_INLINE teColor4<T, ChMaxValue> & operator = (const teColor4<B, OtChMaxVal> & other) {SetRGBA(__CONV_CH(other.r), __CONV_CH(other.g), __CONV_CH(other.b), __CONV_CH(other.a)); return *this;}
		TE_INLINE teColor4<T, ChMaxValue> & operator += (const teColor4<T, ChMaxValue> & other) {SetRGBA(r + other.r, g + other.g, b + other.b, a + other.a); return *this;}
		TE_INLINE teColor4<T, ChMaxValue> & operator -= (const teColor4<T, ChMaxValue> & other) {SetRGBA(r - other.r, g - other.g, b - other.b, a - other.a); return *this;}
		TE_INLINE teColor4<T, ChMaxValue> & operator *= (T other) {SetRGBA(r * other, g * other, b * other, a * other); return *this;}
		TE_INLINE teColor4<T, ChMaxValue> & operator /= (T other) {SetRGBA(r / other, g / other, b / other, a / other); return *this;}
		TE_INLINE teColor4<T, ChMaxValue> operator + (const teColor4<T, ChMaxValue> & other) const {return teColor4<T, ChMaxValue>(r + other.r, g + other.g, b + other.b, a + other.a);}
		TE_INLINE teColor4<T, ChMaxValue> operator - (const teColor4<T, ChMaxValue> & other) const {return teColor4<T, ChMaxValue>(r - other.r, g - other.g, b - other.b, a - other.a);}
		TE_INLINE teColor4<T, ChMaxValue> operator * (const teColor4<T, ChMaxValue> & other) const {return teColor4<T, ChMaxValue>(r * other.r, g * other.g, b * other.b, a * other.a);}
		TE_INLINE teColor4<T, ChMaxValue> operator / (const teColor4<T, ChMaxValue> & other) const {return teColor4<T, ChMaxValue>(r / other.r, g / other.g, b / other.b, a / other.a);}
		TE_INLINE teColor4<T, ChMaxValue> operator * (T value) const {return teColor4<T, ChMaxValue>(r * value, g * value, b * value, a * value);}
		TE_INLINE teColor4<T, ChMaxValue> operator / (T value) const {return teColor4<T, ChMaxValue>(r / value, g / value, b / value, a / value);}
		TE_INLINE u1 operator == (const teColor4<T, ChMaxValue> & other) const {return IsEqual(other);}
		TE_INLINE u1 operator != (const teColor4<T, ChMaxValue> & other) const {return !IsEqual(other);}

		teColor4<T, ChMaxValue> & SetFromHSL(T h, T s, T l) // TODO support conversion with max value
		{
			T x = (l < 0.5f) ? (l * (1.0f + s)) : ((l + s) - (s * l)), c = 2.0f * l - x;
			SetRGBA(GetChannelFromHue(c, x, h + 2.0f / 3.0f), GetChannelFromHue(c, x, h), GetChannelFromHue(c, x, h - 2.0f / 3.0f), ChMaxValue);
			return *this;
		}

	protected:
		T GetChannelFromHue(T c, T x, T h)
		{
			if(h < 0.0f) h += 1.0f;
			else if (h > 1.0f) h -= 1.0f;

			if(6.0f * h < 1.0f) return (c + (x - c) * 6.0f * h);
			if(2.0f * h < 1.0f) return x;
			if(3.0f * h < 2.0f) return (c + (x - c) * (2.0f / 3.0f - h) * 6.0f);
			return c;
		}
	};

	#undef __CONV_TYPE
	#undef __CONV_CH

	typedef teColor4<f32, 1> teColor4f; //! ARGB f32 Color, values [0, 1]
	typedef teColor4<u8, 255> teColor4u; //! ARGB u8 Color, values [0, 255]
}

#endif
