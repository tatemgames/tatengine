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

		TE_FUNC f32 LineSegmentsSide(const teVector2df & p, const teVector2df & q, const teVector2df & a, const teVector2df & b)
		{
			f32 z1 = (b.x - a.x) * (p.y - a.y) - (p.x - a.x) * (b.y - a.y);
			f32 z2 = (b.x - a.x) * (q.y - a.y) - (q.x - a.x) * (b.y - a.y);
			return z1 * z2;
		}

		TE_FUNC u1 IsLineSegmentIntersectTriangle(const teVector2df & lineSegmentA, const teVector2df & lineSegmentB, const teVector2df & trianglePointA, const teVector2df & trianglePointB, const teVector2df & trianglePointC)
		{
			f32 f1 = LineSegmentsSide(lineSegmentA, trianglePointC, trianglePointA, trianglePointB);
			f32 f2 = LineSegmentsSide(lineSegmentB, trianglePointC, trianglePointA, trianglePointB);
			f32 f3 = LineSegmentsSide(lineSegmentA, trianglePointA, trianglePointB, trianglePointC);
			f32 f4 = LineSegmentsSide(lineSegmentB, trianglePointA, trianglePointB, trianglePointC);
			f32 f5 = LineSegmentsSide(lineSegmentA, trianglePointB, trianglePointC, trianglePointA);
			f32 f6 = LineSegmentsSide(lineSegmentB, trianglePointB, trianglePointC, trianglePointA);
			f32 f7 = LineSegmentsSide(trianglePointA, trianglePointB, lineSegmentA, lineSegmentB);
			f32 f8 = LineSegmentsSide(trianglePointB, trianglePointC, lineSegmentA, lineSegmentB);

			if((f1 < 0 && f2 < 0) || (f3 < 0 && f4 < 0) || (f5 < 0 && f6 < 0) || (f7 > 0 && f8 > 0))
				return false;
			else if((f1 == 0 && f2 == 0) || (f3 == 0 && f4 == 0) || (f5 == 0 && f6 == 0))
				return true;
			else if((f1 <= 0 && f2 <= 0) || (f3 <= 0 && f4 <= 0) || (f5 <= 0 && f6 <= 0) || (f7 >= 0 && f8 >= 0))
				return true;
			else if(f1 > 0 && f2 > 0 && f3 > 0 && f4 > 0 && f5 > 0 && f6 > 0)
				return false;
			else
				return true;
		}

		TE_FUNC u1 IsLineSegmentIntersectLineSegment(const teVector2df & lineSegment1A, const teVector2df & lineSegment1B, const teVector2df & lineSegment2A, const teVector2df & lineSegment2B, teVector2df * intersectionPoint = NULL)
		{
			teVector2df b = lineSegment1B - lineSegment1A;
			teVector2df d = lineSegment2B - lineSegment2A;
			teVector2df c = lineSegment2A - lineSegment1A;

			f32 dot = b.GetDet(d);

			if(teAbs(dot) < teRoundingError32)
				return false;

			f32 dot2 = c.GetDet(d) / dot;

			if((dot2 < 0) || (dot2 > 1))
				return false;

			f32 dot3 = c.GetDet(b) / dot;

			if((dot3 < 0) || (dot3 > 1))
				return false;

			if(intersectionPoint)
				*intersectionPoint = lineSegment1A + b * dot2;

			return true;
		}
	}
}

#endif
