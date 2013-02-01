/*
 *  teAssetSprite.cpp
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 12/11/11.
 *  Copyright 2011 Tatem Games. All rights reserved.
 *
 */

#include "teAssetSprite.h"
#include "teContentPack.h"
#include "teAssetPack.h"
#include "teFastScene.h"

namespace te
{
	namespace scene
	{
		u8 RenderSpriteToBatch(const teContentPack & contentPack, const teAssetPack & assetPack, const teAssetSprite & sprite, video::teSurfaceData * batch, const teMatrix4f & matView, const teVector2duh & viewportSize, c8 cameraType)
		{
			TE_ASSERT(batch);

			const u32 spritesPerBatch = teRenderBatchSize / (4 * sizeof(teSpriteVertex) + 6 * sizeof(teSpriteIndex));

			if((batch->vertexCount + 4) > (spritesPerBatch * 4))
				return RTBE_NO_MEMORY;
			
			if(sprite.renderAsset.materialIndex == u32Max)
				return RTBE_OK;

			const video::teMaterial & material = contentPack.materials[sprite.renderAsset.materialIndex];

			if(batch->IsEmpty())
			{
				batch->operationType = video::ROT_TRIANGLES;
				batch->layersIndex = teSurfaceLayerSpriteIndex;
				batch->indexesOffset = (teRenderBatchSize * 4 * sizeof(teSpriteVertex)) / (4 * sizeof(teSpriteVertex) + 6 * sizeof(teSpriteIndex)); // TODO
				batch->materialIndex = material.metaMaterial;//sprite.renderAsset.materialIndex;
			}
			else
			{
				if(batch->materialIndex != material.metaMaterial)//sprite.renderAsset.materialIndex)
					return RTBE_MATERIAL_MISS;
			}

			teSpriteVertex * vertexes = reinterpret_cast<teSpriteVertex*>(batch->Get(contentPack.surfaceLayers[batch->layersIndex], video::SLT_POSITION, batch->vertexCount));
			teSpriteIndex * indexes = reinterpret_cast<teSpriteIndex*>(batch->Get(contentPack.surfaceLayers[batch->layersIndex], video::SLT_INDEXES, batch->indexCount));
			
			const teMatrix4f & mat = assetPack.global[sprite.renderAsset.transformIndex];

			const video::teAtlasSprite * atlasSprite = NULL;
			if(material.atlasSpriteIndex[0] != u32Max)
				atlasSprite = contentPack.atlasSprites.At(material.atlasSpriteIndex[0]);

			teVector3df p[4];

			p[0].SetXYZ(0.0f, 0.0f, 0.0f);
			p[1].SetXYZ(1.0f, 0.0f, 0.0f);
			p[2].SetXYZ(0.0f, 1.0f, 0.0f);
			p[3].SetXYZ(1.0f, 1.0f, 0.0f);

			if(atlasSprite)
			{
				for(u8 i = 0; i < 4; ++i)
				{
					p[i].SetXYZ(p[i].x * (atlasSprite->size.x), p[i].y * atlasSprite->size.y, 0.0f);

					if(atlasSprite && atlasSprite->texelToPixel)
						p[i].SetXYZ(p[i].x - teFloor(atlasSprite->origin.x), p[i].y + teFloor(atlasSprite->origin.y) - atlasSprite->size.y, 0.0f);
					else
						p[i].SetXYZ(p[i].x - atlasSprite->origin.x, p[i].y + atlasSprite->origin.y - atlasSprite->size.y, 0.0f);
				}
			}

			for(u8 i = 0; i < 4; ++i)
			{
				p[i] = mat.MultiplyMatrixOnVector3D(p[i]);
				p[i] = matView.MultiplyMatrixOnVector3D(p[i]);
			}

			if(atlasSprite && atlasSprite->texelToPixel)
				for(u8 i = 0; i < 4; ++i)
					p[i].SetXYZ(teFloor(p[i].x), teFloor(p[i].y), teFloor(p[i].z));

			f32 shiftConstX = 0.0f;
			f32 shiftConstY = 0.0f;

			if(cameraType == CT_ORTHOGRAPHIC)
			{
				if(viewportSize.x % 2)
					shiftConstX = 0.5f;

				if(viewportSize.y % 2)
					shiftConstY = 0.5f;
			}
			
			u1 flipH = (sprite.flags & SF_FLIP_HORIZONTAL);
			u1 flipV = (sprite.flags & SF_FLIP_VERTICAL);

			if(atlasSprite)
			{
				vertexes[0].Set(p[0].x + shiftConstX, p[0].y + shiftConstY, p[0].z, (flipV ? atlasSprite->uv2.x : atlasSprite->uv1.x), (flipH ? atlasSprite->uv1.y : atlasSprite->uv2.y));
				vertexes[1].Set(p[1].x + shiftConstX, p[1].y + shiftConstY, p[1].z, (flipV ? atlasSprite->uv1.x : atlasSprite->uv2.x), (flipH ? atlasSprite->uv1.y : atlasSprite->uv2.y));
				vertexes[2].Set(p[2].x + shiftConstX, p[2].y + shiftConstY, p[2].z, (flipV ? atlasSprite->uv2.x : atlasSprite->uv1.x), (flipH ? atlasSprite->uv2.y : atlasSprite->uv1.y));
				vertexes[3].Set(p[3].x + shiftConstX, p[3].y + shiftConstY, p[3].z, (flipV ? atlasSprite->uv1.x : atlasSprite->uv2.x), (flipH ? atlasSprite->uv2.y : atlasSprite->uv1.y));
			}
			else
			{
				vertexes[0].Set(p[0].x, p[0].y, p[0].z, (flipV ? u16Max : 0), (flipH ? 0 : u16Max));
				vertexes[1].Set(p[1].x, p[1].y, p[1].z, (flipV ? 0 : u16Max), (flipH ? 0 : u16Max));
				vertexes[2].Set(p[2].x, p[2].y, p[2].z, (flipV ? u16Max : 0), (flipH ? u16Max : 0));
				vertexes[3].Set(p[3].x, p[3].y, p[3].z, (flipV ? 0 : u16Max), (flipH ? u16Max : 0));
			}

			vertexes[0].color = sprite.color;
			vertexes[1].color = sprite.color;
			vertexes[2].color = sprite.color;
			vertexes[3].color = sprite.color;

			teSpriteIndex indexesBase[] = {0, 1, 2, 1, 3, 2};
			
			for(u8 i = 0; i < 6; ++i)
				indexes[i] = indexesBase[i] + batch->vertexCount;

			batch->vertexCount += 4;
			batch->indexCount += 6;

			return RTBE_OK;
		}
	}
}
