/*
 *  teVector2D.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 11/23/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEVECTOR2D_H
#define TE_TEVECTOR2D_H

#include "teTypes.h"
#include "teMath.h"

namespace te
{
	template<typename T>
	struct teVector2d
	{
		T x, y;

		TE_FORCE_INLINE teVector2d():x(0), y(0) {}
		TE_FORCE_INLINE teVector2d(T setX, T setY):x(setX), y(setY) {}
		TE_FORCE_INLINE teVector2d(T k) :x(k), y(k) {}

		template <class B>
		TE_FORCE_INLINE teVector2d(const teVector2d<B> & other):x((T)other.x), y((T)other.y) {}

		TE_FORCE_INLINE void SetXY(T setX, T setY) {x = setX; y = setY;}
		TE_FORCE_INLINE void Flush() {SetXY(0, 0);}

		TE_INLINE u1 IsEqual(const teVector2d<T> & other) const {return (x == other.x) && (y == other.y);}

		TE_INLINE teVector2d<T> operator - () const {return teVector2d(-x, -y);}
		TE_INLINE teVector2d<T> & operator = (const teVector2d<T> & other) {SetXY(other.x, other.y); return *this;}

		TE_INLINE u1 operator == (const teVector2d<T> & other) const {return IsEqual(other);}
		TE_INLINE u1 operator != (const teVector2d<T> & other) const {return !IsEqual(other);}

		TE_INLINE teVector2d<T> & operator += (const teVector2d<T> & other) {x += other.x; y += other.y; return *this;}
		TE_INLINE teVector2d<T> & operator -= (const teVector2d<T> & other) {x -= other.x; y -= other.y; return *this;}
		TE_INLINE teVector2d<T> & operator *= (const teVector2d<T> & other) {x *= other.x; y *= other.y; return *this;}
		TE_INLINE teVector2d<T> & operator /= (const teVector2d<T> & other) {x /= other.x; y /= other.y; return *this;}
		TE_INLINE teVector2d<T> & operator *= (T value) {x *= value; y *= value; return *this;}
		TE_INLINE teVector2d<T> & operator /= (T value) {x /= value; y /= value; return *this;}

		TE_INLINE teVector2d<T> operator + (const teVector2d<T> & other) const {return teVector2d<T>(x + other.x, y + other.y);}
		TE_INLINE teVector2d<T> operator - (const teVector2d<T> & other) const {return teVector2d<T>(x - other.x, y - other.y);}
		TE_INLINE teVector2d<T> operator * (const teVector2d<T> & other) const {return teVector2d<T>(x * other.x, y * other.y);}
		TE_INLINE teVector2d<T> operator / (const teVector2d<T> & other) const {return teVector2d<T>(x / other.x, y / other.y);}
		TE_INLINE teVector2d<T> operator * (T value) const {return teVector2d<T>(x * value, y * value);}
		TE_INLINE teVector2d<T> operator / (T value) const {return teVector2d<T>(x / value, y / value);}

		TE_INLINE T DotProduct(const teVector2d<T> & other) const {return x * other.x + y * other.y;}

		TE_INLINE T GetDet(const teVector2d<T> & other) const {return x * other.y - y * other.x;}

		TE_INLINE f32 GetDistanceQR() const {return (f32)(x * x + y * y);}
		TE_INLINE f32 GetDistance() const {return teSqrt(GetDistanceQR());}

		TE_INLINE T GetMaxComponent() const {return teMax(x, y);}
		TE_INLINE T GetMinComponent() const {return teMin(x, y);}

		TE_INLINE teVector2d<T> & Normalize()
		{
			T distSqr = GetDistanceQR();
			if(distSqr == 0) return *this;
			else
			{
				(*this) *= (T)teInvSqrt(distSqr);
				return *this;
			}
		}
		
		TE_INLINE teVector2d<T> & RotateByAng(f32 t_ang)
		{
			T tmpX = this->x;
			this->x = this->x * cos(t_ang) + this->y * sin(t_ang);
			this->y = tmpX * cos(t_ang) - tmpX * sin(t_ang);
			return *this;
		}
		
		TE_INLINE f32 GetAngleRad(const teVector2d<T> & other) const
		{
			return acos(DotProduct(other)/(GetDistance() * other.GetDistance()));
		}
		
		TE_INLINE f32 GetAngleDeg(const teVector2d<T> & other) const
		{
			return GetAngleRad(other) * teRadToDeg;
		}

		TE_INLINE teVector2d<T> LinearInterpolation(teVector2d<T> b, f32 time) const {return (*this) * (1 - time) + b * time;}

		TE_INLINE teVector2d<T> GetXX() const {return teVector2d<T>(x, x);}
		TE_INLINE teVector2d<T> GetXY() const {return teVector2d<T>(x, y);}
		TE_INLINE teVector2d<T> GetYX() const {return teVector2d<T>(y, x);}
		TE_INLINE teVector2d<T> GetYY() const {return teVector2d<T>(y, y);}
	};

	typedef teVector2d<f32> teVector2df;
	typedef teVector2d<s32> teVector2di;

	typedef teVector2d<u32> teVector2du;
	typedef teVector2d<s32> teVector2ds;

	typedef teVector2d<u16> teVector2duh;
	typedef teVector2d<s16> teVector2dsh;
	
	typedef teVector2d<u8> teVector2duc;
	typedef teVector2d<s8> teVector2dsc;
};

#endif
