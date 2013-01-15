/*
 *  teFrustum.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 11/24/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEFRUSTUM_H
#define TE_TEFRUSTUM_H

#include "teTypes.h"
#include "tePlane.h"
#include "teAABB3d.h"
//#include "teMatrix4"
#include "teMath.h"

namespace te
{
	struct teFrustum
	{
	public:
		tePlanef planes[6];

		void CalculateFrustum(const teMatrix4f & projection, const teMatrix4f & modelView)
		{
			teMatrix4f clip;
			projection.Multiplication(modelView, clip);
			
			planes[0].a = clip.Get(3, 0) - clip.Get(0, 0);
			planes[0].b = clip.Get(3, 1) - clip.Get(0, 1);
			planes[0].c = clip.Get(3, 2) - clip.Get(0, 2);
			planes[0].d = clip.Get(3, 3) - clip.Get(0, 3);
			
			planes[1].a = clip.Get(3, 0) + clip.Get(0, 0);
			planes[1].b = clip.Get(3, 1) + clip.Get(0, 1);
			planes[1].c = clip.Get(3, 2) + clip.Get(0, 2);
			planes[1].d = clip.Get(3, 3) + clip.Get(0, 3);
			
			planes[2].a = clip.Get(3, 0) + clip.Get(1, 0);
			planes[2].b = clip.Get(3, 1) + clip.Get(1, 1);
			planes[2].c = clip.Get(3, 2) + clip.Get(1, 2);
			planes[2].d = clip.Get(3, 3) + clip.Get(1, 3);
			
			planes[3].a = clip.Get(3, 0) - clip.Get(1, 0);
			planes[3].b = clip.Get(3, 1) - clip.Get(1, 1);
			planes[3].c = clip.Get(3, 2) - clip.Get(1, 2);
			planes[3].d = clip.Get(3, 3) - clip.Get(1, 3);
			
			planes[4].a = clip.Get(3, 0) - clip.Get(2, 0);
			planes[4].b = clip.Get(3, 1) - clip.Get(2, 1);
			planes[4].c = clip.Get(3, 2) - clip.Get(2, 2);
			planes[4].d = clip.Get(3, 3) - clip.Get(2, 3);
			
			planes[5].a = clip.Get(3, 0) + clip.Get(2, 0);
			planes[5].b = clip.Get(3, 1) + clip.Get(2, 1);
			planes[5].c = clip.Get(3, 2) + clip.Get(2, 2);
			planes[5].d = clip.Get(3, 3) + clip.Get(2, 3);
			
			for(u8 i = 0; i < 6; ++i)
			{
				f32 t = teInvSqrt(planes[i].a * planes[i].a + planes[i].b * planes[i].b + planes[i].c * planes[i].c);
				planes[i].a *= t;
				planes[i].b *= t;
				planes[i].c *= t;
				planes[i].d *= t;
			}
		}
		
		TE_INLINE u1 IsSphereIn(const teVector3df & position, f32 radius) const
		{
			for(u8 i = 0; i < 6; ++i)
				if(planes[i].GetDistance(position) < -radius)
					return false;
			
			return true;
		}
		
		TE_INLINE u1 IsPointIn(const teVector3df & position) const
		{
			return IsSphereIn(position, 0.0f);
		}

		TE_INLINE u1 IsAABBIn(const teAABB3df & aabb) const
		{
			teVector3df middle = aabb.GetMiddle();
			teVector3df radius = aabb.GetRadius();

			for(u8 i = 0; i < 6; ++i)
			{
				teVector3df normal = planes[i].GetNormal();
				f32 distanceMax = teAbs(normal.x * radius.x) + teAbs(normal.y * radius.y) + teAbs(normal.z * radius.z);

				if(planes[i].GetDistance(middle) < -distanceMax)
					return false;
			}

			return true;
		}
	};
}

#endif
