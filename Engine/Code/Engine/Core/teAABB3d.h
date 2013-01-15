/*
 *  teAABB3d.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 09/1/11.
 *  Copyright 2011 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEAABB3D_H
#define TE_TEAABB3D_H

#include "teVector3d.h"
#include "teMatrix4.h"

namespace te
{
	//! Axis aligned bounding box 3d
	template<typename T>
	class teAABB3d
	{
	public:
		teVector3d<T> edgeMin, edgeMax;

		TE_INLINE teAABB3d()
		{
		}

		TE_INLINE teAABB3d(const teVector3d<T> & setMin, const teVector3d<T> & setMax)
			:edgeMin(setMin), edgeMax(setMax)
		{
		}
		
		template <class B>
		inline teAABB3d(const teAABB3d<B> & other)
			:edgeMin(other.edgeMin), edgeMax(other.edgeMax)
		{
		}

		inline void Flush()
		{
			edgeMin.Flush();
			edgeMax.Flush();
		}

		inline void SetEdges(const teVector3d<T> & setMin, const teVector3d<T> & setMax)
		{
			edgeMin = setMin;
			edgeMax = setMax;
		}

		teVector3d<T> GetMiddle() const
		{
			return (edgeMax + edgeMin) / 2;
		}

		teVector3d<T> GetRadius() const
		{
			return edgeMax - GetMiddle();
		}

		teVector3df GetSize() const
		{
			return edgeMax - edgeMin;
		}

		template<typename B>
		inline teAABB3d<T> & operator = (const teAABB3d<B> & other)
		{
			edgeMin = other.edgeMin;
			edgeMax = other.edgeMax;
			return *this;
		}

		inline u1 IsEqual(const teAABB3d<T> & other) const
		{
			return (edgeMin == other.edgeMin) && (edgeMax == other.edgeMax);
		}

		void Unite(const teAABB3d<T> & other)
		{
			edgeMin.x = teMin(edgeMin.x, other.edgeMin.x);
			edgeMin.y = teMin(edgeMin.y, other.edgeMin.y);
			edgeMin.z = teMin(edgeMin.z, other.edgeMin.z);
			edgeMax.x = teMax(edgeMax.x, other.edgeMax.x);
			edgeMax.y = teMax(edgeMax.y, other.edgeMax.y);
			edgeMax.z = teMax(edgeMax.z, other.edgeMax.z);
		}

		void TransformAffine(const teMatrix4f & matrix)
		{
			teVector3d<T> middle = GetMiddle();
			teVector3d<T> radius = GetRadius();

			middle = matrix.MultiplyMatrixOnVector3D(middle);

			radius.SetXYZ(
				teAbs(matrix.Get(0, 0)) * radius.x + teAbs(matrix.Get(0, 1)) * radius.y + teAbs(matrix.Get(0, 2)) * radius.z,
				teAbs(matrix.Get(1, 0)) * radius.x + teAbs(matrix.Get(1, 1)) * radius.y + teAbs(matrix.Get(1, 2)) * radius.z,
				teAbs(matrix.Get(2, 0)) * radius.x + teAbs(matrix.Get(2, 1)) * radius.y + teAbs(matrix.Get(2, 2)) * radius.z);

			SetEdges(middle - radius, middle + radius);
		}

		u1 IsIntersectAABB(const teAABB3d<T> & other) const
		{
			return (edgeMin.x <= other.edgeMax.x) && (edgeMax.x >= other.edgeMin.x) &&
				(edgeMin.y <= other.edgeMax.y) && (edgeMax.y >= other.edgeMin.y) &&
				(edgeMin.z <= other.edgeMax.z) && (edgeMax.z >= other.edgeMin.z);
		}

		u1 IsIntersectLineSegment(const teVector3d<T> & point1, const teVector3d<T> & point2) const
		{
			teVector3d<T> lineDir = (point2 - point1).Normalize();
			teVector3d<T> lineCenter = (point2 - point1) / 2;
			teVector3d<T> lineCenterAbs(teAbs(lineCenter.x), teAbs(lineCenter.y), teAbs(lineCenter.z));

			teVector3d<T> edgeSize = (edgeMax - edgeMin) / 2;

			teVector3d<T> a = GetMiddle() - (point2 + point1) / 2;
			teVector3d<T> b = edgeSize + lineCenterAbs;
	
			if((teAbs(a.x) > b.x) || (teAbs(a.y) > b.y) || (teAbs(a.z) > b.z))
				return false;

			a = a.CrossProduct(lineDir);
			b.SetXYZ(edgeSize.y * teAbs(lineDir.z) + edgeSize.z * teAbs(lineDir.y), edgeSize.x * teAbs(lineDir.z) + edgeSize.z * teAbs(lineDir.x), edgeSize.x * teAbs(lineDir.y) + edgeSize.y * teAbs(lineDir.x));

			if((teAbs(a.x) > b.x) || (teAbs(a.y) > b.y) || (teAbs(a.z) > b.z))
				return false;

			return true;
		}
	};
	
	typedef teAABB3d<f32> teAABB3df;
};

#endif