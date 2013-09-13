/*
 *  teQuaternion.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 11/24/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEQUATERNION_H
#define TE_TEQUATERNION_H

#include "teTypes.h"
#include "teVector3d.h"
#include "teMatrix4.h"
#include "teMath.h"

namespace te
{
	template<typename T>
	class teQuaternion
	{
	public:
		T x, y, z, w;

		TE_FORCE_INLINE teQuaternion():x(0), y(0), z(0), w(1) {}
		TE_FORCE_INLINE teQuaternion(T setX, T setY, T setZ, T setW):x(setX), y(setY), z(setZ), w(setW) {}
		TE_FORCE_INLINE teQuaternion(T k):x(k), y(k), z(k), w(k) {}

		template <class B>
		TE_FORCE_INLINE teQuaternion(const teQuaternion<B> & other):x((T)other.x), y((T)other.y), z((T)other.z), w((T)other.w) {}

		template <class B>
		TE_FORCE_INLINE teQuaternion(const teVector3d<B> & other):x((T)other.x), y((T)other.y), z((T)other.z), w(1) {}

		template <class B>
		TE_FORCE_INLINE teQuaternion(B angle, const teVector3d<B> & axis)
		{
			T valueSin, valueCos;
			te::teSinCos(angle * 0.5f, valueSin, valueCos);
			SetXYZW(axis.x * valueSin, axis.y * valueSin, axis.z * valueSin, valueCos);
		}

		TE_INLINE void SetXYZW(T setX, T setY, T setZ, T setW) {x = setX; y = setY; z = setZ; w = setW;}
		TE_INLINE void Flush() {SetXYZW(0, 0, 0, 1);}

		TE_INLINE u1 IsEqual(const teVector3d<T> & other) const {return (x == other.x) && (y == other.y) && (z == other.z) && (w == other.w);}

		TE_INLINE teQuaternion<T> operator - () const {return teQuaternion(-x, -y, -z, -w);}
		TE_INLINE teQuaternion<T> & operator = (const teQuaternion<T> & other) {SetXYZW(other.x, other.y, other.z, other.w); return *this;}

		TE_INLINE u1 operator == (const teQuaternion<T> & other) const {return IsEqual(other);}
		TE_INLINE u1 operator != (const teQuaternion<T> & other) const {return !IsEqual(other);}

		TE_INLINE teQuaternion<T> & operator += (const teQuaternion<T> & other) {x += other.x; y += other.y; z += other.z; w += other.w; return *this;}
		TE_INLINE teQuaternion<T> & operator -= (const teQuaternion<T> & other) {x -= other.x; y -= other.y; z -= other.z; w -= other.w; return *this;}
		TE_INLINE teQuaternion<T> & operator *= (T value) {x *= value; y *= value; z *= value; w *= value; return *this;}
		TE_INLINE teQuaternion<T> & operator /= (T value) {x /= value; y /= value; z /= value; w /= value; return *this;}

		TE_INLINE teQuaternion<T> operator + (const teQuaternion<T> & other) const {return teQuaternion<T>(x + other.x, y + other.y, z + other.z, w + other.w);}
		TE_INLINE teQuaternion<T> operator - (const teQuaternion<T> & other) const {return teQuaternion<T>(x - other.x, y - other.y, z - other.z, w - other.w);}
		TE_INLINE teQuaternion<T> operator * (T value) const {return teQuaternion<T>(x * value, y * value, z * value, w * value);}
		TE_INLINE teQuaternion<T> operator / (T value) const {return teQuaternion<T>(x / value, y / value, z / value, w / value);}

		TE_INLINE teQuaternion<T> & operator *= (const teQuaternion<T> & other)
		{
			SetXYZW(
				(other.w * x) + (other.x * w) + (other.y * z) - (other.z * y),
				(other.w * y) + (other.y * w) + (other.z * x) - (other.x * z),
				(other.w * z) + (other.z * w) + (other.x * y) - (other.y * x),
				(other.w * w) - (other.x * x) - (other.y * y) - (other.z * z));
			return *this;
		}

		TE_INLINE teQuaternion<T> operator * (const teQuaternion<T> & other) const
		{
			return teQuaternion<T>(
				(other.w * x) + (other.x * w) + (other.y * z) - (other.z * y),
				(other.w * y) + (other.y * w) + (other.z * x) - (other.x * z),
				(other.w * z) + (other.z * w) + (other.x * y) - (other.y * x),
				(other.w * w) - (other.x * x) - (other.y * y) - (other.z * z));
		}

		TE_INLINE teQuaternion<T> GetInverse() const {return teQuaternion(-x, -y, -z, w);}

		TE_INLINE void Normalize() {T d = teInvSqrt(x * x + y * y + z * z + w * w); SetXYZW(x * d, y * d, z * d, w * d);}

		TE_INLINE T GetLength() {T d = teSqrt(x * x + y * y + z * z + w * w); return d;} // VADIM

		TE_INLINE void Rotate(teVector3d<T> & vector) const {vector = ((*this) * teQuaternion(vector) * GetInverse()).GetXYZ();}

//		void SetFromMatrix(const teMatrix4<T> & matrix)
//		{
//			// Algorithm in Ken Shoemake's article in 1987 SIGGRAPH course notes
//			// article "Quaternion Calculus and Fast Animation".
//
//			T trace = matrix.Get(0, 0) + matrix.Get(1, 1) + matrix.Get(2, 2);
//
//			if(trace > 0.0f)
//			{
//				T root = teSqrt(trace + 1.0f);
//				w = 0.5f * root;
//				root = 0.5f / root;
//				x = (matrix.Get(2, 1) - matrix.Get(1, 2)) * root;
//				y = (matrix.Get(0, 2) - matrix.Get(2, 0)) * root;
//				z = (matrix.Get(1, 0) - matrix.Get(0, 1)) * root;
//			}
//			else
//			{
//				u8 i = 0;
//				if(matrix.Get(1, 1) > matrix.Get(0, 0))
//					i = 1;
//				if(matrix.Get(2, 2) > matrix.Get(i, i))
//					i = 2;
//				u8 nextI[3] = {1, 2, 0};
//				u8 j = nextI[i];
//				u8 k = nextI[k];
//
//				T root = teSqrt(matrix.Get(i, i) - matrix.Get(j, j) - matrix.Get(k, k) + 1.0f);
//				T * quat[3] = {&x, &y, &z};
//				*quat[i] = 0.5f * root;
//				root = 0.5f / root;
//				w = (matrix.Get(k, j) - matrix.Get(j, k)) * root;
//				*quat[j] = (matrix.Get(j, i) + matrix.Get(i, j)) * root;
//				*quat[k] = (matrix.Get(k, i) + matrix.Get(i, k)) * root;
//			}
//		}

		TE_INLINE void SetFromMatrix(const teMatrix4<T> & mtr)
		{
			const T t = mtr.Get(0, 0) + mtr.Get(1, 1) + mtr.Get(2, 2);
				  T s;

			if(t > 0.0f)
			{
				s = 0.5f / sqrtf(t + 1.0f);
				x = (mtr.Get(1, 2) - mtr.Get(2, 1)) * s;
				y = (mtr.Get(2, 0) - mtr.Get(0, 2)) * s;
				z = (mtr.Get(0, 1) - mtr.Get(1, 0)) * s;
				w = 0.25f / s;
			}
			else if(mtr.Get(0, 0) > mtr.Get(1, 1) && mtr.Get(0, 0) > mtr.Get(2, 2))
			{
				s = 2.0f * sqrtf(1.0f + mtr.Get(0, 0) - mtr.Get(1, 1) - mtr.Get(2, 2));
				x = 0.25f * s;
				y = (mtr.Get(1, 0) + mtr.Get(0, 1)) / s;
				z = (mtr.Get(2, 0) + mtr.Get(0, 2)) / s;
				w = (mtr.Get(2, 1) - mtr.Get(1, 2)) / s;
			}
			else if(mtr.Get(1, 1) > mtr.Get(2, 2))
			{
				s = 2.0f * sqrtf(1.0f + mtr.Get(1, 1) - mtr.Get(0, 0) - mtr.Get(2, 2));
				x = (mtr.Get(1, 0) + mtr.Get(0, 1)) / s;
				y = 0.25f * s;
				z = (mtr.Get(2, 1) + mtr.Get(1, 2)) / s;
				w = (mtr.Get(2, 0) - mtr.Get(0, 2)) / s;
			}
			else
			{
				s = 2.0f * sqrtf(1.0f + mtr.Get(2, 2) - mtr.Get(0, 0) - mtr.Get(1, 1));
				x = (mtr.Get(2, 0) + mtr.Get(0, 2)) / s;
				y = (mtr.Get(2, 1) + mtr.Get(1, 2)) / s;
				z = 0.25f * s;
				w = (mtr.Get(1, 0) - mtr.Get(0, 1)) / s;
			}
		}

		TE_INLINE void GetMatrix(teMatrix4<T> & result) const
		{
			// left handed, column major (?), ccw rotation matrix
			result.SetIdentity();

			result.Get(0, 0) = 1.0f - 2.0f * (y * y + z * z);
			result.Get(0, 1) = 2.0f * (x * y + z * w);
			result.Get(0, 2) = 2.0f * (x * z - y * w);
			result.Get(1, 0) = 2.0f * (x * y - z * w);
			result.Get(1, 1) = 1.0f - 2.0f * (x * x + z * z);
			result.Get(1, 2) = 2.0f * (z * y + x * w);
			result.Get(2, 0) = 2.0f * (x * z + y * w);
			result.Get(2, 1) = 2.0f * (z * y - x * w);
			result.Get(2, 2) = 1.0f - 2.0f * (x * x + y * y);
		}

		TE_INLINE f32 DotProduct(const teQuaternion<T> & other) const {return x * other.x + y * other.y + z * other.z + w * other.w;}

		TE_INLINE teQuaternion<T> Slerp(const teQuaternion<T> & q, T time) const
		{
			teQuaternion<T> q1 = *this;
			teQuaternion<T> q2 = q;

			f32 angle = q1.DotProduct(q2);

			if(angle < 0.0f)
			{
				q1 = q1 * (-1.0f);
				angle = -angle;
			}

			f32 scale, invScale;

			const f32 evp = 0.05f;

			if((angle + 1.0f) > evp)
			{
				if((1.0f - angle) >= evp)
				{
					f32 th = acosf(angle);
					f32 invSinTh = 1.0f / teSin(th);
					scale = teSin(th * (1.0f - time)) * invSinTh;
					invScale = teSin(th * time) * invSinTh;
				}
				else
				{
					scale = 1.0f - time;
					invScale = time;
				}
			}
			else
			{
				q2.SetXYZW(-q1.y, q1.x, -q1.w, q1.z);
				scale = teSin(tePi * (0.5f - time));
				invScale = teSin(tePi * time);
			}

			return q1 * scale + q2 * invScale;
		}

		TE_INLINE void SetFromEulerAngles(const teVector3d<T> & angles)
		{
			f32 sr, cr, sp, cp, sy, cy;

			teSinCos(-angles.x * teDegToRad / 2.0f, sr, cr); // invert angles
			teSinCos(-angles.y * teDegToRad / 2.0f, sp, cp);
			teSinCos(-angles.z * teDegToRad / 2.0f, sy, cy);

			f32 cpcy = cp * cy;
			f32 spcy = sp * cy;
			f32 cpsy = cp * sy;
			f32 spsy = sp * sy;

			x = sr * cpcy - cr * spsy;
			y = cr * spcy + sr * cpsy;
			z = cr * cpsy - sr * spcy;
			w = cr * cpcy + sr * spsy;

			Normalize();
		}

		TE_INLINE teVector3d<T> GetEulerAngles() const
		{
			return teVector3d<T>(
				atan2(2.0f * (y * z + x * w), -x * x - y * y + z * z + w * w) * (-teRadToDeg),
				atan2(2.0f * (x * y + z * w),  x * x - y * y - z * z + w * w) * (-teRadToDeg),
				asinf(teClamp(-2.0f * (x * z - y * w), -1.0f, 1.0f) * (-teRadToDeg)));
		}

		TE_INLINE teVector3d<T> GetXYZ() const {return teVector3d<T>(x, y, z);}
	};

	//! f32 quaternion
	typedef teQuaternion<f32> teQuaternionf;

	class teDualQuaternion
	{
	public:
		teQuaternionf a, b;

		TE_FORCE_INLINE teDualQuaternion()
		{
		}

		TE_FORCE_INLINE teDualQuaternion(const teQuaternionf & setA, const teQuaternionf & setB)
			:a(setA), b(setB)
		{
		}

		TE_FORCE_INLINE teDualQuaternion(const teQuaternionf & rotate, const teVector3df & translate)
		{
			SetFrom(rotate, translate);
		}

		TE_FORCE_INLINE teDualQuaternion(const teDualQuaternion & other)
			:a(other.a), b(other.b)
		{
		}

		TE_INLINE teDualQuaternion & SetFrom(const teQuaternionf & rotate, const teVector3df & translate)
		{
			a = rotate;
			b.x = 0.5f * ( translate.x * rotate.w + translate.y * rotate.z - translate.z * rotate.y);
			b.y = 0.5f * (-translate.x * rotate.z + translate.y * rotate.w + translate.z * rotate.x);
			b.z = 0.5f * ( translate.x * rotate.y - translate.y * rotate.x + translate.z * rotate.w);
			b.w =-0.5f * ( translate.x * rotate.x + translate.y * rotate.y + translate.z * rotate.z);
			return *this;
		}

		TE_INLINE void MultiplyBy(const teDualQuaternion & other)
		{
			b = a * other.b + b * other.a;
			a = a * other.a;
		}

		TE_INLINE void AddBy(const teDualQuaternion & other)
		{
			a += other.a;
			b += other.b;
		}

		TE_INLINE void MultiplyBy(const f32 k)
		{
			a *= k;
			b *= k;
		}

		TE_INLINE void MakePositive()
		{
			if(a.w < 0.0f)
			{
				a.SetXYZW(-a.x, -a.y, -a.z, -a.w);
				b.SetXYZW(-b.x, -b.y, -b.z, -b.w);
			}
		}
		
		TE_INLINE void Normalize()
		{
			f32 d = a.GetLength();
			
			if(d > teRoundingError32)
			{
				a = a / d;
				b = b / d;
			
				b = b - a * a.DotProduct(b);
			}
		}

//		TE_INLINE void GetMatrix(teMatrix4f & result) const
//		{
//			result.Set(0, 0, a.w*a.w + a.x*a.x - a.y*a.y - a.z*a.z);
//			result.Set(0, 1, 2*a.x*a.y - 2*a.w*a.z);
//			result.Set(0, 2, 2*a.x*a.z + 2*a.w*a.y);
//			result.Set(0, 3, -2*b.w*a.x + 2*b.x*a.w - 2*b.y*a.z + 2*b.z*a.y);
//			result.Set(1, 0, 2*a.x*a.y + 2*a.w*a.z);
//			result.Set(1, 1, a.w*a.w + a.y*a.y - a.x*a.x - a.z*a.z);
//			result.Set(1, 2, 2*a.y*a.z - 2*a.w*a.x);
//			result.Set(1, 3, -2*b.w*a.y + 2*b.x*a.z + 2*b.y*a.w - 2*b.z*a.x);
//			result.Set(2, 0, 2*a.x*a.z - 2*a.w*a.y);
//			result.Set(2, 1, 2*a.y*a.z + 2*a.w*a.x);
//			result.Set(2, 2, a.w*a.w + a.z*a.z - a.x*a.x - a.y*a.y);
//			result.Set(2, 3, -2*b.w*a.z - 2*b.x*a.y + 2*b.y*a.x + 2*b.z*a.w);
//			result.Set(3, 0, 0.0f);
//			result.Set(3, 1, 0.0f);
//			result.Set(3, 2, 0.0f);
//			result.Set(3, 3, 1.0f);
//
//			result = result * (1.0f / a.DotProduct(a));
//		}
	};
}

#endif
