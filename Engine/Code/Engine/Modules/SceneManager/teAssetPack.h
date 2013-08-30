/*
 *  teAssetPack.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 12/1/11.
 *  Copyright 2011 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEASSETPACK_H
#define TE_TEASSETPACK_H

#include "teConstArray.h"
#include "teContentPack.h"

#include "teRenderPipeline.h"

#include "teAssetTransform.h"
#include "teAssetCamera.h"
#include "teAssetSprite.h"
#include "teAssetSurface.h"
#include "teAssetText.h"
#include "teAssetDrawActor.h"
#include "teAssetVariable.h"
#include "teAssetArray.h"

#include "teActor.h"

namespace te
{
	namespace scene
	{
		class teFastScene;

		class teAssetPack
		{
		public:
			teAssetPack();
			~teAssetPack();

			void Save(core::IBuffer * buffer);
			void Load(core::IBuffer * buffer);
			void Clear();

			void Finalize(teFastScene * scene, teContentPack & contentPack);

			void InitActors(teFastScene * scene, teContentPack & contentPack);
			void FreeActors();
			TE_INLINE void UpdateActors() {if(actorsInited) actorsMachine.ActorsUpdate();}

			ELinkData GetLinkData(void * linkTo);
			ELinkDataType GetLinkDataType(void * linkTo);

			u32 RequestTransform(u32 parent = u32Max);
			u32 RequestCamera(u32 transformIndex);
			u32 RequestSprite(u32 transformIndex, u32 materialIndex, s16 layer = 0);
			u32 RequestSurface(u32 transformIndex, u32 surfaceIndex, u32 materialIndex, s16 layer = 0);
			u32 RequestText(u32 transformIndex, u32 materialIndex, u32 fontIndex, u32 stringIndex, s16 layer = 0);
			u32 RequestDrawActor(u32 transformIndex, u32 materialIndex, s16 layer = 0);
			u32 RequestProgramCommand(ERenderCommandType command, u32 from, u32 count);
			u32 RequestVariable(EAssetVariableType type = AVT_F32);
			u32 RequestArray();
			u32 RequestActorRTDataLink(u32 assetActorIndex, u16 actorLinkIndex, ELinkData linkData, u32 linkIndex);

			teConstArray<teAssetTransform> transforms;
			teConstArray<teAssetTransform> transformsChangesBuffer;
			teConstArray<u8> transformsChangesFlags;
			teConstArray<teMatrix4f> global;
			teConstArray<teAssetCamera> cameras;
			teConstArray<teAssetSprite> sprites;
			teConstArray<teAssetSurface> surfaces;
			teConstArray<teAssetText> texts;
			teConstArray<teAssetDrawActor> drawActors;
			teConstArray<teRenderCommand> program;
			teConstArray<teAssetVariable> variables;
			teConstArray<teAssetArray> arrays;
			teConstArray<teptr_t> arraysNested; // arrays data that defined in json
			teConstArray<teLink> arrayRTDataLinks;
			teConstArray<teActorRTDataLink> actorsRTDataLinks;
			teConstArray<teLinkRTTI> actorsLinksRTTI;

			u1 actorsInited;
			teActorsMachine actorsMachine;

		private:
			teAssetPack(const teAssetPack & other){}
		};
	}
};

#endif