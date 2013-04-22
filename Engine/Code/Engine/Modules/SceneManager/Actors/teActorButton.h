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
#include "tePlatform.h"
#include "teMathTools.h"

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
				if((!surface) && (!sprite))
					return;

				u1 isNotSurface = (surface == NULL);

				u1 notInFrame = isNotSurface ? (!scene->GetAssetPack().transforms[sprite->renderAsset.transformIndex].inFrame) : (!scene->GetAssetPack().transforms[surface->renderAsset.transformIndex].inFrame);
				u1 pressedNotInFrame = isNotSurface ? ((spritePressed && (!scene->GetAssetPack().transforms[spritePressed->renderAsset.transformIndex].inFrame)) || (!spritePressed)) : true;
				u1 workIfInvisible = ((u8)flags) & BF_WORK_IF_INVISIBLE;

				u32 assetIndex = isNotSurface ? scene->GetAssetPack().sprites.GetIndexInArray(sprite) : u32Max;
				s16 assetLayer = isNotSurface ? sprite->renderAsset.layer : surface->renderAsset.layer;

				if(notInFrame && pressedNotInFrame && (!workIfInvisible))
				{
					wasPressed = false;
					wasTouchInside = false;
					return;
				}

				if((buttonCheckFrame != app::GetApplicationManager()->GetTick()) && (!IsFlag(BF_UNDETECTABLE_WORK)))
				{
					buttonCheckFrame = app::GetApplicationManager()->GetTick();

					if(buttonForceGrab)
					{
						buttonForceGrab = false;
						anyPressed = false;
					}
					else
					{
						if(!anyPressed)
							buttonGrabLayer = s16Min;
						else
							anyPressed = false;
					}
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
							OnPressed((f32)assetIndex);

							if(!IsFlag(BF_UNDETECTABLE_WORK))
								anyPressed = true;

							u1 canGrab = (((buttonGrabLayer < assetLayer) && (!buttonActiveOnlyGrabedLayer)) || ((buttonGrabLayer == assetLayer) && buttonActiveOnlyGrabedLayer));

							if(canGrab && (!IsFlag(BF_DONT_GRAB)))
								buttonGrabLayer = assetLayer;
						}
						
						touchPos = input::GetInputManager()->GetTouch(0);
					}
					else
					{
						if(wasTouchInside)
						{
							if((!IsFlag(BF_IGNORE_MOVING)) && ((touchPos - input::GetInputManager()->GetTouch(0)).GetDistance() > 16.0f))
							{
								SetStyle(false);
								wasTouchInside = false;
							}
							else
							{
								if(!IsFlag(BF_UNDETECTABLE_WORK))
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
							u1 grabOk = (buttonGrabLayer == assetLayer) || IsFlag(BF_DONT_GRAB);
							u1 grabHigher = (buttonGrabLayer > assetLayer) && IsFlag(BF_DISABLE_IF_OTHER_GRABBED);// && (!buttonActiveOnlyGrabedLayer);

							if(grabOk && (!grabHigher))
							{
								if(!IsFlag(BF_UNDETECTABLE_WORK))
									anyClicked = true;
								OnClicked((f32)assetIndex);
							}
							else
								OnNotClicked((f32)assetIndex);
						}
						else
							OnNotClicked((f32)assetIndex);

						if(wasTouchInside)
						{
							if(!IsFlag(BF_UNDETECTABLE_WORK))
								anyPressed = true;

							SetStyle(false);
							OnClear((f32)assetIndex);
						}
					}
				}
			}

			u1 IsTouchInside(const teVector2df & touchInScreenSpace) const
			{
				if((!surface) && (!sprite))
					return false;

				if(surface)
				{
					u32 cameraIndex = FindCameraThatRenderAsset(scene->GetRenderProgram(), RCT_DRAW_SURFACES, (u32)scene->GetAssetPack().surfaces.GetIndexInArray(surface));

					const teAssetCamera & camera = scene->GetAssetPack().cameras[cameraIndex];
					video::teSurfaceData * data = reinterpret_cast<video::teSurfaceData*>(scene->GetContentPack().surfaceData.At(surface->surfaceIndex));
					const video::teSurfaceLayers & layers = scene->GetContentPack().surfaceLayers[data->layersIndex];

					teMatrix4f matModel, matView, matPrj;
					matModel = scene->GetAssetPack().global[surface->renderAsset.transformIndex];
					scene->GetAssetPack().global[camera.transformIndex].Inverse(matView);
					camera.BuildProjectionMatrix(matPrj);

					u1 pressed = false;

					teVector2df touchPos(touchInScreenSpace.x, camera.viewportSize.y - touchInScreenSpace.y);

					TE_ASSERT_NODEBUG((data->operationType == video::ROT_TRIANGLES) || (data->operationType == video::ROT_TRIANGLE_STRIP));

					u32 ind[3] = {u32Max, u32Max, u32Max};

					for(u32 i = 0; i < data->indexCount; ((data->operationType == video::ROT_TRIANGLES) ? (i += 3) : (++i)))
					{
						if(data->operationType == video::ROT_TRIANGLES)
						{
							ind[0] = data->GetIndexValue(layers, i + 0);
							ind[1] = data->GetIndexValue(layers, i + 1);
							ind[2] = data->GetIndexValue(layers, i + 2);
						}
						else
						{
							if(i == 0)
							{
								ind[0] = data->GetIndexValue(layers, i + 0);
								ind[1] = data->GetIndexValue(layers, i + 1);
								ind[2] = data->GetIndexValue(layers, i + 2);

								//i = 2; // TODO with this doesnt work
							}
							else
							{
								ind[1] = ind[0];
								ind[0] = ind[2];
								ind[2] = data->GetIndexValue(layers, i);
							}
						}

						teVector3df p[3];

						for(u8 j = 0; j < 3; ++j)
						{
							teVector2df pointPrj;
							data->GetVertexValue(layers, video::SLT_POSITION, ind[j], (f32*)&p[j], 3);
							p[j] = matModel.MultiplyMatrixOnVector3D(p[j]);
							ProjectPointToViewport(scene->GetAssetPack(), cameraIndex, p[j], matView, matPrj, camera.viewportPosition, camera.viewportSize, pointPrj);
							p[j].SetXYZ(pointPrj.x, pointPrj.y, 0.0f);
						}

						if(core::IsPointInsideTriangle(touchPos, p[0].GetXY(), p[1].GetXY(), p[2].GetXY()))
						{
							pressed = true;
							break;
						}
					}

					return pressed;
				}
				else
				{
					u32 cameraIndex = FindCameraThatRenderAsset(scene->GetRenderProgram(), RCT_DRAW_SPRITES, (u32)scene->GetAssetPack().sprites.GetIndexInArray(sprite));

					teVector3df rayNear, rayFar;
					GetRayFromPointInViewport(scene->GetAssetPack(), cameraIndex, touchInScreenSpace, rayNear, rayFar);

					teAABB3df aabb;

					if(spritePressed && scene->GetAssetPack().transforms[spritePressed->renderAsset.transformIndex].visible)
						aabb = spritePressed->renderAsset.aabb;
					else
						aabb = sprite->renderAsset.aabb;

					if(!(((u8)flags) & BF_DISABLE_MINIMAL_TOUCH_ZONE))
					{
						f32 ppi = core::GetPlatform()->GetDevicePPI();

						teVector2df minimalSize;

						if(ppi < 0.0f)
							minimalSize.SetXY(0.0f, 0.0f);
						else
						{
							f32 pixelsInCm = (ppi / 2.54f) / teSqrt(2.0f); // approximately
							f32 cm = 1.0f;

							minimalSize.SetXY(pixelsInCm * cm, pixelsInCm * cm);
						}

						teVector2df size = (aabb.edgeMax - aabb.edgeMin).GetXY();
						size.SetXY(teAbs(size.x), teAbs(size.y));

						if(size.x < minimalSize.x)
						{
							aabb.edgeMax.x += (minimalSize.x - size.x) / 2.0f;
							aabb.edgeMin.x -= (minimalSize.x - size.x) / 2.0f;
						}

						if(size.y < minimalSize.y)
						{
							aabb.edgeMax.y += (minimalSize.y - size.y) / 2.0f;
							aabb.edgeMin.y -= (minimalSize.y - size.y) / 2.0f;
						}
					}

					return aabb.IsIntersectLineSegment(rayNear, rayFar);
				}
			}

			void SetAssets(teAssetSprite * setSprite, teAssetSprite * setSpritePressed = NULL, teAssetSprite * setSpritePart1 = NULL, teAssetSprite * setSpritePart2 = NULL)
			{
				sprite = setSprite;
				spritePressed = setSpritePressed;
				spritePart1 = setSpritePart1;
				spritePart2 = setSpritePart2;
			}

			void SetAssetsSurface(teAssetSurface * setSurface)
			{
				surface = setSurface;
			}

			TE_ACTOR_SIGNAL(0, OnClicked);
			TE_ACTOR_SIGNAL(1, OnPressed);
			TE_ACTOR_SIGNAL(2, OnClear);
			TE_ACTOR_SIGNAL(3, OnNotClicked); // if clicked somewhere else

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

			// use it when you need to disable all buttons for current frame
			static void ForceDisable(s16 grabOnLayer = s16Max, u1 activeOnlyThisLayer = false)
			{
				buttonForceGrab = true;
				buttonGrabLayer = grabOnLayer;
				buttonActiveOnlyGrabedLayer = activeOnlyThisLayer;
			}

			static void ClearForceDisable()
			{
				if(buttonForceGrab)
				{
					buttonForceGrab = false;
					buttonGrabLayer = s16Min;
					buttonActiveOnlyGrabedLayer = false;
				}
			}

		protected:
			f32 flags;
			teAssetSprite * sprite;
			teAssetSprite * spritePart1;
			teAssetSprite * spritePart2;
			teAssetSprite * spritePressed;
			teAssetSurface * surface;

			teFastScene * scene;
			teVector2df touchPos;
			u1 wasPressed;
			u1 wasTouchInside;

			static u1 anyClicked;
			static u1 anyPressed;
			static u1 buttonActiveOnlyGrabedLayer;
			static u1 buttonForceGrab;
			static s16 buttonGrabLayer;
			static u32 buttonCheckFrame;

			enum EButtonFlags
			{
				BF_DONT_SHADE_SPRITE			= 0x1,
				BF_WORK_IF_INVISIBLE			= 0x2,
				BF_IGNORE_MOVING				= 0x4,
				BF_DISABLE_MINIMAL_TOUCH_ZONE	= 0x8,
				BF_DONT_GRAB					= 0x10,
				BF_UNDETECTABLE_WORK			= 0x20,
				BF_DISABLE_IF_OTHER_GRABBED		= 0x40,
			};

			TE_INLINE void SetStyle(u1 pressed)
			{
				if((!surface) && (!sprite))
					return;

				if(surface)
					return;

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

			TE_INLINE u1 IsFlag(u8 flag)
			{
				return TE_GET_FLAG(((u8)flags), flag);
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
			ti->AddLink("surface");
			ti->AddSignal("OnClicked");
			ti->AddSignal("OnPressed");
			ti->AddSignal("OnClear");
			ti->AddSignal("OnNotClicked");
		}
	}
}

#endif