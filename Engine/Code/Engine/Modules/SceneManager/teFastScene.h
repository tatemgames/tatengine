/*
 *  teFastScene.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 12/1/11.
 *  Copyright 2011 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEFASTSCENE_H
#define TE_TEFASTSCENE_H

#include "teReferenceCounter.h"

#include "teAssetPack.h"

#include "teConstArray.h"
#include "teRenderPipeline.h"
#include "teSurfaceData.h"

namespace te
{
	namespace scene
	{
		// ------------------------------------------------------------------------------------------

		//struct teFastSceneOptions
		//{
		//	u32 batchSize;
		//	u16 spritesPerBatch;
		//};

		const u32 teRenderBatchSize = 128 * 1024;

		const u32 maxBatchs = 4;

		// ------------------------------------------------------------------------------------------

		enum ERenderToBatchError
		{
			RTBE_OK = 0,
			RTBE_NO_MEMORY = 1, // cant even start render to this batch
			RTBE_OUT_OF_MEMORY = 2, // render part of data to this batch, but run out of memory
			RTBE_MATERIAL_MISS = 3,
			RTBE_VERTEX_STRUCT_MISS = 4,
			RTBE_INVALID = 5,
		};

		// ------------------------------------------------------------------------------------------

		struct teFastSceneStatistic
		{
			u32 dipCounts;
			f32 batchUtilization;
			u32 materialMissesSprite;
			u32 materialMissesText;

			inline void Clear()
			{
				dipCounts = 0;
				batchUtilization = 0;
				materialMissesSprite = 0;
				materialMissesText = 0;
			}

			inline teString ToString(teStringConcate & pool) const
			{
				return pool.Add("dip %u ut %i% mms %u mmt %u", dipCounts, batchUtilization, materialMissesSprite, materialMissesText).BakeToString();
			}
		};

		// ------------------------------------------------------------------------------------------

		typedef void (*teRegisterActorsCallback)(teActorsTI *);

		class teFastScene : public te::teReferenceCounter
		{
		public:
			teFastScene(teRegisterActorsCallback registerActorsCallback = NULL);
			~teFastScene();

			void OnUpdate();
			void OnRender();

			void Load(u8 stage = 0, u1 deferred = false); // use deferred if you call from actor OnUpdate
			TE_INLINE u8 GetStage() const {return stageLoaded;}

			TE_INLINE const teFastSceneStatistic & GetStatistic() const {return statistic;}

			TE_INLINE teRenderProgram & GetRenderProgram() {return program;}
			TE_INLINE const teRenderProgram & GetRenderProgram() const {return program;}

			TE_INLINE teActorsTI & GetActorsTI() {return actorsTI;}
			TE_INLINE const teActorsTI & GetActorsTI() const {return actorsTI;}

			TE_INLINE teAssetPack & GetAssetPack() {return scenePack;}
			TE_INLINE const teAssetPack & GetAssetPack() const {return scenePack;}
			
			TE_INLINE teContentPack & GetContentPack() {return contentPack;}
			TE_INLINE const teContentPack & GetContentPack() const {return contentPack;}

			video::teSurfaceData * NextBatch();
			void RenderBatch(video::teSurfaceData * batch);
		protected:
			// ---------------------------------------------------------------------------

			// nodes, assets, actors

			teActorsTI actorsTI;

			u8 stageLoaded, stageWaitForLoading;
			teAssetPack scenePack;
			teContentPack contentPack;

			// ---------------------------------------------------------------------------

			// render pipeline

			teFastSceneStatistic statistic;
			teRenderProgram program;

			// ---------------------------------------------------------------------------

			// render batch fill

			teConstArray<u32> batchs;
			u8 * batchData;
		};
	}
}

#endif