/*
 *  teActorCurve.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 12/11/11.
 *  Copyright 2011 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEACTORCURVE_H
#define TE_TEACTORCURVE_H

#include "teActor.h"
#include "teTimer.h"

namespace te
{
	namespace scene
	{
		const u8 teCurvePointsCount = 8;

		enum ECurveTypeFlags
		{
			CTF_INTERPOLATION_LINEAR	= 0x1,
			CTF_INTERPOLATION_CUBIC		= 0x2,
			CTF_INTERPOLATION_KBS		= 0x4,

			CTF_LOOPED					= 0x8,
			CTF_AUTOSTART				= 0x10,
			CTF_ADD_TO_START_VALUE		= 0x20
		};

		class teActorCurve
		{
		public:
			TE_INLINE teActorCurve(teFastScene * setScene)
				:scene(setScene), lastT(0.0f), enableAnimation(false)
			{
				valueType = scene->GetAssetPack().GetLinkDataType(value);

				if(value)
					GetLinkVariable(value, (ELinkDataType)valueType, valueAtStart);

				if(((u8)curveType) & CTF_AUTOSTART)
					Start();
			}

			TE_INLINE ~teActorCurve()
			{
			}

			TE_INLINE void OnUpdate()
			{
				if(enableAnimation)
				{
					core::teTime2 delta = core::GetTimer()->GetTime2() - startTime;
					core::teTime2 animationTime2 = core::teTime2().FromSeconds(animationTime);

					if(delta.units >= animationTime2.units)
					{
						if(((u8)curveType) & CTF_LOOPED)
							delta.units %= animationTime2.units;
						else
						{
							delta.units = animationTime2.units;
							Stop();
						}
					}

					SetT((f32)(delta.ToSeconds() / animationTime2.ToSeconds()));
				}
			}

			TE_INLINE void SetT(f32 t)
			{
				f32 v = 0.0f;

				if(t <= teRoundingError32)
					v = curveY[0];
				else if(t >= (1.0f - teRoundingError32))
				{
					v = curveY[0];
					for(u8 i = 0; i < teCurvePointsCount; ++i)
						if(curveX[i] + teRoundingError32 >= 1.0)
						{
							v = curveY[i];
							break;
						}
				}
				else
				{
					u8 p = u8Max;

					for(u8 i = 0; i < teCurvePointsCount; ++i)
						if(curveX[i] + teRoundingError32 >= t)
							break;
						else
							p = i;

					if((p == u8Max) || (p == (teCurvePointsCount - 1)))
						return;

					f32 t2 = (t - curveX[p]) / (curveX[p + 1] - curveX[p]);
					v = teLinearInterpolation(curveY[p], curveY[p + 1], t2);
				}

				if(((u8)curveType) & CTF_ADD_TO_START_VALUE)
					v += valueAtStart;

				SetLinkVariable(value, (ELinkDataType)valueType, v);
				SetValue(v);

				if((lastT <= curveTrigger) && (t >= curveTrigger))
					OnTrigger();

				lastT = t;
			}

			TE_INLINE void Start()
			{
				if(enableAnimation)
					return;

				startTime = core::GetTimer()->GetTime2();
				enableAnimation = true;
			}

			TE_INLINE void Stop()
			{
				enableAnimation = false;
			}
			
			TE_INLINE f32 GetCurveX(s32 index)
			{
				return curveX[index];
			}

			TE_INLINE void SetCurveX(s32 index, f32 value)
			{
				curveX[index] = value;
			}

			TE_INLINE f32 GetCurveY(s32 index)
			{
				return curveY[index];
			}
			
			TE_INLINE void SetCurveY(s32 index, f32 value)
			{
				curveY[index] = value;
			}

			TE_ACTOR_SIGNAL(0, SetValue)
			TE_ACTOR_SIGNAL(1, OnTrigger)

		protected:
			f32 curveX[teCurvePointsCount];
			f32 curveY[teCurvePointsCount];
			f32 curveTrigger;
			f32 curveType;
			f32 animationTime;
			void * value;

			teFastScene * scene;
			core::teTime2 startTime;
			f32 lastT;
			f32 valueAtStart;
			u8 valueType;
			u1 enableAnimation;

			TE_INLINE f32 GetCurvePoint(s8 i)
			{
				if(i < 0)
					return curveY[0] * 2.0f - curveY[1];
				else if(i >= teCurvePointsCount)
					return curveY[teCurvePointsCount - 1] * 2.0f - curveY[teCurvePointsCount - 2];
				else
					return curveY[i];
			}
		};

		TE_ACTOR_PROXY(teActorCurve)
		TE_ACTOR_SLOT_1(teActorCurve, SetT)
		TE_ACTOR_SLOT_0(teActorCurve, Start)
		TE_ACTOR_SLOT_0(teActorCurve, Stop)

		TE_FUNC void RegisterCurve(teActorsTI * ti)
		{
			ti->AddClass(sizeof(teActorCurve), "curve", TE_ACTOR_PROXY_NAMES(teActorCurve));
			for(u8 i = 0; i < teCurvePointsCount; ++i)
			{
				const c8 nameX[] = {'c', 'u', 'r', 'v', 'e', '0' + i, '.', 'x', 0};
				ti->AddData(nameX);
			}
			for(u8 i = 0; i < teCurvePointsCount; ++i)
			{
				const c8 nameY[] = {'c', 'u', 'r', 'v', 'e', '0' + i, '.', 'y', 0};
				ti->AddData(nameY);
			}
			ti->AddData("curveTrigger");
			ti->AddData("curveType");
			ti->AddData("animationTime");
			ti->AddLink("value");
			ti->AddSignal("SetValue");
			ti->AddSignal("OnTrigger");
			ti->AddSlot("SetT", TE_ACTOR_SLOT_PROXY(teActorCurve, SetT));
			ti->AddSlot("Start", TE_ACTOR_SLOT_PROXY(teActorCurve, Start));
			ti->AddSlot("Stop", TE_ACTOR_SLOT_PROXY(teActorCurve, Stop));
		}
	}
}

#endif