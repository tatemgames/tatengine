/*
 *  teSpline.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 02/24/10.
 *  Copyright 2010 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TESPLINE_H
#define TE_TESPLINE_H

#include "teVector2D.h"
#include "teVector3D.h"
#include "teConstArray.h"

namespace te
{
	enum ESplineType
	{
		ST_CARDINAL_CUBIC_HERMITE,
		ST_KOCHANEK_BARTELS
	};

	struct teSpline
	{
		f32 length;
		f32 koefA;
		f32 koefB;
		f32 koefC;
		f32 deltaLength;
		f32 deltaAnchors;
		teConstArray<teVector3df> points;
		teConstArray<teVector2df> anchors;
		ESplineType type;

		TE_INLINE teSpline()
			:length(0.0f), deltaLength(1.0f / 1000.0f), deltaAnchors(1.0f / 200.0f)
		{
			SetupCardinalCubicHermiteSpline(0.5f);
		}

		TE_INLINE ~teSpline()
		{
			anchors.Clear();
			points.Clear();
		}

		TE_INLINE teConstArray<teVector3df> & GetPoints() {return points;}
		TE_INLINE const teConstArray<teVector3df> & GetPoints() const {return points;}

		void SetupNewPoints(u1 setupAnchors = false)
		{
			length = 0.0f;

			teVector3df lastPoint = GetPoint(0.0f);
			for(f32 u = deltaLength; u < 1.0f; u += deltaLength)
			{
				teVector3df point = GetPoint(u);
				length += (point - lastPoint).GetDistance();
				lastPoint = point;
			}

			if(setupAnchors)
			{
				u32 newSize = ((s32)(1.0f / deltaAnchors)) + 1;

				if(newSize != anchors.GetSize())
				{
					anchors.Clear();
					anchors.Reserve(newSize);
					anchors.Request(anchors.GetSize());
				}

				f32 s = 0.0f;
				u32 i = 0;

				teVector3df lastPoint = GetPoint(0.0f);
				for(f32 u = deltaAnchors; u < 1.0f; u += deltaAnchors)
				{
					teVector3df point = GetPoint(u);
					s += (point - lastPoint).GetDistance();
					anchors[teMin(i, (u32)(anchors.GetAlive() - 1))].SetXY(s / length, u);
					++i;
					lastPoint = point;
				}
			}
		}

		TE_INLINE void SetupCardinalCubicHermiteSpline(f32 c)
		{
			type = ST_CARDINAL_CUBIC_HERMITE;
			koefA = c;
			koefB = 0.0f;
			koefC = 0.0f;
		}

		TE_INLINE void SetupKochanekBartelsSpline(f32 tension, f32 bias, f32 continuity)
		{
			type = ST_KOCHANEK_BARTELS;
			koefA = tension;
			koefB = continuity;
			koefC = bias;
		}

		TE_INLINE teVector3df GetPoint(f32 u)
		{
			if(points.GetAlive() < 2)
				return teVector3df();

			f32 localTime = u * (points.GetAlive() - 1); // TODO right setup for tangents in begin and end
			s32 number = (s32)teFloor(localTime);
			localTime -= number;

			switch(type)
			{
			case ST_CARDINAL_CUBIC_HERMITE:
				return GetCardinalCubicHermiteSplinePoint(localTime, number);
			case ST_KOCHANEK_BARTELS:
				return GetKochanekBartelsSplinePoint(localTime, number);
			default:
				return teVector3df();
			}
		}

		TE_INLINE teVector3df GetPointWithConstantSpeed(f32 s)
		{
			if(anchors.GetSize() < 2)
				return teVector3df();

			for(u32 i = 0; i < anchors.GetAlive() - 1; ++i)
			{
				if(((s >= anchors[i].x) && (s <= anchors[i + 1].x)) || (i == (anchors.GetAlive() - 2)))
				{
					f32 u1 = anchors[i].y;
					f32 u2 = anchors[i + 1].y;

					f32 dist = anchors[i + 1].x - anchors[i].x;
					f32 ut = (s - anchors[i].x) / dist;

					if(teIsNaN(ut))
						return teVector3df();

					f32 u = u1 * (1 - ut) + u2 * ut; // maybe some cubic here ?

					return GetPoint(u);
				}
			}

			return teVector3df();
		}

		TE_INLINE teVector3df GetArrayPoint(s32 i)
		{
			if(i < 0)
				return points[0] * 2.0f - points[1];
			else if((u32)i >= points.GetAlive())
				return points[points.GetAlive() - 1] * 2.0f - points[points.GetAlive() - 2];
			else
				return points[i];
		}

		TE_INLINE teVector3df GetCardinalCubicHermiteSplinePoint(f32 localTime, s32 number)
		{
			f32 h1 = 2.0f * pow(localTime, 3) - 3 * pow(localTime, 2) + 1;
			f32 h2 = 1 - h1;
			f32 h3 = pow(localTime, 3) - 2.0f * pow(localTime, 2) + localTime;
			f32 h4 = pow(localTime, 3) - pow(localTime, 2);

			teVector3df p1 = GetArrayPoint(number);
			teVector3df p2 = GetArrayPoint(number + 1);

			teVector3df t1 = (GetArrayPoint(number + 1) - GetArrayPoint(number - 1)) * koefA;
			teVector3df t2 = (GetArrayPoint(number + 2) - GetArrayPoint(number)) * koefA;

			return p1 * h1 + p2 * h2 + t1 * h3 + t2 * h4;
		}

		TE_INLINE teVector3df GetKochanekBartelsSplinePoint(f32 localTime, s32 number)
		{
			f32 h1 = 2.0f * pow(localTime, 3) - 3 * pow(localTime, 2) + 1;
			f32 h2 = 1 - h1;
			f32 h3 = pow(localTime, 3) - 2.0f * pow(localTime, 2) + localTime;
			f32 h4 = pow(localTime, 3) - pow(localTime, 2);

			teVector3df p1 = GetArrayPoint(number);
			teVector3df p2 = GetArrayPoint(number + 1);

			teVector3df t1 = (GetArrayPoint(number) - GetArrayPoint(number - 1)) * (1 - koefA) * (1 + koefB) * (1 + koefC) / 2.0f + (GetArrayPoint(number + 1) - GetArrayPoint(number)) * (1 - koefA) * (1 - koefB) * (1 - koefC) / 2.0f;
			teVector3df t2 = (GetArrayPoint(number + 1) - GetArrayPoint(number)) * (1 - koefA) * (1 - koefB) * (1 + koefC) / 2.0f + (GetArrayPoint(number + 2) - GetArrayPoint(number + 1)) * (1 - koefA) * (1 + koefB) * (1 - koefC) / 2.0f;

			return p1 * h1 + p2 * h2 + t1 * h3 + t2 * h4;
		}
	};
};

#endif
