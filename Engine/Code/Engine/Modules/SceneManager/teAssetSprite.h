/*
 *  teAssetSprite.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 12/1/11.
 *  Copyright 2011 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEASSETSPRITE_H
#define TE_TEASSETSPRITE_H

#include "teRenderAsset.h"
#include "teVector2D.h"
#include "teMatrix4.h"
#include "teColor.h"
#include "teSurfaceData.h"

namespace te
{
	namespace scene
	{
		enum ESpriteFlags
		{
			SF_FLIP_HORIZONTAL	= 0x1,
			SF_FLIP_VERTICAL	= 0x2
		};

		struct teAssetSprite
		{
			teRenderAsset renderAsset;
			teColor4u color;
			u8 flags;

			TE_INLINE void Clear()
			{
				renderAsset.Clear();
				color.SetRGBA(255, 255, 255, 255);
				flags = 0;
			}
		};

		#pragma pack(push, 1)
		
		struct teSpriteVertex
		{
			teVector3df pos;
			teVector2duh uv;
			//teVector2df uv; // for opengl 1.1
			teColor4u color;
			
			TE_INLINE void Set(f32 setX, f32 setY, f32 setZ = 0, u16 setU = 0, u16 setV = 0)
			{
				pos.SetXYZ(setX, setY, setZ);
				uv.SetXY(setU, setV);
				//uv.x = (f32)setU / ((f32)u16Max - 1.0f);
				//uv.y = (f32)setV / ((f32)u16Max - 1.0f);
			}

			void Set(f32 setX, f32 setY, f32 setZ, const teVector2duh & setUV, const teColor4u & setColor)
			{
				pos.SetXYZ(setX, setY, setZ);
				uv = setUV;
				color = setColor;
			}

			TE_INLINE void Set(const teVector2df & setPos, const teVector2duh & setUV, const teColor4u & setColor)
			{
				pos = setPos;
				uv = setUV;
				//uv = (teVector2df)setUV / ((f32)u16Max - 1.0f);
				color = setColor;
			}

			TE_INLINE void Set3D(const teVector3df & setPos, const teVector2duh & setUV, const teColor4u & setColor)
			{
				pos = setPos;
				uv = setUV;
				//uv = (teVector2df)setUV / ((f32)u16Max - 1.0f);
				color = setColor;
			}
		};

		#pragma pack(pop)

		typedef u16 teSpriteIndex;

		class teContentPack;
		class teAssetPack;
		//const video::teSurfaceLayers & GetSpriteLayers();
		u8 RenderSpriteToBatch(const teContentPack & contentPack, const teAssetPack & assetPack, const teAssetSprite & sprite, video::teSurfaceData * batch, const teMatrix4f & matView, const teVector2duh & viewportSize, c8 cameraType);
	}
}

#endif