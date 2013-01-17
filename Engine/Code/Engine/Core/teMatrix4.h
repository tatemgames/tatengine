/*
 *  teMatrix4.h
 *  TatEngine
 *
 *  Created by Vadim Luchko on 03/12/12.
 *  Copyright 2012 Tatem Games. All rights reserved.
 *
 */
#ifndef TE_TEMATRIX4_H
#define TE_TEMATRIX4_H

#include "teTypes.h"
#include "teMath.h"
#include "teVector2d.h"
#include "teVector3d.h"
#include "teMemoryManager.h"

#define TE_MATRIX4X4_ORDER_RULE(i, j) i + 4 * j

namespace te
{
	template <typename T>
	class teMatrix4
	{
	public:
		T m[16];

		teMatrix4<T>()
		{
		}

		teMatrix4<T>(const teMatrix4 & setM) {memcpy(m, setM.m, sizeof(T) * 16);}

		TE_INLINE teMatrix4(T m00, T m01, T m02, T m03,
							T m10, T m11, T m12, T m13,
							T m20, T m21, T m22, T m23,
							T m30, T m31, T m32, T m33)
		{
			Set(m00, m01, m02, m03,
				m10, m11, m12, m13,
				m20, m21, m22, m23,
				m30, m31, m32, m33);
		}

		TE_FORCE_INLINE void Set(T m00, T m01, T m02, T m03,
						   T m10, T m11, T m12, T m13,
						   T m20, T m21, T m22, T m23,
						   T m30, T m31, T m32, T m33)
		{
			Get(0, 0) = m00;
			Get(0, 1) = m01;
			Get(0, 2) = m02;
			Get(0, 3) = m03;

			Get(1, 0) = m10;
			Get(1, 1) = m11;
			Get(1, 2) = m12;
			Get(1, 3) = m13;

			Get(2, 0) = m20;
			Get(2, 1) = m21;
			Get(2, 2) = m22;
			Get(2, 3) = m23;

			Get(3, 0) = m30;
			Get(3, 1) = m31;
			Get(3, 2) = m32;
			Get(3, 3) = m33;
		}

		TE_FORCE_INLINE void Set(const teMatrix4 & setM)
		{
			memcpy(m, setM.m, sizeof(T) * 16);
		}

		TE_FORCE_INLINE T & Get(u8 i, u8 j) {return m[TE_MATRIX4X4_ORDER_RULE(i, j)];}

		TE_FORCE_INLINE const T & Get(u8 i, u8 j) const {return m[TE_MATRIX4X4_ORDER_RULE(i, j)];}

		TE_INLINE teMatrix4<T> & SetIdentity()
		{
			Set(1, 0, 0, 0,
				0, 1, 0, 0,
				0, 0, 1, 0,
				0, 0, 0, 1);

			return *this;
		}

		TE_INLINE teMatrix4<T> & SetZero()
		{
			Set(0, 0, 0, 0,
				0, 0, 0, 0,
				0, 0, 0, 0,
				0, 0, 0, 0);

			return *this;
		}

		TE_INLINE teMatrix4<T> & operator = (const teMatrix4 & setM) {Set(setM); return *this;}

		TE_INLINE const teMatrix4<T> operator + (const teMatrix4 & mtr) const
		{
			return teMatrix4(Get(0, 0) + mtr.Get(0, 0), Get(0, 1) + mtr.Get(0, 1), Get(0, 2) + mtr.Get(0, 2), Get(0, 3) + mtr.Get(0, 3),
							 Get(1, 0) + mtr.Get(1, 0), Get(1, 1) + mtr.Get(1, 1), Get(1, 2) + mtr.Get(1, 2), Get(1, 3) + mtr.Get(1, 3),
							 Get(2, 0) + mtr.Get(2, 0), Get(2, 1) + mtr.Get(2, 1), Get(2, 2) + mtr.Get(2, 2), Get(2, 3) + mtr.Get(2, 3),
							 Get(3, 0) + mtr.Get(3, 0), Get(3, 1) + mtr.Get(3, 1), Get(3, 2) + mtr.Get(3, 2), Get(3, 3) + mtr.Get(3, 3));
		}

		TE_INLINE const teMatrix4<T> operator - (const teMatrix4 & mtr) const
		{
			return teMatrix4(Get(0, 0) - mtr.Get(0, 0), Get(0, 1) - mtr.Get(0, 1), Get(0, 2) - mtr.Get(0, 2), Get(0, 3) - mtr.Get(0, 3),
							 Get(1, 0) - mtr.Get(1, 0), Get(1, 1) - mtr.Get(1, 1), Get(1, 2) - mtr.Get(1, 2), Get(1, 3) - mtr.Get(1, 3),
							 Get(2, 0) - mtr.Get(2, 0), Get(2, 1) - mtr.Get(2, 1), Get(2, 2) - mtr.Get(2, 2), Get(2, 3) - mtr.Get(2, 3),
							 Get(3, 0) - mtr.Get(3, 0), Get(3, 1) - mtr.Get(3, 1), Get(3, 2) - mtr.Get(3, 2), Get(3, 3) - mtr.Get(3, 3));
		}

		TE_INLINE const teMatrix4<T> operator * (const teMatrix4 & mtr) const
		{
			teMatrix4<T> temp;
			Multiplication(mtr, temp);
			return temp;
		}

		TE_INLINE const teMatrix4<T> operator * (T f) const
		{
			return teMatrix4(Get(0, 0) * f, Get(0, 1) * f, Get(0, 2) * f, Get(0, 3) * f,
							 Get(1, 0) * f, Get(1, 1) * f, Get(1, 2) * f, Get(1, 3) * f,
							 Get(2, 0) * f, Get(2, 1) * f, Get(2, 2) * f, Get(2, 3) * f,
							 Get(3, 0) * f, Get(3, 1) * f, Get(3, 2) * f, Get(3, 3) * f);
		}
		TE_INLINE const teMatrix4<T> operator / (T f) const
		{
			return teMatrix4(Get(0, 0) / f, Get(0, 1) / f, Get(0, 2) / f, Get(0, 3) / f,
							 Get(1, 0) / f, Get(1, 1) / f, Get(1, 2) / f, Get(1, 3) / f,
							 Get(2, 0) / f, Get(2, 1) / f, Get(2, 2) / f, Get(2, 3) / f,
							 Get(3, 0) / f, Get(3, 1) / f, Get(3, 2) / f, Get(3, 3) / f);
		}

		TE_INLINE teMatrix4<T> & operator += (const teMatrix4 & mtr)
		{
			return ((*this) = (*this) + mtr);
		}

		TE_INLINE teMatrix4<T> & operator -= (const teMatrix4 & mtr)
		{
			return ((*this) = (*this) - mtr);
		}

		TE_INLINE teMatrix4<T> & operator *= (const teMatrix4 & mtr)
		{
			return ((*this) = (*this) * mtr);
		}

		TE_INLINE teMatrix4<T>& operator *= (T f)
		{
			return ((*this) = (*this) * f);
		}

		TE_INLINE teMatrix4<T>& operator /= (T f)
		{
			return ((*this) = (*this) / f);
		}

		TE_INLINE T Determinant() const
		{
			T a = Get(2, 2) * Get(3, 3) - Get(2, 3) * Get(3, 2);
			T b = Get(2, 1) * Get(3, 3) - Get(2, 3) * Get(3, 1);
			T c = Get(2, 1) * Get(3, 2) - Get(2, 2) * Get(3, 1);
			T d = Get(2, 0) * Get(3, 3) - Get(2, 3) * Get(3, 0);
			T e = Get(2, 0) * Get(3, 2) - Get(2, 2) * Get(3, 0);
			T f = Get(2, 0) * Get(3, 1) - Get(2, 1) * Get(3, 0);

			return Get(0, 0) * (a * Get(1, 1) - b * Get(1, 2) + c * Get(1, 3))
				 - Get(0, 1) * (a * Get(1, 0) - d * Get(1, 2) + e * Get(1, 3))
				 + Get(0, 2) * (b * Get(1, 0) - d * Get(1, 1) + f * Get(1, 3))
				 - Get(0, 3) * (c * Get(1, 0) - e * Get(1, 1) + f * Get(1, 2));
		}

		TE_INLINE void Inverse(teMatrix4 & result) const
		{
			const T det = 1.0f / Determinant();

			#define MDET3(a0, a1, a2, a3, a4, a5, a6, a7, a8) \
				( a0 * (a4 * a8 - a5 * a7) \
				- a1 * (a3 * a8 - a5 * a6) \
				+ a2 * (a3 * a7 - a4 * a6) )

			result.Set(
				 MDET3(Get(1, 1), Get(1, 2), Get(1, 3), Get(2, 1), Get(2, 2), Get(2, 3), Get(3, 1), Get(3, 2), Get(3, 3)) * det,
				-MDET3(Get(0, 1), Get(0, 2), Get(0, 3), Get(2, 1), Get(2, 2), Get(2, 3), Get(3, 1), Get(3, 2), Get(3, 3)) * det,
				 MDET3(Get(0, 1), Get(0, 2), Get(0, 3), Get(1, 1), Get(1, 2), Get(1, 3), Get(3, 1), Get(3, 2), Get(3, 3)) * det,
				-MDET3(Get(0, 1), Get(0, 2), Get(0, 3), Get(1, 1), Get(1, 2), Get(1, 3), Get(2, 1), Get(2, 2), Get(2, 3)) * det,
				-MDET3(Get(1, 0), Get(1, 2), Get(1, 3), Get(2, 0), Get(2, 2), Get(2, 3), Get(3, 0), Get(3, 2), Get(3, 3)) * det,
				 MDET3(Get(0, 0), Get(0, 2), Get(0, 3), Get(2, 0), Get(2, 2), Get(2, 3), Get(3, 0), Get(3, 2), Get(3, 3)) * det,
				-MDET3(Get(0, 0), Get(0, 2), Get(0, 3), Get(1, 0), Get(1, 2), Get(1, 3), Get(3, 0), Get(3, 2), Get(3, 3)) * det,
				 MDET3(Get(0, 0), Get(0, 2), Get(0, 3), Get(1, 0), Get(1, 2), Get(1, 3), Get(2, 0), Get(2, 2), Get(2, 3)) * det,
				 MDET3(Get(1, 0), Get(1, 1), Get(1, 3), Get(2, 0), Get(2, 1), Get(2, 3), Get(3, 0), Get(3, 1), Get(3, 3)) * det,
				-MDET3(Get(0, 0), Get(0, 1), Get(0, 3), Get(2, 0), Get(2, 1), Get(2, 3), Get(3, 0), Get(3, 1), Get(3, 3)) * det,
				 MDET3(Get(0, 0), Get(0, 1), Get(0, 3), Get(1, 0), Get(1, 1), Get(1, 3), Get(3, 0), Get(3, 1), Get(3, 3)) * det,
				-MDET3(Get(0, 0), Get(0, 1), Get(0, 3), Get(1, 0), Get(1, 1), Get(1, 3), Get(2, 0), Get(2, 1), Get(2, 3)) * det,
				-MDET3(Get(1, 0), Get(1, 1), Get(1, 2), Get(2, 0), Get(2, 1), Get(2, 2), Get(3, 0), Get(3, 1), Get(3, 2)) * det,
				 MDET3(Get(0, 0), Get(0, 1), Get(0, 2), Get(2, 0), Get(2, 1), Get(2, 2), Get(3, 0), Get(3, 1), Get(3, 2)) * det,
				-MDET3(Get(0, 0), Get(0, 1), Get(0, 2), Get(1, 0), Get(1, 1), Get(1, 2), Get(3, 0), Get(3, 1), Get(3, 2)) * det,
				 MDET3(Get(0, 0), Get(0, 1), Get(0, 2), Get(1, 0), Get(1, 1), Get(1, 2), Get(2, 0), Get(2, 1), Get(2, 2)) * det);

			#undef MDET3
		}

		TE_INLINE T Determinant33() const
		{
			return	 Get(0, 0) * (Get(1, 1) * Get(2, 2) - Get(1, 2) * Get(2, 1))
				   - Get(0, 1) * (Get(1, 0) * Get(2, 2) - Get(1, 2) * Get(2, 0))
				   + Get(0, 2) * (Get(1, 0) * Get(2, 1) - Get(1, 1) * Get(2, 0));
		}

		TE_INLINE void Inverse33(teMatrix4 & result) const
		{
			const T det = 1.0f / Determinant33();
		
			result.Get(0, 0) = (Get(1, 1) * Get(2, 2) - Get(1, 2) * Get(2, 1)) * det;
			result.Get(0, 1) = (Get(0, 2) * Get(2, 1) - Get(0, 1) * Get(2, 2)) * det;
			result.Get(0, 2) = (Get(0, 1) * Get(1, 2) - Get(0, 2) * Get(1, 1)) * det;
			result.Get(1, 0) = (Get(1, 2) * Get(2, 0) - Get(1, 0) * Get(2, 2)) * det;
			result.Get(1, 1) = (Get(0, 0) * Get(2, 2) - Get(0, 2) * Get(2, 0)) * det;
			result.Get(1, 2) = (Get(0, 2) * Get(1, 0) - Get(0, 0) * Get(1, 2)) * det;
			result.Get(2, 0) = (Get(1, 0) * Get(2, 1) - Get(1, 1) * Get(2, 0)) * det;
			result.Get(2, 1) = (Get(0, 1) * Get(2, 0) - Get(0, 0) * Get(2, 1)) * det;
			result.Get(2, 2) = (Get(0, 0) * Get(1, 1) - Get(0, 1) * Get(1, 0)) * det;
			
		}

		TE_INLINE void Transpose(teMatrix4 & result) const
		{
			result.Set(Get(0, 0), Get(1, 0), Get(2, 0), Get(3, 0),
					   Get(0, 1), Get(1, 1), Get(2, 1), Get(3, 1),
					   Get(0, 2), Get(1, 2), Get(2, 2), Get(3, 2),
					   Get(0, 3), Get(1, 3), Get(2, 3), Get(3, 3));
		}

		TE_INLINE void Rotate(const teVector3d<T> & r)
		{
			teMatrix4<T> rM;
			rM.SetRotation(r);
			*this *= rM;
		}

		TE_INLINE void SetRotation(const teVector3d<T> & r)
		{
			const T cx = teCos(r.x * teDegToRad), sx = teSin(r.x * teDegToRad),
					cy = teCos(r.y * teDegToRad), sy = teSin(r.y * teDegToRad),
					cz = teCos(r.z * teDegToRad), sz = teSin(r.z * teDegToRad);

			// rotationX * rotationY * rotationZ
			Set(cy * cz,			   -cy * sz,				sy,		 0,
				cx * sz + sx * cz * sy, cx * cz - sx * sy * sz,-cy * sx, 0,
				sx * sz - cx * cz * sy, cz * sx + cx * sy * sz, cx * cy, 0,
				0,						0,						0,		 1);
		}

		TE_INLINE void Scale(const teVector3d<T> & s)
		{
			teMatrix4<T> sM;
			sM.SetScale(s);
			*this *= sM;
		}

		TE_INLINE void SetScale(const teVector3d<T> & s)
		{
			Set(s.x, 0, 0, 0,
				0, s.y, 0, 0,
				0, 0, s.z, 0,
				0, 0, 0, 1);
		}

		TE_INLINE void Translate(const teVector3d<T> &t)
		{
			teMatrix4<T> tM;
			tM.SetTranslation(t);
			*this *= tM;
		}

		TE_INLINE void SetTranslation(const teVector3d<T> &t)
		{
			Set(1, 0, 0, t.x,
				0, 1, 0, t.y,
				0, 0, 1, t.z,
				0, 0, 0, 1);
		}

		TE_INLINE void SetTransforms2D(const teVector2d<T> &pos, const teVector2d<T> & scale, T angle)
		{
			// scale, rotate
			const T c = teCos(angle * teDegToRad),
					s = teSin(angle * teDegToRad);

			Get(0, 0) = scale.x * c; // rotate around Z
			Get(0, 1) = -s;
			Get(0, 2) = 0;
			Get(1, 0) = s;
			Get(1, 1) = scale.y * c;
			Get(1, 2) = 0;
			Get(2, 0) = 0;
			Get(2, 1) = 0;
			Get(2, 2) = 1;

			//translate
			Get(0, 3) = pos.x;
			Get(1, 3) = pos.y;
			Get(2, 3) = 0;
			Get(3, 3) = 1;
			Get(3, 0) = 0;
			Get(3, 1) = 0;
			Get(3, 2) = 0;
		}

		TE_INLINE void SetTransforms3D(const teVector3d<T> & pos, const teVector3d<T> & scale, const teMatrix4 & rotM) // VADIM
		{
			SetIdentity();

			// scale
			Get(0, 0) = scale.x;
			Get(1, 1) = scale.y;
			Get(2, 2) = scale.z;

			// rotate
			*this *= rotM;

			//translate
			Get(0, 3) += pos.x;
			Get(1, 3) += pos.y;
			Get(2, 3) += pos.z;
		}


		TE_INLINE void SetOrthographic(T left, T right,
									   T bottom, T top, T zNear, T zFar)
		{
			const T tx = - (right + left) / (right - left),
					ty = - (top + bottom) / (top - bottom),
					tz = - (zFar + zNear) / (zFar - zNear);

			Set(2.0f / (right - left), 0, 0, tx,
				0, 2.0f / (top - bottom), 0, ty,
				0, 0, -2.0f / (zFar - zNear), tz,
				0, 0, 0, 1);
		}

		TE_INLINE void SetPerspective(T fov, T aspect, T zNear, T zFar)
		{
			const T f = 1.0f / teTan(fov * teDegToRad / 2.0f),
					a = (zFar + zNear) / (zNear - zFar),
					b = (2.0f * zFar * zNear) / (zNear - zFar);

			Set(f / aspect, 0, 0, 0,
				0,			f, 0, 0,
				0,			0, a, b,
				0,			0, -1, 0);
		}

		TE_INLINE void SetPerspective(T fovX, T fovY, T aspect, T zNear, T zFar)
		{
			const T wideX = 1.0f / teTan(fovX * teDegToRad / 2.0f),
					wideY = 1.0f / teTan(fovY * teDegToRad / 2.0f),
					a = (zFar + zNear) / (zNear - zFar),
					b = (2 * zFar * zNear) / (zNear - zFar);

			Set(wideX,	0,     0, 0,
				0,		wideY, 0, 0,
				0,		0,     a, b,
				0,		0,    -1, 0);
		}

		TE_INLINE teVector2d<T> MultiplyMatrixOnVector2D(const teVector2d<T> & Vector) const
		{
			teVector2d<T> temp;

			temp.x = Get(0, 0) * Vector.x + Get(0, 1) * Vector.y + Get(0, 3);
			temp.y = Get(1, 0) * Vector.x + Get(1, 1) * Vector.y + Get(1, 3);

			return temp;
		}

		TE_INLINE teVector3d<T> MultiplyMatrixOnVector3D(const teVector3d<T> & Vector) const
		{
			teVector3d<T> temp;

			temp.x = Get(0, 0) * Vector.x + Get(0, 1) * Vector.y + Get(0, 2) * Vector.z + Get(0, 3);
			temp.y = Get(1, 0) * Vector.x + Get(1, 1) * Vector.y + Get(1, 2) * Vector.z + Get(1, 3);
			temp.z = Get(2, 0) * Vector.x + Get(2, 1) * Vector.y + Get(2, 2) * Vector.z + Get(2, 3);

			return temp;
		}

		TE_INLINE void MultiplyMatrixOnVector4D(T x, T y, T z, T w, T * arr) const
		{
			arr[0] = Get(0, 0) * x + Get(0, 1) * y + Get(0, 2) * z + Get(0, 3) * w;
			arr[1] = Get(1, 0) * x + Get(1, 1) * y + Get(1, 2) * z + Get(1, 3) * w;
			arr[2] = Get(2, 0) * x + Get(2, 1) * y + Get(2, 2) * z + Get(2, 3) * w;
			arr[3] = Get(3, 0) * x + Get(3, 1) * y + Get(3, 2) * z + Get(3, 3) * w;
		}

		TE_INLINE void Multiplication(const teMatrix4<T> & mtr, teMatrix4<T> & result) const
		{
			result.Set(Get(0, 0) * mtr.Get(0, 0) + Get(0, 1) * mtr.Get(1, 0) + Get(0, 2) * mtr.Get(2, 0) + Get(0, 3) * mtr.Get(3, 0),
					   Get(0, 0) * mtr.Get(0, 1) + Get(0, 1) * mtr.Get(1, 1) + Get(0, 2) * mtr.Get(2, 1) + Get(0, 3) * mtr.Get(3, 1),
					   Get(0, 0) * mtr.Get(0, 2) + Get(0, 1) * mtr.Get(1, 2) + Get(0, 2) * mtr.Get(2, 2) + Get(0, 3) * mtr.Get(3, 2),
					   Get(0, 0) * mtr.Get(0, 3) + Get(0, 1) * mtr.Get(1, 3) + Get(0, 2) * mtr.Get(2, 3) + Get(0, 3) * mtr.Get(3, 3),
					   Get(1, 0) * mtr.Get(0, 0) + Get(1, 1) * mtr.Get(1, 0) + Get(1, 2) * mtr.Get(2, 0) + Get(1, 3) * mtr.Get(3, 0),
					   Get(1, 0) * mtr.Get(0, 1) + Get(1, 1) * mtr.Get(1, 1) + Get(1, 2) * mtr.Get(2, 1) + Get(1, 3) * mtr.Get(3, 1),
					   Get(1, 0) * mtr.Get(0, 2) + Get(1, 1) * mtr.Get(1, 2) + Get(1, 2) * mtr.Get(2, 2) + Get(1, 3) * mtr.Get(3, 2),
					   Get(1, 0) * mtr.Get(0, 3) + Get(1, 1) * mtr.Get(1, 3) + Get(1, 2) * mtr.Get(2, 3) + Get(1, 3) * mtr.Get(3, 3),
					   Get(2, 0) * mtr.Get(0, 0) + Get(2, 1) * mtr.Get(1, 0) + Get(2, 2) * mtr.Get(2, 0) + Get(2, 3) * mtr.Get(3, 0),
					   Get(2, 0) * mtr.Get(0, 1) + Get(2, 1) * mtr.Get(1, 1) + Get(2, 2) * mtr.Get(2, 1) + Get(2, 3) * mtr.Get(3, 1),
					   Get(2, 0) * mtr.Get(0, 2) + Get(2, 1) * mtr.Get(1, 2) + Get(2, 2) * mtr.Get(2, 2) + Get(2, 3) * mtr.Get(3, 2),
					   Get(2, 0) * mtr.Get(0, 3) + Get(2, 1) * mtr.Get(1, 3) + Get(2, 2) * mtr.Get(2, 3) + Get(2, 3) * mtr.Get(3, 3),
					   Get(3, 0) * mtr.Get(0, 0) + Get(3, 1) * mtr.Get(1, 0) + Get(3, 2) * mtr.Get(2, 0) + Get(3, 3) * mtr.Get(3, 0),
					   Get(3, 0) * mtr.Get(0, 1) + Get(3, 1) * mtr.Get(1, 1) + Get(3, 2) * mtr.Get(2, 1) + Get(3, 3) * mtr.Get(3, 1),
					   Get(3, 0) * mtr.Get(0, 2) + Get(3, 1) * mtr.Get(1, 2) + Get(3, 2) * mtr.Get(2, 2) + Get(3, 3) * mtr.Get(3, 2),
					   Get(3, 0) * mtr.Get(0, 3) + Get(3, 1) * mtr.Get(1, 3) + Get(3, 2) * mtr.Get(2, 3) + Get(3, 3) * mtr.Get(3, 3));
		}

		TE_INLINE void MultiplicationAffine(const teMatrix4<T> & mtr, teMatrix4<T> & result) const
		{
			#define TE_MULTMATRIX_ELEMENT_A(i, j) \
				{result.Get(i, j) = \
				(*this).Get(i, 0) * mtr.Get(0, j) + \
				(*this).Get(i, 1) * mtr.Get(1, j) + \
				(*this).Get(i, 2) * mtr.Get(2, j);}

			#define TE_MULTMATRIX_ELEMENT_A_F(i, j) \
				{result.Get(i, j) = \
				(*this).Get(i, 0) * mtr.Get(0, j) + \
				(*this).Get(i, 1) * mtr.Get(1, j) + \
				(*this).Get(i, 2) * mtr.Get(2, j) + \
				(*this).Get(i, 3);}

			TE_MULTMATRIX_ELEMENT_A(0, 0);
			TE_MULTMATRIX_ELEMENT_A(0, 1);
			TE_MULTMATRIX_ELEMENT_A(0, 2);
			TE_MULTMATRIX_ELEMENT_A(1, 0);
			TE_MULTMATRIX_ELEMENT_A(1, 1);
			TE_MULTMATRIX_ELEMENT_A(1, 2);
			TE_MULTMATRIX_ELEMENT_A(2, 0);
			TE_MULTMATRIX_ELEMENT_A(2, 1);
			TE_MULTMATRIX_ELEMENT_A(2, 2);

			TE_MULTMATRIX_ELEMENT_A_F(0, 3);
			TE_MULTMATRIX_ELEMENT_A_F(1, 3);
			TE_MULTMATRIX_ELEMENT_A_F(2, 3);

			result.Get(3, 0) = 0;
			result.Get(3, 1) = 0;
			result.Get(3, 2) = 0;
			result.Get(3, 3) = 1;
		}
	};

	//! f32 Matrix 4x4
	typedef teMatrix4<f32> teMatrix4f;

#undef Get
}

#endif
