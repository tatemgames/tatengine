/*
 *  teFastScene.cpp
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 12/1/11.
 *  Copyright 2011 Tatem Games. All rights reserved.
 *
 */

#include "teFastScene.h"
#include "teRender.h"
#include "teFileManager.h"
#include "teString.h"
#ifdef TE_PLATFORM_IPHONE
#include "teApplicationManager.h"
#endif

#include "teActorAbstractPlayerInput.h"
#include "teActorSpriteAnimation.h"
#include "teActorSkeletonAnimation.h"
#include "teActorSwitchScene.h"
#include "teActorCombineArguments.h"
#include "teActorViewportSizeWatcher.h"
#include "teActorAlign.h"
#include "teActorButton.h"
#include "teActorScroll.h"
#include "teActorCurve.h"
#include "teActorTimer.h"
#include "teActorVideoPlayer.h"

#include "teFrustum.h"

#ifdef TE_MODULE_SCRIPTING
	#include "teActorScriptLua.h"
#endif

#ifdef TE_MODULE_PUBLISHING
	#include "teActorPublishing.h"
#endif

namespace te
{
	namespace scene
	{
		u1 teActorButton::anyClicked = false;
		u1 teActorButton::anyPressed = false;
		u1 teActorButton::buttonActiveOnlyGrabedLayer = false;
		u1 teActorButton::buttonForceGrab = false;
		s16 teActorButton::buttonGrabLayer = s16Min;
		u32 teActorButton::buttonCheckFrame = 0;
		teActorButton::teActorButtonCallback teActorButton::buttonCallback = NULL;
		teVector2df teActorViewportSizeWatcher::sizeRootScale(1.0f, 1.0f);
		u1 teActorViewportSizeWatcher::firstFrame = true;

		#ifdef TE_MODULE_SCRIPTING
			u32 teActorScriptLua::metaIdCounter = 0;
			u8 teActorScriptLua::metaIdCounterLastStage = 0;
			lua_State * teActorScriptLua::L = NULL;
			u32 teActorScriptLua::stateUsageCount = 0;
			#ifdef TE_DEBUG
				teActorScriptLua::teLuaMemoryUsage teActorScriptLua::memoryUsage;
			#endif
			#ifdef TE_LUA_SPECIAL_ALLOC
				teLuaAllocator teActorScriptLua::luaSpecAlloc;
			#endif
		#endif

		teFastScene::teFastScene(teRegisterActorsCallback registerActorsCallback)
			:needToUpdateRenderProgram(false), debugFlags(0)
		{
			stageLoaded = u8Max;
			stageWaitForLoading = u8Max;

			for(u8 i = 0; i < 2; ++i)
			{
				actorsTI.SetCalculateSizeMode(i == 0);

				RegisterSpriteAnimation(&actorsTI);
				RegisterSkeletonAnimation(&actorsTI);
				RegisterAbstractPlayerInput(&actorsTI);
				RegisterSwitchScene(&actorsTI);
				RegisterCombineArguments(&actorsTI);
				RegisterViewportSizeWatcher(&actorsTI);
				RegisterAlign(&actorsTI);
				RegisterButton(&actorsTI);
				RegisterScroll(&actorsTI);
				RegisterCurve(&actorsTI);
				RegisterTimer(&actorsTI);
				RegisterVideoPlayer(&actorsTI);

				#ifdef TE_MODULE_SCRIPTING
					RegisterScriptLua(&actorsTI);
				#endif

				#ifdef TE_MODULE_PUBLISHING
					RegisterPublishing(&actorsTI);
				#endif

				if(registerActorsCallback)
					(*registerActorsCallback)(&actorsTI);
			}

			// ---------------------------------------------------------
			#ifdef TE_CTTI

			teStringConcate cttiPath;
			teStringPool poolJSON(256);
			cttiPath.SetBuffer(poolJSON.Allocate(poolJSON.GetSize()));
			cttiPath.Add(core::GetPlatform()->GetFileSystem()->GetPath(core::FPT_BIN).c_str());
			cttiPath.Add("ctti.json");

			core::IBuffer * cttiJSONBuffer = core::GetFileManager()->OpenFile(cttiPath.BakeToString().c_str(), core::CFileBuffer::FWM_WRITE, false, false);

			if(cttiJSONBuffer)
			{
				actorsTI.CTTIToJSON(cttiJSONBuffer);
				TE_SAFE_DROP(cttiJSONBuffer);
			}

			cttiPath.Add(core::GetPlatform()->GetFileSystem()->GetPath(core::FPT_BIN).c_str());
			cttiPath.Add("ctti.bin");

			core::IBuffer * cttiBinBuffer = core::GetFileManager()->OpenFile(cttiPath.BakeToString().c_str(), core::CFileBuffer::FWM_WRITE, false, false);

			if(cttiBinBuffer)
			{
				actorsTI.Save(cttiBinBuffer);
				TE_SAFE_DROP(cttiBinBuffer);
			}
			else
				actorsTI.Save(NULL); // TODO rewrite this, force crc calculation

			#endif

			// ---------------------------------------------------------

			batchs.Reserve(maxBatchs);

			#if defined(TE_RENDER_GL) || defined(TE_RENDER_D3D9)
				batchData = (u8*)TE_ALLOCATE(maxBatchs * (sizeof(video::teSurfaceData) + teRenderBatchSize + 8));
			#endif

			for(u32 i = 0; i < batchs.GetSize(); ++i)
			{
				u32 temp = (sizeof(video::teSurfaceData) + teRenderBatchSize + 8) * i;
				u32 & batch = *batchs.Request();
				batch = TE_ALIGN_PTR(temp, 4) + 0; // TODO HACK, make offset of data in teSurfaceData aligned to 4

				video::teSurfaceData * data = reinterpret_cast<video::teSurfaceData*>(batchData + batch);
				data->Clear();
				data->dataSize = teRenderBatchSize;
			}
		}

		teFastScene::~teFastScene()
		{
			scenePack.FreeActors();

			#if defined(TE_RENDER_GL) || defined(TE_RENDER_D3D9)
				TE_FREE(batchData);
			#endif
		}

		void teFastScene::OnUpdate()
		{
			TE_TIME_BEGIN(timeUpdate)

			// --------------------------------------------------------------- update actors

			TE_TIME_BEGIN(timeActors)

			scenePack.UpdateActors();

			TE_TIME_END(timeActors)

			// --------------------------------------------------------------- update transforms

			TE_TIME_BEGIN(timeTransforms)

			for(u32 i = 0; i < scenePack.transforms.GetAlive(); ++i)
			{
				if(memcmp(&scenePack.transformsChangesBuffer[i], &scenePack.transforms[i], sizeof(teAssetTransform)) != 0)
				{
					memcpy(&scenePack.transformsChangesBuffer[i], &scenePack.transforms[i], sizeof(teAssetTransform));
					CalculateTransformGlobalMatrix(scenePack, i);
					scenePack.transformsChangesFlags[i] = 1;
				}
				else
				{
					if((scenePack.transforms[i].parent != u32Max) && (scenePack.transformsChangesFlags[scenePack.transforms[i].parent]))
					{
						CalculateTransformGlobalMatrix(scenePack, i);
						scenePack.transformsChangesFlags[i] = 1;
					}
					else
					{
						scenePack.transformsChangesFlags[i] = 0;
					}
				}
			}

			TE_TIME_END(timeTransforms)

			// --------------------------------------------------------------- update aabb

			UpdateAABBs();

			// --------------------------------------------------------------- update render program

			//UpdateRenderProgram(program, scenePack, contentPack);

			if(needToUpdateRenderProgram)
			{
				FormRenderProgram(program, scenePack, contentPack, &programSortBufferKeys);
				needToUpdateRenderProgram = false;
			}

			// --------------------------------------------------------------- deferred stage loading

			if(stageWaitForLoading != u8Max)
			{
				Load(stageWaitForLoading);
				stageWaitForLoading = u8Max;
			}

			TE_TIME_END(timeUpdate)

			statistic.timeActors = (f32)timeActors.ToMilliSeconds();
			statistic.timeTransforms = (f32)timeTransforms.ToMilliSeconds();
			statistic.timeUpdate = (f32)timeUpdate.ToMilliSeconds();
		}

		void teFastScene::OnRender()
		{
			TE_TIME_BEGIN(timeRender)

			statistic.ClearRender();

			ERenderCommandType lastCommand = RCT_INVALID;

			video::teSurfaceData * batch = NULL;

			teMatrix4f matView;
			matView.SetIdentity();

			u8 currentCameraType = CT_PERSPECTIVE;

			video::teFrameBuffer * frameBuffer = NULL;

			// wip
			teFrustum frustum;

			for(u32 i = 0; i < program.GetAlive(); ++i)
			{
				teRenderCommand & command = program[i];

				u1 lastWasBatched = (lastCommand == RCT_DRAW_SPRITES) || (lastCommand == RCT_DRAW_TEXT);
				u1 needBatched = (command.type == RCT_DRAW_SPRITES) || (command.type == RCT_DRAW_TEXT);

				u1 skipBatchClear = lastWasBatched && needBatched;

				if((lastCommand != command.type) && batch) // invalidate batch
				{
					if(!skipBatchClear)
					{
						if(!batch->IsEmpty())
							RenderBatch(batch);

						batch = NULL;
					}
				}

				switch(command.type)
				{
				case RCT_BEGIN:
				case RCT_END:
					break;

				case RCT_BEGIN_TRANSLUCENT:
				case RCT_END_TRANSLUCENT:
					break;

				case RCT_SET_CAMERA:
					{
						scenePack.global[scenePack.cameras[command.from].transformIndex].Inverse(matView);

						// TODO setup viewport from window or etc
						//scenePack.cameras[command.from].SetViewportDefault();

						if(frameBuffer)
						{
							teVector2duh frameBufferSize;

							for(u32 j = 0; j < video::FBL_MAX; ++j)
								if(frameBuffer->textures[j])
								{
									frameBufferSize = frameBuffer->textures[j]->size;
									break;
								}

							scenePack.cameras[command.from].viewportPosition.SetXY(0, 0);
							scenePack.cameras[command.from].viewportSize = frameBufferSize;
						}
						else
						{
							scenePack.cameras[command.from].viewportPosition.SetXY(0, 0);
							scenePack.cameras[command.from].viewportSize = video::GetRender()->GetContext()->GetSize();
						}

						scenePack.cameras[command.from].aspect = (f32)scenePack.cameras[command.from].viewportSize.x / (f32)scenePack.cameras[command.from].viewportSize.y;

						if(scenePack.cameras[command.from].cameraType == CT_ORTHOGRAPHIC)
							scenePack.cameras[command.from].fov = (f32)scenePack.cameras[command.from].viewportSize.y;

						scenePack.cameras[command.from].BuildProjectionMatrix(video::GetRender()->GetMatrixProjection());

						video::GetRender()->SetViewportOptions(video::teViewport(scenePack.cameras[command.from].viewportSize, scenePack.cameras[command.from].viewportPosition));
						video::GetRender()->GetMatrixView() = matView;

						currentCameraType = scenePack.cameras[command.from].cameraType;

						// wip
						frustum.CalculateFrustum(video::GetRender()->GetMatrixProjection(), matView);

						break;
					}
				case RCT_SET_FRAMEBUFFER:
					{
						if(command.from != u32Max)
						{
							frameBuffer = contentPack.frameBuffers.At(command.from);
							frameBuffer->Bind();
						}
						else if(frameBuffer)
						{
							frameBuffer->UnBind();
							frameBuffer = NULL;

							#ifdef TE_PLATFORM_IPHONE
								app::GetApplicationManager()->GetFrameBuffer()->Bind();
							#endif
						}

						break;
					}
				case RCT_SET_PASS:
					{
						video::GetRender()->SetPass((video::EShaderPass)command.from);
						break;
					}
				case RCT_CLEAR_SCREEN:
					{
						video::GetRender()->ClearScreen();
						break;
					}
				case RCT_DRAW_SPRITES:
					{
						if((lastCommand != command.type) && (!skipBatchClear))
						{
							video::GetRender()->GetMatrixModelView().SetIdentity();
							batch = NextBatch();
						}

						u32 from = command.from;
						u32 to = command.count ? (command.from + command.count) : scenePack.sprites.GetAlive();

						while(from < to)
						{
							if(!scenePack.transforms[scenePack.sprites[from].renderAsset.transformIndex].inFrame)
							{
								++from;
								continue;
							}

							if(!frustum.IsAABBIn(scenePack.sprites[from].renderAsset.aabb))
							{
								++from;
								continue;
							}

							u8 result = RenderSpriteToBatch(contentPack, scenePack, scenePack.sprites[from], batch, matView, video::GetRender()->GetViewportOptions().size, currentCameraType);

							if(result == RTBE_MATERIAL_MISS)
								++statistic.materialMissesSprite;

							if(!result)
							{
								if(debugFlags & FSDF_SPRITE_AABB)
									RenderDebugAABB(scenePack.sprites[from].renderAsset.aabb, 2);

								++from;
							}
							else
							{
								if(!batch->IsEmpty())
									RenderBatch(batch);

								batch = NextBatch();
							}
						}

						break;
					}
				case RCT_DRAW_SURFACES:
					{
						if(lastCommand != command.type)
							batch = NULL;

						u32 from = command.from;
						u32 to = command.count ? (command.from + command.count) : scenePack.surfaces.GetAlive();

						while(from < to)
						{
							//if(!frustum.IsAABBIn(scenePack.surfaces[from].renderAsset.aabb))
							//{
							//	++from;
							//	continue;
							//}

							if(!scenePack.transforms[scenePack.surfaces[from].renderAsset.transformIndex].inFrame)
							{
								++from;
								continue;
							}

							video::GetRender()->GetMatrixModelView() = matView * scenePack.global[scenePack.surfaces[from].renderAsset.transformIndex];
							//video::GetRender()->GetMatrixModelView() = matView;

							if(scenePack.surfaces[from].surfaceIndex != u32Max)
							{
								video::teSurfaceData * surfaceData = reinterpret_cast<video::teSurfaceData*>(contentPack.surfaceData.At(scenePack.surfaces[from].surfaceIndex));

								if(surfaceData->skeletonIndex != u32Max)
								{
									scene::teSkeleton * skeleton = (scene::teSkeleton*)contentPack.skeletonData.At(surfaceData->skeletonIndex);
									skeleton->CalculateSkin(scenePack.surfaces[from].blendPairs, scenePack.surfaces[from].blendTimes, scenePack.surfaces[from].blendMode);
								}

								u32 matTemp = surfaceData->materialIndex;
								if(scenePack.surfaces[from].renderAsset.IsValid())
									surfaceData->materialIndex = scenePack.surfaces[from].renderAsset.materialIndex;

								video::GetRender()->Render(contentPack, surfaceData, scenePack.surfaces[from].color);

								u32 aproxDataSize = contentPack.surfaceLayers[surfaceData->layersIndex].stride[video::SLT_POSITION] * surfaceData->vertexCount + contentPack.surfaceLayers[surfaceData->layersIndex].stride[video::SLT_INDEXES] * surfaceData->indexCount;
								statistic.batchUtilization += 100.0f * ((f32)aproxDataSize) / ((f32)surfaceData->dataSize);
								++statistic.dipCounts;

								surfaceData->materialIndex = matTemp;
							}

							if(debugFlags & FSDF_SURFACE_AABB)
								RenderDebugAABB(scenePack.surfaces[from].renderAsset.aabb, 1);

							++from;
						}

						break;
					}
				case RCT_DRAW_TEXT:
					{
						if((lastCommand != command.type) && (!skipBatchClear))
						{
							video::GetRender()->GetMatrixModelView().SetIdentity();
							batch = NextBatch();
						}

						u32 from = command.from;
						u32 to = command.count ? (command.from + command.count) : scenePack.texts.GetAlive();
						u32 textPosition = u32Max;

						while(from < to) // shadow pass
						{
							if(!scenePack.transforms[scenePack.texts[from].renderAsset.transformIndex].inFrame)
							{
								++from;
								continue;
							}

							if(!scenePack.texts[from].options.drawShadow)
							{
								++from;
								continue;
							}

							u8 result = RenderTextToBatch(contentPack, scenePack, scenePack.texts[from], batch, matView, video::GetRender()->GetViewportOptions().size, textPosition, true);

							if(result == RTBE_MATERIAL_MISS)
								++statistic.materialMissesText;

							if(!result)
								++from;
							else
							{
								if(!batch->IsEmpty())
									RenderBatch(batch);

								batch = NextBatch();
							}
						}

						from = command.from;
						to = command.count ? (command.from + command.count) : scenePack.texts.GetAlive();
						textPosition = u32Max;
						u1 okRender = true;

						while(from < to)
						{
							if(okRender)
								scenePack.texts[from].renderAsset.aabb.Flush();

							okRender = true;

							if(!scenePack.transforms[scenePack.texts[from].renderAsset.transformIndex].inFrame)
							{
								++from;
								continue;
							}

							u8 result = RenderTextToBatch(contentPack, scenePack, scenePack.texts[from], batch, matView, video::GetRender()->GetViewportOptions().size, textPosition, false);

							if(result == RTBE_MATERIAL_MISS)
								++statistic.materialMissesText;

							if(!result)
							{
								if(debugFlags & FSDF_TEXT_AABB)
									RenderDebugAABB(scenePack.texts[from].renderAsset.aabb, 3);

								++from;
							}
							else
							{
								okRender = false;

								if(!batch->IsEmpty())
									RenderBatch(batch);

								batch = NextBatch();
							}
						}

						break;
					}
				case RCT_DRAW_ACTOR:
					{
						if((lastCommand != command.type) && (!skipBatchClear))
						{
							video::GetRender()->GetMatrixModelView() = matView;
						}

						u32 from = command.from;
						u32 to = command.count ? (command.from + command.count) : scenePack.drawActors.GetAlive();

						while(from < to)
						{
							if(!scenePack.transforms[scenePack.drawActors[from].renderAsset.transformIndex].inFrame)
							{
								++from;
								continue;
							}

							if(scenePack.drawActors[from].actorPtr && scenePack.drawActors[from].actorSlot)
								(*scenePack.drawActors[from].actorSlot)(scenePack.drawActors[from].actorPtr, 0.0f, 0.0f, 0.0f, 0.0f);

							++from;
						}

						break;
					}
				default:
					TE_ASSERT(0);
					break;
				}

				lastCommand = command.type;
			}

			if(statistic.dipCounts)
				statistic.batchUtilization /= (f32)statistic.dipCounts;

			TE_TIME_END(timeRender)

			statistic.timeRender = (f32)timeRender.ToMilliSeconds();
		}

		void teFastScene::UpdateAABBs()
		{
			for(u32 i = 0; i < scenePack.sprites.GetAlive(); ++i)
			{
				if(scenePack.transformsChangesFlags[scenePack.sprites[i].renderAsset.transformIndex] == 0)
					continue;

				teAABB3df & aabb = scenePack.sprites[i].renderAsset.aabb;

				aabb.SetEdges(teVector3df(0.0f, 0.0f, 0.0f), teVector3df(1.0f, 1.0f, 0.0f));

				if(scenePack.sprites[i].renderAsset.materialIndex == u32Max)
					continue;

				const video::teAtlasSprite * atlasSprite = NULL;
				if(contentPack.materials[scenePack.sprites[i].renderAsset.materialIndex].atlasSpriteIndex[0] != u32Max)
					atlasSprite = contentPack.atlasSprites.At(contentPack.materials[scenePack.sprites[i].renderAsset.materialIndex].atlasSpriteIndex[0]);

				if(atlasSprite)
				{
					aabb.edgeMin.SetXYZ(aabb.edgeMin.x * atlasSprite->size.x, aabb.edgeMin.y * atlasSprite->size.y, 0.0f);
					aabb.edgeMin.SetXYZ(aabb.edgeMin.x - atlasSprite->origin.x, aabb.edgeMin.y + atlasSprite->origin.y - atlasSprite->size.y, 0.0f);
					aabb.edgeMax.SetXYZ(aabb.edgeMax.x * atlasSprite->size.x, aabb.edgeMax.y * atlasSprite->size.y, 0.0f);
					aabb.edgeMax.SetXYZ(aabb.edgeMax.x - atlasSprite->origin.x, aabb.edgeMax.y + atlasSprite->origin.y - atlasSprite->size.y, 0.0f);
				}

				scenePack.sprites[i].renderAsset.aabb.TransformAffine(scenePack.global[scenePack.sprites[i].renderAsset.transformIndex]);
			}

			for(u32 i = 0; i < scenePack.surfaces.GetAlive(); ++i)
			{
				if(scenePack.transformsChangesFlags[scenePack.surfaces[i].renderAsset.transformIndex] == 0)
					continue;

				u32 aabbIndex = contentPack.GetSurfaceAABBIndex(scenePack.surfaces[i].surfaceIndex);
				if(aabbIndex != u32Max)
					scenePack.surfaces[i].renderAsset.aabb = contentPack.surfaceAABB[aabbIndex];
				else
					scenePack.surfaces[i].renderAsset.aabb.Flush();
				scenePack.surfaces[i].renderAsset.aabb.TransformAffine(scenePack.global[scenePack.surfaces[i].renderAsset.transformIndex]);
			}
		}

		void teFastScene::Load(u8 stage, u1 deferred, u1 resaveContentPack)
		{
			if(deferred)
			{
				stageWaitForLoading = stage;
				return;
			}

			#ifdef TE_CTTI

			core::IBuffer * cttiCRCBuffer = core::GetFileManager()->OpenFile("ctti.crc", core::CFileBuffer::FWM_READ, true, true);
			u32 crc = 0;

			if(cttiCRCBuffer)
			{
				cttiCRCBuffer->Lock(core::BLT_READ);
				cttiCRCBuffer->SetPosition(0);
				cttiCRCBuffer->Read(&crc, 4);
				cttiCRCBuffer->Unlock();
			}

			TE_SAFE_DROP(cttiCRCBuffer);

			if(crc != actorsTI.GetCTTICRC())
			{
				TE_LOG_DBG("ctti : incorrect crc");
				return;
			}

			#endif

			// unload loaded stage
			if(stageLoaded != u8Max)
			{
				scenePack.FreeActors();
				scenePack.Clear();
				contentPack.Clear();
			}

			// loading stage
			stageLoaded = stage;

			TE_TIME_BEGIN(loadingTime);

			TE_ASSERT(stage < 100);

			core::IBuffer * stageBuffer = NULL;
			core::IBuffer * contentBuffer = NULL;

			c8 stageName[16];
			c8 contentName[16];
			sprintf(stageName, "s%i.bin", stage);
			sprintf(contentName, "c%i.bin", stage);

			stageBuffer = core::GetFileManager()->OpenFile(stageName);
			if(stageBuffer)
			{
				scenePack.Load(stageBuffer);
				TE_SAFE_DROP(stageBuffer);
			}

			contentBuffer = core::GetFileManager()->OpenFile(contentName);
			if(contentBuffer)
			{
				contentPack.Load(contentBuffer);
				TE_SAFE_DROP(contentBuffer);

				if(resaveContentPack)
				{
					contentBuffer = core::GetFileManager()->OpenFile(contentName, core::CFileBuffer::FWM_WRITE);

					if(contentBuffer)
					{
						contentPack.Save(contentBuffer);
						TE_SAFE_DROP(contentBuffer);
					}
				}
			}

			contentPack.Finalize();

			contentPack.UpdateSurfaceAABB(u32Max, u32Max, false); // TODO temporary calculate aabb here, until https://github.com/tatemgames/tatengine/issues/14

			scenePack.Finalize(this, contentPack);

			FormRenderProgram(program, scenePack, contentPack);

			for(u32 i = 0; i < scenePack.transforms.GetAlive(); ++i)
			{
				memcpy(&scenePack.transformsChangesBuffer[i], &scenePack.transforms[i], sizeof(teAssetTransform));
				CalculateTransformGlobalMatrix(scenePack, i);
				scenePack.transformsChangesFlags[i] = 1;
			}

			UpdateAABBs();

			scenePack.actorsMachine.SetTypeInformation(&actorsTI);
			scenePack.InitActors(this, contentPack);

			TE_TIME_END(loadingTime);

			TE_LOG_DBG("loading time %f", loadingTime.ToSeconds());
		}

		void teFastScene::RenderDebugAABB(const teAABB3df & aabb, u8 color)
		{
			if(contentPack.materials.GetAlive() == 0)
				return;

			teVector3df p0 = aabb.edgeMin;
			teVector3df p1 = aabb.edgeMax;

			u8 temp[2048];

			teColor4u colors[5];
			colors[0].SetRGB(0xff, 0xff, 0xff);
			colors[1].SetRGB(0xf5, 0xd0, 0x9a);
			colors[2].SetRGB(0x4a, 0x9c, 0x7c);
			colors[3].SetRGB(0x8f, 0x43, 0x21);
			colors[4].SetRGB(0x7d, 0x9d, 0xc2);

			video::teMaterial temp2 = contentPack.materials[0];
			contentPack.materials[0].color = colors[teMin(color, (u8)4)];
			contentPack.materials[0].atlasSpriteIndex[0] = u32Max;
			contentPack.materials[0].shaderIndex = video::ST_FAILSAFE;
			contentPack.materials[0].flags = 0;
			contentPack.materials[0].blend = video::BT_DISABLED;
			contentPack.materials[0].metaMaterial = 0;

			video::teSurfaceData * d = (video::teSurfaceData*)temp;

			d->Clear();

			d->operationType = video::ROT_LINES;
			d->layersIndex = teSurfaceLayerSpriteIndex;
			d->indexesOffset = 400;
			d->materialIndex = 0;

			teSpriteVertex * vertexes = reinterpret_cast<teSpriteVertex*>(d->Get(contentPack.surfaceLayers[d->layersIndex], video::SLT_POSITION, d->vertexCount));
			teSpriteIndex * indexes = reinterpret_cast<teSpriteIndex*>(d->Get(contentPack.surfaceLayers[d->layersIndex], video::SLT_INDEXES, d->indexCount));

			d->vertexCount = 0;
			vertexes[d->vertexCount++].Set3D(teVector3df(p0.x, p0.y, p0.z), teVector2duh(0, 0), teColor4u(0, 0, 0, 255));
			vertexes[d->vertexCount++].Set3D(teVector3df(p1.x, p0.y, p0.z), teVector2duh(0, 0), teColor4u(0, 0, 0, 255));
			vertexes[d->vertexCount++].Set3D(teVector3df(p1.x, p1.y, p0.z), teVector2duh(0, 0), teColor4u(0, 0, 0, 255));
			vertexes[d->vertexCount++].Set3D(teVector3df(p0.x, p1.y, p0.z), teVector2duh(0, 0), teColor4u(0, 0, 0, 255));
			vertexes[d->vertexCount++].Set3D(teVector3df(p0.x, p0.y, p1.z), teVector2duh(0, 0), teColor4u(0, 0, 0, 255));
			vertexes[d->vertexCount++].Set3D(teVector3df(p1.x, p0.y, p1.z), teVector2duh(0, 0), teColor4u(0, 0, 0, 255));
			vertexes[d->vertexCount++].Set3D(teVector3df(p1.x, p1.y, p1.z), teVector2duh(0, 0), teColor4u(0, 0, 0, 255));
			vertexes[d->vertexCount++].Set3D(teVector3df(p0.x, p1.y, p1.z), teVector2duh(0, 0), teColor4u(0, 0, 0, 255));

			d->indexCount = 0;
			indexes[d->indexCount++] = 0;
			indexes[d->indexCount++] = 1;
			indexes[d->indexCount++] = 1;
			indexes[d->indexCount++] = 2;
			indexes[d->indexCount++] = 2;
			indexes[d->indexCount++] = 3;
			indexes[d->indexCount++] = 3;
			indexes[d->indexCount++] = 0;

			indexes[d->indexCount++] = 4;
			indexes[d->indexCount++] = 5;
			indexes[d->indexCount++] = 5;
			indexes[d->indexCount++] = 6;
			indexes[d->indexCount++] = 6;
			indexes[d->indexCount++] = 7;
			indexes[d->indexCount++] = 7;
			indexes[d->indexCount++] = 4;

			indexes[d->indexCount++] = 1;
			indexes[d->indexCount++] = 5;
			indexes[d->indexCount++] = 2;
			indexes[d->indexCount++] = 6;
			indexes[d->indexCount++] = 3;
			indexes[d->indexCount++] = 7;
			indexes[d->indexCount++] = 0;
			indexes[d->indexCount++] = 4;

			for(u32 i = 0; i < d->vertexCount; ++i)
				vertexes[i].pos = video::GetRender()->GetMatrixView().MultiplyMatrixOnVector3D(vertexes[i].pos);

			teMatrix4f tempm = video::GetRender()->GetMatrixModelView();
			video::GetRender()->GetMatrixModelView().SetIdentity();

			video::GetRender()->Render(contentPack, d);

			video::GetRender()->GetMatrixModelView() = tempm;

			contentPack.materials[0] = temp2;
		}

		video::teSurfaceData * teFastScene::NextBatch()
		{
			static u32 batch = 0;

			if(++batch > batchs.GetAlive() - 1)
				batch = 0;

			video::teSurfaceData * result = reinterpret_cast<video::teSurfaceData*>(batchData + batchs[batch]);
			result->Clear();
			result->dataSize = teRenderBatchSize;

			return result;
		}

		void teFastScene::RenderBatch(video::teSurfaceData * batch)
		{
			batch->flags |= video::SDF_WAS_CHANGED;

			video::GetRender()->Render(contentPack, batch);

			// fast and good for statistic, but work ONLY with interleaved surface data
			u32 aproxDataSize = contentPack.surfaceLayers[batch->layersIndex].stride[video::SLT_POSITION] * batch->vertexCount +
				contentPack.surfaceLayers[batch->layersIndex].stride[video::SLT_INDEXES] * batch->indexCount;

			statistic.batchUtilization += 100.0f * ((f32)aproxDataSize) / ((f32)batch->dataSize);
			++statistic.dipCounts;
		}
	}
}
