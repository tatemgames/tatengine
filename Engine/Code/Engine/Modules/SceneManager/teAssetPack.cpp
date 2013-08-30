/*
 *  teAssetPack.cpp
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 12/1/11.
 *  Copyright 2011 Tatem Games. All rights reserved.
 *
 */

#include "teAssetPack.h"

namespace te
{
	namespace scene
	{
		teAssetPack::teAssetPack()
			:actorsInited(false)
		{
		}

		teAssetPack::~teAssetPack()
		{
		}

		void teAssetPack::Save(core::IBuffer * buffer)
		{
			u32 size = 4;
			size += transforms.GetSystemSize();
			size += cameras.GetSystemSize();
			size += sprites.GetSystemSize();
			size += surfaces.GetSystemSize();
			size += texts.GetSystemSize();
			size += drawActors.GetSystemSize();
			size += program.GetSystemSize();
			size += variables.GetSystemSize();
			size += arrays.GetSystemSize();
			size += arraysNested.GetSystemSize();
			size += arrayRTDataLinks.GetSystemSize();
			size += actorsRTDataLinks.GetSystemSize();
			size += actorsMachine.GetSystemSize();

			buffer->Lock(core::BLT_WRITE);
			buffer->Allocate(size);
			buffer->SetPosition(0);
			buffer->Write("TEAP", 4);

			transforms.Save(buffer);
			cameras.Save(buffer);
			sprites.Save(buffer);
			surfaces.Save(buffer);
			texts.Save(buffer);
			drawActors.Save(buffer);
			program.Save(buffer);
			variables.Save(buffer);
			arrays.Save(buffer);
			arraysNested.Save(buffer);
			arrayRTDataLinks.Save(buffer);
			actorsRTDataLinks.Save(buffer);
			actorsMachine.Save(buffer);

			buffer->Unlock();
		}

		void teAssetPack::Load(core::IBuffer * buffer)
		{
			buffer->Lock(core::BLT_READ);
			buffer->SetPosition(4);

			transforms.Load(buffer);
			cameras.Load(buffer);
			sprites.Load(buffer);
			surfaces.Load(buffer);
			texts.Load(buffer);
			drawActors.Load(buffer);
			program.Load(buffer);
			variables.Load(buffer);
			arrays.Load(buffer);
			arraysNested.Load(buffer);
			arrayRTDataLinks.Load(buffer);
			actorsRTDataLinks.Load(buffer);
			actorsMachine.Load(buffer);
			
			buffer->Unlock();

			global.Reserve(transforms.GetAlive());
			global.Request(transforms.GetAlive());
			transformsChangesBuffer.Reserve(transforms.GetAlive());
			transformsChangesBuffer.Request(transforms.GetAlive());
			transformsChangesFlags.Reserve(transforms.GetAlive());
			transformsChangesFlags.Request(transforms.GetAlive());

			for(u32 i = 0; i < global.GetAlive(); ++i)
				global[i].SetIdentity();
		}

		void teAssetPack::Clear()
		{
			transforms.Clear();
			cameras.Clear();
			sprites.Clear();
			surfaces.Clear();
			texts.Clear();
			drawActors.Clear();
			program.Clear();
			variables.Clear();
			arrays.Clear();
			arraysNested.Clear();
			arrayRTDataLinks.Clear();
			actorsRTDataLinks.Clear();
			actorsLinksRTTI.Clear();
			actorsMachine.Clear();
			global.Clear();
		}

		void teAssetPack::Finalize(teFastScene * scene, teContentPack & contentPack)
		{
			for(u32 i = 0; i < arrayRTDataLinks.GetAlive(); ++i)
			{
				teptr_t & link = *arraysNested.At(i);
				link = reinterpret_cast<teptr_t>(LinkResolve(arrayRTDataLinks[i], *this, contentPack));
			}

			arrayRTDataLinks.Clear();

			for(u32 i = 0; i < arrays.GetAlive(); ++i)
				if(arrays[i].data != teptr_t_Max)
					arrays[i].data = (teptr_t)arraysNested.At(arrays[i].data);
		}

		// ---------------------------------------------------------------------------------------------------------

		void teAssetPack::InitActors(teFastScene * scene, teContentPack & contentPack)
		{
			TE_ASSERT(!actorsInited);

			if(actorsRTDataLinks.GetAlive())
			{
				actorsLinksRTTI.Reserve(actorsRTDataLinks.GetAlive());
				actorsLinksRTTI.Request(actorsRTDataLinks.GetAlive());

				for(u32 i = 0; i < actorsRTDataLinks.GetAlive(); ++i)
				{
					teptr_t & link = actorsMachine.GetActorLink(actorsRTDataLinks[i].assetActorIndex, actorsRTDataLinks[i].actorLinkIndex);
					link = reinterpret_cast<teptr_t>(LinkResolve(actorsRTDataLinks[i].link, *this, contentPack));

					actorsLinksRTTI[i].ptr = link;
					actorsLinksRTTI[i].link = actorsRTDataLinks[i].link;
				}

				actorsRTDataLinks.Clear();
			}

			actorsMachine.ActorsFinalize();
			actorsMachine.ActorsConstruct(scene);

			actorsInited = true;
		}

		void teAssetPack::FreeActors()
		{
			if(actorsInited)
			{
				actorsMachine.ActorsDestruct();
				actorsInited = false;
			}
		}

		ELinkData teAssetPack::GetLinkData(void * linkTo)
		{
			if(!linkTo)
				return LD_RAW;

			for(u32 i = 0; i < actorsLinksRTTI.GetAlive(); ++i)
				if(actorsLinksRTTI[i].ptr == (teptr_t)linkTo)
					return (ELinkData)actorsLinksRTTI[i].link.data;

			return LD_RAW;
		}

		ELinkDataType teAssetPack::GetLinkDataType(void * linkTo)
		{
			if(!linkTo)
				return LDT_UNKNOWN;

			for(u32 i = 0; i < actorsLinksRTTI.GetAlive(); ++i)
				if(actorsLinksRTTI[i].ptr == (teptr_t)linkTo)
					return GetLinkType(actorsLinksRTTI[i].link, *this);

			return LDT_UNKNOWN;
		}

		// ---------------------------------------------------------------------------------------------------------

		#define TE_ASSET_PACK_RESERVE_MORE_BLOCK_SIZE 32

		#define TE_ASSET_PACK_RESERVE_MORE(__const_array) \
			{ \
				if(__const_array.GetAlive() >= __const_array.GetSize()) \
					__const_array.ReserveMore(TE_ASSET_PACK_RESERVE_MORE_BLOCK_SIZE); \
			}

		u32 teAssetPack::RequestTransform(u32 parent)
		{
			TE_ASSET_PACK_RESERVE_MORE(transforms);
			TE_ASSET_PACK_RESERVE_MORE(global);

			u32 index = 0;
			teAssetTransform * transform = transforms.Request(&index);
			global.Request();
			transform->Clear();
			transform->parent = parent;
			return index;
		}

		u32 teAssetPack::RequestCamera(u32 transformIndex)
		{
			TE_ASSET_PACK_RESERVE_MORE(cameras);

			u32 index = 0;
			teAssetCamera * camera = cameras.Request(&index);
			camera->Clear();
			camera->transformIndex = transformIndex;
			return index;
		}

		u32 teAssetPack::RequestSprite(u32 transformIndex, u32 materialIndex, s16 layer)
		{
			TE_ASSET_PACK_RESERVE_MORE(sprites);

			u32 index = 0;
			teAssetSprite * sprite = sprites.Request(&index);
			sprite->Clear();
			sprite->renderAsset.transformIndex = transformIndex;
			sprite->renderAsset.layer = layer;
			sprite->renderAsset.materialIndex = materialIndex;
			return index;
		}

		u32 teAssetPack::RequestSurface(u32 transformIndex, u32 surfaceIndex, u32 materialIndex, s16 layer)
		{
			TE_ASSET_PACK_RESERVE_MORE(surfaces);

			u32 index = 0;
			teAssetSurface * surface = surfaces.Request(&index);
			surface->Clear();
			surface->renderAsset.transformIndex = transformIndex;
			surface->renderAsset.layer = layer;
			surface->renderAsset.materialIndex = materialIndex;
			surface->surfaceIndex = surfaceIndex;
			return index;
		}

		u32 teAssetPack::RequestText(u32 transformIndex, u32 materialIndex, u32 fontIndex, u32 stringIndex, s16 layer)
		{
			TE_ASSET_PACK_RESERVE_MORE(texts);

			u32 index = 0;
			teAssetText * text = texts.Request(&index);
			text->Clear();
			text->renderAsset.transformIndex = transformIndex;
			text->renderAsset.layer = layer;
			text->renderAsset.materialIndex = materialIndex;
			text->fontIndex = fontIndex;
			text->stringIndex = stringIndex;
			return index;
		}

		u32 teAssetPack::RequestDrawActor(u32 transformIndex, u32 materialIndex, s16 layer)
		{
			TE_ASSET_PACK_RESERVE_MORE(drawActors);

			u32 index = 0;
			teAssetDrawActor * drawActor = drawActors.Request(&index);
			drawActor->Clear();
			drawActor->renderAsset.transformIndex = transformIndex;
			drawActor->renderAsset.layer = layer;
			drawActor->renderAsset.materialIndex = materialIndex;
			return index;
		}

		u32 teAssetPack::RequestProgramCommand(ERenderCommandType command, u32 from, u32 count)
		{
			TE_ASSET_PACK_RESERVE_MORE(program);

			u32 index = 0;
			teRenderCommand * renderCommand = program.Request(&index);
			renderCommand->Set(command, from, count);
			return index;
		}

		u32 teAssetPack::RequestVariable(EAssetVariableType type)
		{
			TE_ASSET_PACK_RESERVE_MORE(variables);

			u32 index = 0;
			teAssetVariable * variable = variables.Request(&index);
			variable->Clear();
			variable->type = type;
			return index;
		}

		u32 teAssetPack::RequestArray()
		{
			TE_ASSET_PACK_RESERVE_MORE(arrays);

			u32 index = 0;
			teAssetArray * assetArray = arrays.Request(&index);
			assetArray->Clear();
			return index;
		}

		u32 teAssetPack::RequestActorRTDataLink(u32 assetActorIndex, u16 actorLinkIndex, ELinkData linkData, u32 linkIndex)
		{
			TE_ASSET_PACK_RESERVE_MORE(actorsRTDataLinks);

			u32 index = 0;
			teActorRTDataLink * actorsRTDataLink = actorsRTDataLinks.Request(&index);
			actorsRTDataLink->Clear();
			actorsRTDataLink->assetActorIndex = assetActorIndex;
			actorsRTDataLink->actorLinkIndex = actorLinkIndex;
			actorsRTDataLink->link.data = linkData;
			actorsRTDataLink->link.index = linkIndex;
			return index;
		}
	}
};
