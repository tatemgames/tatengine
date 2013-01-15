/*
 *  teMathTools.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 02/12/10.
 *  Copyright 2010 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEMATHTOOLS_H
#define TE_TEMATHTOOLS_H

#include "teTypes.h"
#include "teVector2D.h"

namespace te
{
	namespace core
	{
		TE_FUNC f32 GetDistanceFromPointToLine(const teVector2df & linePoint1, const teVector2df & linePoint2, const teVector2df & point)
		{
			teVector2df line = linePoint2 - linePoint1;
			teVector2df pointLine = linePoint1 - point;

			return teAbs(pointLine.y * line.x - pointLine.x * line.y) / line.GetDistance();
		}

		TE_FUNC f32 GetDistanceFromPointToLineSegment(const teVector2df & linePoint1, const teVector2df & linePoint2, const teVector2df & point)
		{
			teVector2df line = linePoint2 - linePoint1;
			teVector2df pointLine = point - linePoint1;

			f32 r1 = pointLine.DotProduct(line);
			f32 r2 = line.GetDistanceQR();

			f32 r = r1 / r2;

			if((r < 0.0f) || (r > 1.0f))
				return teSqrt(teMin(pointLine.GetDistanceQR(), (point - linePoint2).GetDistanceQR()));
			else
				return fabs((pointLine.y * line.x - pointLine.x * line.y) / r2) * teSqrt(r2);
		}

		TE_FUNC u1 IsPointInsideTriangle(const teVector2df & point, const teVector2df & trianglePointA, const teVector2df & trianglePointB, const teVector2df & trianglePointC)
		{
			teVector2df v = point;
			teVector2df v0 = trianglePointA;
			teVector2df v1 = trianglePointB - trianglePointA;
			teVector2df v2 = trianglePointC - trianglePointA;

			f32 a =  (v.GetDet(v2) - v0.GetDet(v2)) / v1.GetDet(v2);
			f32 b = -(v.GetDet(v1) - v0.GetDet(v1)) / v1.GetDet(v2);

			return (a >= 0.0f) && (b >= 0.0f) && (a + b <= 1.0f);
		}
	}
}

#endif
