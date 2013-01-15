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
		const f32 teScrollMaxMove = 100.66f;
		
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
				wasPressed = 0;
				wasTouchInside = 0;

				//scrollType->vs32 = SCRL_HORIZONTAL;
				inertVec.Flush();
				moveTouch.Flush();
				slowVec.SetXY(2.0, 2.0);

				setPosOffset = 0.0f;
				needReset = false;
				
				Resize(sprite->renderAsset.aabb.edgeMin , sprite->renderAsset.aabb.edgeMax);
			}

			~teActorScroll()
			{
				
			}

			void Resize(teVector3df A, teVector3df B)
			{
				f32 width = (B.x - A.x);// * 2;
				f32 height = (B.y - A.y);// * 2;
				
				f32 elSize = elementSize->GetF32();
				f32 elCount = elementCount->GetF32();
				
				sSize.SetXY(elSize * elCount, elSize * elCount);
				
				if (width > sSize.x)
					sOrigin.x = 0.0f - (width - sSize.x)/2;
				else
					sOrigin.x = 0.0f;
				
				if (height > sSize.y)
					sOrigin.y = 0.0f + (height - sSize.y)/2;
				else
					sOrigin.y = 0.0f;

				horGap = (f32)width;
				verGap = (f32)height;
				
				limHor = ((f32)sSize.x - (f32)width) / 2;
				if (limHor < 0) limHor = 0.0f; 
				limVert = ((f32)sSize.y - (f32)height) / 2;
				if (limVert < 0) limVert = 0.0f;
				
				sizeA = teVector2df(A.x, A.y);
				sizeB = teVector2df(B.x, B.y);
				curElemCount = elementCount->GetF32();
			}

			void OnUpdate()
			{
				if(sprite->renderAsset.aabb.edgeMin.x != sizeA.x || sprite->renderAsset.aabb.edgeMin.y != sizeA.y ||
				   sprite->renderAsset.aabb.edgeMax.x != sizeB.x || sprite->renderAsset.aabb.edgeMax.y != sizeB.y ||
				   curElemCount != elementCount->GetF32())
					Resize(sprite->renderAsset.aabb.edgeMin , sprite->renderAsset.aabb.edgeMax);
				
				//Resize(teVector2df(32,0), teVector2df(992,200)); // TODO optimize this!

				//static f32 posX = sSize.x / 2.0f;
				
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
					}
				}
				else
				{
					wasPressed = 0;
					wasTouchInside = 0;
				}
				
				teAssetTransform & transform = scene->GetAssetPack().transforms[sprite->renderAsset.transformIndex];
				f32 pos2, leftX, tempX, topY, tempY;
				u32 j;

				switch (scrollType->vs32)
				{
				case SCRL_HORIZONTAL:
						
					if(needReset)
					{
						posX = sOrigin.x + limHor + setPosOffset;//sSize.x / 2.0f - elementSize->GetF32() * 2 - 130; // WARNING HARDCODE!!!
						setPosOffset = 0.0f;
						
						wasPressed = 0;
						wasTouchInside = 0;
						
						//scrollType = SCRL_HORIZONTAL;
						inertVec.Flush();
						moveTouch.Flush();
						slowVec.SetXY(2.0, 2.0);
						
						needReset = false;
					}
					
					//if(input::GetInputManager()->IsKeyDown(input::KC_L))
					//	posX += 2000.0f * TE_DT;
					//if(input::GetInputManager()->IsKeyDown(input::KC_K))
					//	posX -= 2000.0f * TE_DT;
					
					pos2 = posX - sSize.x / 2.0f + elementSize->GetF32()/2;
					
					leftX = -elementSize->GetF32()*2 - elementSize->GetF32()/2 ;
					tempX = pos2;
					
					j = 0;
					
					for(u32 i = 0; i < elementCount->GetS32(); ++i)
					{
						if((tempX > leftX) && (j < cacheCount->GetS32()))
						{
							UpdateElement(j, i, tempX);
							++j;
						}
						
						tempX += elementSize->GetF32();
					}
					
					if(j < cacheCount->GetS32())
					{
						for(u32 k = j; k < cacheCount->GetS32(); ++k)
						{
							UpdateElement(k, 0, -5000.0f);
						}
					}
						
					//-------- reaction on touches
					if (wasTouchInside)
						if (wasPressed)
						{
							if (teAbs(posX - sOrigin.x) > limHor)
								moveTouch = (cur_touch - prevTouch) * (1 - f32(teAbs(posX - sOrigin.x) - limHor)/horGap);
							else
								moveTouch = cur_touch - prevTouch;
							
							//----- limit for speed !
							//						if (teAbs(moveTouch.x) > teScrollMaxMove)
							//							moveTouch.x = teScrollMaxMove * teSign(moveTouch.x);
							//						
							//						if (teAbs(moveTouch.y) > teScrollMaxMove)
							//							moveTouch.y = teScrollMaxMove * teSign(moveTouch.y);
							
							inertVec.Flush();
							prevTouch = cur_touch;
						}	
						
						
					if (posX < sOrigin.x) moveSig.x = -1;
					else moveSig.x = 1;
					
					// ----- calc inert
					if ((teAbs(posX - sOrigin.x) > limHor + 0.5f ) && !wasTouchInside)
					{
						inertVec.x = (f32(teAbs(posX - sOrigin.x) - limHor ))/horGap * (-moveSig.x) * teScrollInert * TE_DT;
						//inertVec.x = 0.0f;
					}
					else
					{
						inertVec.x = 0.0f;
					}
	
					// ----- slowing down
					if (teAbs(moveTouch.x) > slowVec.x )
					{
						if (teAbs(posX - sOrigin.x) > limHor)
							moveTouch.x *= (teScrollSlow + (1 - teScrollSlow) * TE_DT) / 2; //(teScrollSlow/2);
						else	
							moveTouch.x *= (teScrollSlow + (1 - teScrollSlow) * TE_DT); //(teScrollSlow);
					}
					else
					{
						moveTouch.x = 0.0f;
					}
						
					//------ translate
						posX += (moveTouch.x + inertVec.x);
						
					if (teAbs(posX - sOrigin.x) > limHor + horGap)
					{
						//transform.position.x = teAbs(transform.position.x)/transform.position.x * (limHor + horGap);
						posX = teAbs(posX)/posX * (limHor + horGap);
						moveTouch.Flush();
					}
					
					break;
							
				case SCRL_VERTICAL:
						if(needReset)
						{
							posY = sOrigin.y + limVert + setPosOffset;
							setPosOffset = 0.0f;
							
							wasPressed = 0;
							wasTouchInside = 0;

							inertVec.Flush();
							moveTouch.Flush();
							slowVec.SetXY(2.0, 2.0);
							
							needReset = false;
						}
						
						pos2 = posY - sSize.y / 2.0f + elementSize->GetF32()/2;
						
						topY = -elementSize->GetF32()*2 - elementSize->GetF32()/2 ;
						tempY = pos2;
						
						j = 0;
						
						for(u32 i = 0; i < elementCount->GetS32(); ++i)
						{
							if((tempY > topY) && (j < cacheCount->GetS32()))
							{
								UpdateElement(j, i, tempY);
								++j;
							}
							
							tempY += elementSize->GetF32();
						}
						
						if(j < cacheCount->GetS32())
						{
							for(u32 k = j; k < cacheCount->GetS32(); ++k)
							{
								UpdateElement(k, 0, -5000.0f);
							}
						}
						
						
						//-------- reaction on touches
						if (wasTouchInside)
							if (wasPressed)
							{
								if (teAbs(posY - sOrigin.y) > limVert)
									moveTouch = (cur_touch - prevTouch) * (1 - f32(teAbs(posY - sOrigin.y) - limVert)/verGap);
								else
									moveTouch = cur_touch - prevTouch;
								
								//----- limit for speed !
								//						if (teAbs(moveTouch.x) > teScrollMaxMove)
								//							moveTouch.x = teScrollMaxMove * teSign(moveTouch.x);
								//						
								//						if (teAbs(moveTouch.y) > teScrollMaxMove)
								//							moveTouch.y = teScrollMaxMove * teSign(moveTouch.y);
								
								inertVec.Flush();
								prevTouch = cur_touch;
							}	
						
						if (posY < sOrigin.y) moveSig.y = -1;
						else moveSig.y = 1;
						
						// ----- calc inert
						if ((teAbs(posY - sOrigin.y) > limVert + 0.5f ) && !wasTouchInside)
						{
							inertVec.y = (f32(teAbs(posY - sOrigin.y) - limVert ))/verGap * (-moveSig.y) * teScrollInert * TE_DT;

						}
						else
						{
							inertVec.y = 0.0f;
						}
						
						// ----- slowing down
						if (teAbs(moveTouch.y) > slowVec.y )
						{
							if (teAbs(posY - sOrigin.y) > limVert)
								moveTouch.y *= (teScrollSlow + (1 - teScrollSlow) * TE_DT) / 2; //(teScrollSlow/2);
							else	
								moveTouch.y *= (teScrollSlow + (1 - teScrollSlow) * TE_DT); //(teScrollSlow);
						}
						else
						{
							moveTouch.y = 0.0f;
						}
						
						//------ translate
						posY += (-moveTouch.y + inertVec.y);
						
						if (teAbs(posY - sOrigin.y) > limVert + verGap)
						{
							//transform.position.x = teAbs(transform.position.x)/transform.position.x * (limHor + horGap);
							posY = teAbs(posY)/posY * (limVert + verGap);
							moveTouch.Flush();
						}

					break;
						
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

			TE_ACTOR_SIGNAL(0, UpdateElement)

		protected:
			teAssetSprite * sprite;
			teAssetVariable * elementCount;
			teAssetVariable * elementSize;
			teAssetVariable * cacheCount;
			teAssetVariable * scrollType;
			
			teFastScene * scene;

			teVector2df sOrigin; // sprite Origin
			teVector2duh sSize; // sprite Size;
			
			teVector2df sizeA, sizeB; // AABB
			u32 curElemCount;
						
			f32 limHor, limVert; // scrolling limit 
			f32 horGap, verGap; // Scrolling gap
			f32 posX, posY;
			
			f32 position;

			f32 setPosOffset;
			
			teVector2dsh moveSig; // signum of moving
			
			u1 wasPressed;
			u1 wasTouchInside;
			u1 needReset;
			
			teVector2df prevTouch, moveTouch, cur_touch; //cursor
			teVector2df slowVec, inertVec;

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