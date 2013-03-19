/*
 *  teActorScroll.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 02/25/12.
 *  Copyright 2012 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEACTORSCROLL_H
#define TE_TEACTORSCROLL_H

#include "teActor.h"
#include "teFastScene.h"
#include "teInputManager.h"
#include "teFpsCounter.h"

namespace te
{
	namespace scene
	{
		const f32 teScrollInert = 3200.0f;
		const f32 teScrollSlow = 0.93f;
		const u8 teScrollMaxMoves = 3;
		
		enum EScrollingType
		{
			SCRL_HORIZONTAL = 0,
			SCRL_VERTICAL,
			SCRL_BOTH 
		};

		class teActorScroll
		{
		public:
			teActorScroll(teFastScene * setScene)
				:scene(setScene)
			{
				Reset();
				
				Resize(sprite->renderAsset.aabb.edgeMin, sprite->renderAsset.aabb.edgeMax);
			}

			~teActorScroll()
			{
				
			}

			void Resize(teVector3df A, teVector3df B)
			{
				teVector2df koef = teActorViewportSizeWatcher::GetSizeRootScale();
				
				sizeA = teVector2df(A.x, A.y) / koef;
				sizeB = teVector2df(B.x, B.y) / koef;
				curElemCount = elementCount->GetF32();
				
				f32 width = (B.x - A.x)  / koef.x;
				f32 height = (B.y - A.y) / koef.y;
				
				f32 elSize = elementSize->GetF32();
				f32 elCount = elementCount->GetF32();
				
				sSize = elSize * elCount;
				
				switch (scrollType->vs32)
				{
					case SCRL_HORIZONTAL:
						if (width > sSize)
							sOrigin = 0.0f - (width - sSize)/2;
						else
							sOrigin = 0.0f;
						
						scrollLim = (sSize - width) / 2.0f;
						if (scrollLim < 0.0f)
							scrollLim = 0.0f;
						
						scrollEdge = -(sizeB.x - sizeA.x) / 2.0f - elementSize->GetF32() / 2.0f;
						
						break;
						
					case SCRL_VERTICAL:
						if (height > sSize)
							sOrigin = 0.0f + (height - sSize)/2;
						else
							sOrigin = 0.0f;
						
						scrollLim = (sSize - height) / 2.0f;
						if (scrollLim < 0.0f)
							scrollLim = 0.0f;
						
						scrollEdge = -teAbs(sizeB.y - sizeA.y) / 2.0f - elementSize->GetF32() / 2.0f;
						
						break;
				}
				
				scrollGap = elSize;
				
				SetSnapPos();
			}

			void OnUpdate()
			{
				if(sprite->renderAsset.aabb.edgeMin.x != sizeA.x || sprite->renderAsset.aabb.edgeMin.y != sizeA.y ||
				   sprite->renderAsset.aabb.edgeMax.x != sizeB.x || sprite->renderAsset.aabb.edgeMax.y != sizeB.y ||
				   curElemCount != elementCount->GetF32())
					Resize(sprite->renderAsset.aabb.edgeMin , sprite->renderAsset.aabb.edgeMax);
				
				//------- check for touches
				if(input::GetInputManager()->GetTouchesCount())
				{
					wasPressed = 1;
					cur_touch = input::GetInputManager()->GetTouch(0);
					if (IsTouchInside(cur_touch))
					{
						if (!wasTouchInside)
						{
							wasTouchInside = 1;
							prevTouch = cur_touch;
						}
						
						StartMoving();
					}
				}
				else
				{
					wasPressed = 0;
					wasTouchInside = 0;
				}
				
				teAssetTransform & transform = scene->GetAssetPack().transforms[sprite->renderAsset.transformIndex];
				
				if(needReset)
					Reset();
				
				CalcCache();
				
				f32 overLimitSize = teAbs(position - sOrigin) - scrollLim;
				u1 isOverLimit = (overLimitSize > 0.0f);
				
				//-------- reaction on touches
				if(wasTouchInside)
				{
					if(wasPressed)
					{
						if(isOverLimit)
							moveTouch = (cur_touch - prevTouch) * (1.0f - overLimitSize / scrollGap);
						else
							moveTouch = cur_touch - prevTouch;
						
						inert = 0.0f;
						prevTouch = cur_touch;
						
						touchMoves[curTouchMove] = moveTouch;
						++curTouchMove;
						if(curTouchMove >= teScrollMaxMoves)
							curTouchMove = 0;
						if(touchMovesCount < curTouchMove)
							touchMovesCount = curTouchMove;
						
						StartMoving();
					}
				}
				else
				{
					if(touchMovesCount)
					{
						teVector2df sumVec = teVector2df(0.0f, 0.0f);
						for(int i = 0; i < touchMovesCount; ++i)
							sumVec += touchMoves[i];
						moveTouch = sumVec / touchMovesCount;
						
						touchMovesCount = 0;
						
						StartMoving();
					}
				}
				
				// ----- calc inert (return force)
				f32 moveSig;
				if(position < sOrigin) moveSig = 1.0f;
				else moveSig = -1.0f;
				
				if(scrollType->vs32 == SCRL_VERTICAL)
					moveSig *= -1.0f;
				
				if(isOverLimit && !wasTouchInside)
				{
					inert = overLimitSize / scrollGap * moveSig * teScrollInert * TE_DT;
					StartMoving();
				}
				else
				{
					inert = 0.0f;
				}
				
				// ----- slowing down
				if(!wasPressed)
				{
					if(isOverLimit)
						moveTouch *= teScrollSlow / 2.0f;
					else
						moveTouch *= teScrollSlow;
				}
				else
				{
					if(isOverLimit)
						moveTouch *= (1.0f - overLimitSize / scrollGap);
				}
				
				//------ translate
				if(isMoving)
				{
					f32 movingCount;
					
					switch (scrollType->vs32)
					{
						case SCRL_HORIZONTAL:
							movingCount = moveTouch.x + inert;
							break;
					
						case SCRL_VERTICAL:
							movingCount = -(moveTouch.y + inert);
							break;
					}
					
					if(teAbs(movingCount) > 1.0f)
						position += movingCount;
					else
						StopMoving();
				}
				else
				{
					if(isSnapping)
					{
						float snapDist = snapPos - position;
						float snapCount = 4.0f * snapDist * TE_DT;
						if(teAbs(snapCount) < 1.0f)
							snapCount = 1.0f * teSign(snapDist);
						
						if((teAbs(snapDist) <= teAbs(snapCount)) || teAbs(snapDist) < 1.0f)
						{
							position = snapPos;
							isSnapping = false;
						}
						else
						{
							position += snapCount;
						}
					}
				}
				
				if (teAbs(position - sOrigin) > scrollLim + scrollGap)
				{
					if(teAbs(position) > 0.0f)
						position = teAbs(position) / position * (scrollLim /*+ scrollGap*/);
					moveTouch.Flush();
					StopMoving();
				}
			}
			
			TE_INLINE void SetSnapPos()
			{
				f32 spriteSize = 0.0f;
				switch (scrollType->vs32)
				{
					case SCRL_HORIZONTAL:
						spriteSize = sizeB.x - sizeA.x;
						break;
						
					case SCRL_VERTICAL:
						spriteSize = sizeB.y - sizeA.y;
						break;
				}
				
				f32 tNom = sSize / 2.0f - spriteSize + spriteSize / 2.0f;
				
				f32 tfirst = (tNom - position) / elementSize->GetF32();
				s32 intFirst = s32(tfirst);
				if(tfirst - (f32)intFirst >= 0.5f)
					++intFirst;
				
				snapPos = tNom - (f32)intFirst * elementSize->GetF32();
				
				isSnapping = true;
			}
			
			TE_INLINE void Reset()
			{
				if(scrollType->vs32 == SCRL_HORIZONTAL)
					position = sOrigin + scrollLim + setPosOffset;
				else if(scrollType->vs32 == SCRL_VERTICAL)
					position = sOrigin + scrollLim + setPosOffset;
				
				setPosOffset = 0.0f;
				
				wasPressed = 0;
				wasTouchInside = 0;
				
				inert = 0.0f;
				moveTouch.Flush();
				slow = 2.0f;
				
				needReset = false;
				
				touchMovesCount = 0;
				curTouchMove = 0;
				isMoving = 0;
				
				SetSnapPos();
			}
			
			TE_INLINE void CalcCache()
			{
				f32 limVar, casheVar;
				
				limVar = position - sSize / 2.0f + elementSize->GetF32() / 2.0f;
				
				casheVar = 0;
				
				for(u32 i = 0; i < elementCount->GetS32(); ++i)
				{
					if((limVar > scrollEdge) && (casheVar < cacheCount->GetS32()))
					{
						UpdateElement(casheVar, i, limVar);
						++casheVar;
					}
					
					limVar += elementSize->GetF32();
				}
				
				if(casheVar < cacheCount->GetS32())
				{
					for(u32 k = casheVar; k < cacheCount->GetS32(); ++k)
					{
						UpdateElement(k, 0, -5000.0f);
					}
				}
			}

			u1 IsTouchInside(const teVector2df & touchInScreenSpace) const
			{
				u32 cameraIndex = FindCameraThatRenderAsset(scene->GetRenderProgram(), RCT_DRAW_SPRITES, scene->GetAssetPack().sprites.GetIndexInArray(sprite));

				teVector3df rayNear, rayFar;
				GetRayFromPointInViewport(scene->GetAssetPack(), cameraIndex, touchInScreenSpace, rayNear, rayFar);

				return sprite->renderAsset.aabb.IsIntersectLineSegment(rayNear, rayFar);
			}

			/*
			void SetScrollType(EScrollingType srclType)
			{
				scrollType = srclType;
			}
			*/
			
			void SetPosition(f32 value){position = value;};
			f32 GetPosition(){return position;};

			TE_INLINE void ResetScroll()
			{
				needReset = true;
			}
			
			TE_INLINE void SetScrollType(EScrollingType scrollType_)
			{
				scrollType->vs32 = scrollType_;
			}
			
			TE_INLINE void SetScrollPosition(f32 x)
			{
				setPosOffset = x;
				needReset = true;
			}
			
			TE_INLINE void StartMoving()
			{
				isMoving = true;
				isSnapping = false;
			}
			
			TE_INLINE void StopMoving()
			{
				isMoving = false;
				
				SetSnapPos();
			}
			
			TE_ACTOR_SIGNAL(0, UpdateElement)

		protected:
			teAssetSprite * sprite;
			teAssetVariable * elementCount;
			teAssetVariable * elementSize;
			teAssetVariable * cacheCount;
			teAssetVariable * scrollType;
			
			teFastScene * scene;

			f32 sOrigin; // scroll sprite Origin
			f32 sSize; // total size of scrolling ellements;
			
			teVector2df sizeA, sizeB; // AABB
			u32 curElemCount;
						
			f32 scrollLim;//limHor, limVert; // scrolling limit
			f32 scrollGap;//horGap, verGap; // Scrolling gap
			f32 scrollEdge; //--- edge for visible ellements
			f32 position;
			f32 snapPos;

			f32 setPosOffset;
			
			u1 wasPressed;
			u1 wasTouchInside;
			u1 needReset;
			u1 isMoving;
			u1 isSnapping;
			
			u8 touchMovesCount; // --- for filtering touch inertion
			u8 curTouchMove;
			
			u8 reserved_;
			
			teVector2df prevTouch, moveTouch, cur_touch; //cursor
			teVector2df touchMoves[teScrollMaxMoves];
			
			f32 slow, inert;
		};
		
		TE_ACTOR_PROXY(teActorScroll);
		//TE_ACTOR_SLOT_1(teActorScroll, SwitchTo);
		TE_ACTOR_SLOT_0(teActorScroll, ResetScroll);
		TE_ACTOR_SLOT_1(teActorScroll, SetScrollPosition);
		
		TE_FUNC void RegisterScroll(teActorsTI * ti)
		{
			ti->AddClass(sizeof(teActorScroll), "scroll", TE_ACTOR_PROXY_NAMES(teActorScroll));
			ti->AddLink("sprite");
			ti->AddLink("elementCount");
			ti->AddLink("elementSize");
			ti->AddLink("cacheCount");
			ti->AddLink("scrollType");
			
			ti->AddSignal("UpdateElement");
			ti->AddSlot("ResetScroll", TE_ACTOR_SLOT_PROXY(teActorScroll, ResetScroll));
			ti->AddSlot("SetScrollPosition", TE_ACTOR_SLOT_PROXY(teActorScroll, SetScrollPosition));
			//ti->AddSlot("SwitchTo", TE_ACTOR_SLOT_PROXY(teActorScroll, SwitchTo));
		}
	}
}

#endif