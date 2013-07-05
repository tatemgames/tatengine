/*
 *  teVector3D.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 11/23/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEVECTOR3D_H
#define TE_TEVECTOR3D_H

#include "teTypes.h"
#include "teMath.h"
#include "teVector2d.h"

namespace te
{
	template<typename T>
	class teVector3d
	{
	public:
		T x, y, z;
		
		TE_FORCE_INLINE teVector3d():x(0), y(0), z(0) {}
		TE_FORCE_INLINE teVector3d(T setX, T setY, T setZ):x(setX), y(setY), z(setZ) {}
		TE_FORCE_INLINE teVector3d(T k):x(k), y(k), z(k) {}
		
		template <class B>
		TE_FORCE_INLINE teVector3d(const teVector3d<B> & other):x((T)other.x), y((T)other.y), z((T)other.z) {}

		template <class B>
		TE_FORCE_INLINE teVector3d(const teVector2d<B> & other):x((T)other.x), y((T)other.y), z(0) {}

		TE_INLINE void SetXYZ(T setX, T setY, T setZ) {x = setX; y = setY; z = setZ;}
		TE_INLINE void Flush() {SetXYZ(0, 0, 0);}

		TE_INLINE u1 IsEqual(const teVector3d<T> & other) const {return (x == other.x) && (y == other.y) && (z == other.z);}

		TE_INLINE teVector3d<T> operator - () const {return teVector3d(-x, -y, -z);}
		TE_INLINE teVector3d<T> & operator = (const teVector3d<T> & other) {SetXYZ(other.x, other.y, other.z); return *this;}

		TE_INLINE u1 operator == (const teVector3d<T> & other) const {return IsEqual(other);}
		TE_INLINE u1 operator != (const teVector3d<T> & other) const {return !IsEqual(other);}

		TE_INLINE teVector3d<T> & operator += (const teVector3d<T> & other) {x += other.x; y += other.y; z += other.z; return *this;}
		TE_INLINE teVector3d<T> & operator -= (const teVector3d<T> & other) {x -= other.x; y -= other.y; z -= other.z; return *this;}
		TE_INLINE teVector3d<T> & operator *= (const teVector3d<T> & other) {x *= other.x; y *= other.y; z *= other.z; return *this;}
		TE_INLINE teVector3d<T> & operator /= (const teVector3d<T> & other) {x /= other.x; y /= other.y; z /= other.z; return *this;}
		TE_INLINE teVector3d<T> & operator *= (T value) {x *= value; y *= value; z *= value; return *this;}
		TE_INLINE teVector3d<T> & operator /= (T value) {x /= value; y /= value; z /= value; return *this;}

		TE_INLINE teVector3d<T> operator + (const teVector3d<T> & other) const {return teVector3d<T>(x + other.x, y + other.y, z + other.z);}
		TE_INLINE teVector3d<T> operator - (const teVector3d<T> & other) const {return teVector3d<T>(x - other.x, y - other.y, z - other.z);}
		TE_INLINE teVector3d<T> operator * (const teVector3d<T> & other) const {return teVector3d<T>(x * other.x, y * other.y, z * other.z);}
		TE_INLINE teVector3d<T> operator / (const teVector3d<T> & other) const {return teVector3d<T>(x / other.x, y / other.y, z / other.z);}
		TE_INLINE teVector3d<T> operator * (T value) const {return teVector3d<T>(x * value, y * value, z * value);}
		TE_INLINE teVector3d<T> operator / (T value) const {return teVector3d<T>(x / value, y / value, z / value);}

		TE_INLINE T DotProduct(const teVector3d<T> & other) const {return x * other.x + y * other.y + z * other.z;}
		
		TE_INLINE teVector3d<T> CrossProduct(const teVector3d<T> & other) const {return teVector3d<T>(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x);}
		
		TE_INLINE f32 GetDistanceQR() const {return x * x + y * y + z * z;}
		TE_INLINE f32 GetDistance() const {return teSqrt(GetDistanceQR());}

		TE_INLINE T GetMaxComponent() const {return teMax(x, y, z);}
		TE_INLINE T GetMinComponent() const {return teMin(x, y, z);}
		
		TE_INLINE teVector3d<T> & Normalize()
		{
			T distSqr = GetDistanceQR();
			if(distSqr == 0) return *this;
			else
			{
				(*this) *= (T)teInvSqrt(distSqr);
				return *this;
			}
		}
		
		TE_INLINE f32 GetAngleRad(const teVector3d<T> & other) const
		{
			return acos(DotProduct(other)/(GetDistance() * other.GetDistance()));
		}
		
		TE_INLINE f32 GetAngleDeg(const teVector3d<T> & other) const
		{
			return GetAngleRad(other) * teRadToDeg;
		}

		TE_INLINE teVector3d<T> LinearInterpolation(teVector3d<T> b, f32 time) const {return (*this) * (1 - time) + b * time;}

		TE_INLINE teVector2d<T> GetXX() const {return teVector2d<T>(x, x);}
		TE_INLINE teVector2d<T> GetXY() const {return teVector2d<T>(x, y);}
		TE_INLINE teVector2d<T> GetXZ() const {return teVector2d<T>(x, z);}
		TE_INLINE teVector2d<T> GetYX() const {return teVector2d<T>(y, x);}
		TE_INLINE teVector2d<T> GetYY() const {return teVector2d<T>(y, y);}
		TE_INLINE teVector2d<T> GetYZ() const {return teVector2d<T>(y, z);}
		TE_INLINE teVector2d<T> GetZX() const {return teVector2d<T>(z, x);}
		TE_INLINE teVector2d<T> GetZY() const {return teVector2d<T>(z, y);}
		TE_INLINE teVector2d<T> GetZZ() const {return teVector2d<T>(z, z);}
	};
	
	typedef teVector3d<f32> teVector3df;
	typedef teVector3d<s32> teVector3di;

	typedef teVector3d<u32> teVector3du;
	typedef teVector3d<s32> teVector3ds;

	typedef teVector3d<u16> teVector3duh;
	typedef teVector3d<s16> teVector3dsh;

	typedef teVector3d<u8> teVector3duc;
	typedef teVector3d<s8> teVector3dsc;
};

#endif
