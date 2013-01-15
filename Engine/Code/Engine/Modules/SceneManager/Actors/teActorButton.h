/*
 *  teActorButton.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 03/24/12.
 *  Copyright 2012 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEACTORBUTTON_H
#define TE_TEACTORBUTTON_H

#include "teActor.h"
#include "teInputManager.h"
#include "teAssetTools.h"
#include "teApplicationManager.h"

namespace te
{
	namespace scene
	{
		class teActorButton
		{
		public:
			TE_INLINE teActorButton(teFastScene * setScene)
				:scene(setScene), wasPressed(false), wasTouchInside(false)
			{
			}

			TE_INLINE ~teActorButton()
			{
			}

			TE_INLINE void OnUpdate()
			{
				u1 spriteNotInFrame = !scene->GetAssetPack().transforms[sprite->renderAsset.transformIndex].inFrame;
				u1 spritePressedNotInFrame = (spritePressed && (!scene->GetAssetPack().transforms[spritePressed->renderAsset.transformIndex].inFrame)) || (!spritePressed);
				u1 workIfInvisible = ((u8)flags) & BF_WORK_IF_INVISIBLE;

				u32 spriteIndex = scene->GetAssetPack().sprites.GetIndexInArray(sprite);

				if(spriteNotInFrame && spritePressedNotInFrame && (!workIfInvisible))
				{
					wasPressed = false;
					wasTouchInside = false;
					return;
				}

				if(buttonCheckFrame != app::GetApplicationManager()->GetTick())
				{
					buttonCheckFrame = app::GetApplicationManager()->GetTick();

					if(!anyPressed)
						buttonGrabLayer = s16Min;
					else
						anyPressed = false;
				}

				if(input::GetInputManager()->GetTouchesCount())
				{
					if(!wasPressed)
					{
						wasPressed = true;
						wasTouchInside = IsTouchInside(input::GetInputManager()->GetTouch(0));

						if(wasTouchInside)
						{
							SetStyle(true);
							OnPressed((f32)spriteIndex);

							anyPressed = true;

							if(buttonGrabLayer < sprite->renderAsset.layer)
								buttonGrabLayer = sprite->renderAsset.layer;
						}
						
						touchPos = input::GetInputManager()->GetTouch(0);
					}
					else
					{
						if(wasTouchInside)
						{
							if((!(((u8)flags) & BF_IGNORE_MOVING)) && ((touchPos - input::GetInputManager()->GetTouch(0)).GetDistance() > 2.0f))
							{
								SetStyle(false);
								wasTouchInside = false;
							}
							else
							{
								anyPressed = true;
							}
						}
					}
				}
				else
				{
					if(wasPressed)
					{
						wasPressed = false;

						if(IsTouchInside(input::GetInputManager()->GetTouch(0)) && wasTouchInside)
						{
							if(buttonGrabLayer == sprite->renderAsset.layer)
							{
								anyClicked = true;
								OnClicked((f32)spriteIndex);
							}
						}

						if(wasTouchInside)
						{
							anyPressed = true;

							SetStyle(false);
							OnClear((f32)spriteIndex);
						}
					}
				}
			}

			u1 IsTouchInside(const teVector2df & touchInScreenSpace) const
			{
				u32 cameraIndex = FindCameraThatRenderAsset(scene->GetRenderProgram(), RCT_DRAW_SPRITES, (u32)scene->GetAssetPack().sprites.GetIndexInArray(sprite));

				teVector3df rayNear, rayFar;
				GetRayFromPointInViewport(scene->GetAssetPack(), cameraIndex, touchInScreenSpace, rayNear, rayFar);

				if(spritePressed && scene->GetAssetPack().transforms[spritePressed->renderAsset.transformIndex].visible)
					return spritePressed->renderAsset.aabb.IsIntersectLineSegment(rayNear, rayFar);
				else
					return sprite->renderAsset.aabb.IsIntersectLineSegment(rayNear, rayFar);
			}

			TE_ACTOR_SIGNAL(0, OnClicked);
			TE_ACTOR_SIGNAL(1, OnPressed);
			TE_ACTOR_SIGNAL(2, OnClear);

			// check if any button was clicked between method calls
			static u1 IsAnyClicked()
			{
				u1 result = anyClicked;
				anyClicked = false;
				return result;
			}

			static u1 IsAnyGrab()
			{
				return buttonGrabLayer != s16Min;
			}

		protected:
			f32 flags;
			teAssetSprite * sprite;
			teAssetSprite * spritePart1;
			teAssetSprite * spritePart2;
			teAssetSprite * spritePressed;

			teFastScene * scene;
			teVector2df touchPos;
			u1 wasPressed;
			u1 wasTouchInside;

			static u1 anyClicked;
			static u1 anyPressed;
			static s16 buttonGrabLayer;
			static u32 buttonCheckFrame;

			enum EButtonFlags
			{
				BF_DONT_SHADE_SPRITE	= 0x1,
				BF_WORK_IF_INVISIBLE	= 0x2,
				BF_IGNORE_MOVING		= 0x4,
			};

			TE_INLINE void SetStyle(u1 pressed)
			{
				if(spritePressed)
				{
					scene->GetAssetPack().transforms[spritePressed->renderAsset.transformIndex].visible = pressed;
					scene->GetAssetPack().transforms[sprite->renderAsset.transformIndex].visible = !pressed;
				}
				else
				{
					if(!(((u8)flags) & BF_DONT_SHADE_SPRITE))
					{
						if(pressed)
						{
							sprite->color.SetRGBA(200, 200, 200, 255);
							if(spritePart1) spritePart1->color.SetRGBA(200, 200, 200, 255);
							if(spritePart2) spritePart2->color.SetRGBA(200, 200, 200, 255);
						}
						else
						{
							sprite->color.SetRGBA(255, 255, 255, 255);
							if(spritePart1) spritePart1->color.SetRGBA(255, 255, 255, 255);
							if(spritePart2) spritePart2->color.SetRGBA(255, 255, 255, 255);
						}
					}
				}
			}
		};

		TE_ACTOR_PROXY(teActorButton);

		TE_FUNC void RegisterButton(teActorsTI * ti)
		{
			ti->AddClass(sizeof(teActorButton), "button", TE_ACTOR_PROXY_NAMES(teActorButton));
			ti->AddData("flags");
			ti->AddLink("sprite", false, "sprite");
			ti->AddLink("spritePart1", true, "sprite");
			ti->AddLink("spritePart2", true, "sprite");
			ti->AddLink("spritePressed");
			ti->AddSignal("OnClicked");
			ti->AddSignal("OnPressed");
			ti->AddSignal("OnClear");
		}
	}
}

#endif