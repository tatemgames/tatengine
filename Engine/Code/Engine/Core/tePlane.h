/*
 *  tePlane.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 11/24/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEPLANE_H
#define TE_TEPLANE_H

#include "teTypes.h"
#include "teVector3d.h"

namespace te
{
	//! Plane
	template<typename T>
	struct tePlane
	{
	public:
		T a, b, c, d;
		
		TE_INLINE tePlane():a(0), b(0), c(0), d(0) {}
		TE_INLINE tePlane(T setA, T setB, T setC, T setD):a(setA), b(setB), c(setC), d(setD) {}

		TE_INLINE T GetDistance(const teVector3d<T> & point) const {return a * point.x + b * point.y + c * point.z + d;}
		TE_INLINE teVector3d<T> GetNormal() const {return teVector3d<T>(a, b, c).Normalize();}
		
		TE_INLINE teVector3d<T> GetIntersect(teVector3d<T> & point, teVector3d<T> & dir)
		{
			T temp;
			teVector3d<T> norm = GetNormal();
			
			temp = -(d + norm.DotProduct(point))/(norm.DotProduct(dir));
			
			teVector3d<T> intersect;
			intersect = point + dir * temp;
			
			return intersect;
		}
		
	};
	
	typedef tePlane<f32> tePlanef;
};

#endif
