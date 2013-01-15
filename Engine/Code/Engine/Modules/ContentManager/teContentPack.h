/*
 *  teContentPack.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 12/1/11.
 *  Copyright 2011 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TECONTENTPACK_H
#define TE_TECONTENTPACK_H

#include "teConstArray.h"
#include "teString.h"
#include "teColor.h"
#include "teSurfaceData.h"
#include "teVector2D.h"
#include "teAABB3d.h"
#include "teMaterial.h"
#include "teTexture.h"
#include "teSkeleton.h"
#include "teShaderLibrary.h"
#include "teAtlasSprite.h"
#include "teFrameBuffer.h"
#include "teSoundManager.h"

namespace te
{
	namespace scene
	{
		struct teStringInfo
		{
			u32 size; // in bytes with null character
			c8 data[]; // with null character at end

			TE_INLINE void Clear()
			{
				size = 0;
			}

			TE_INLINE u32 GetDataSize() const
			{
				return sizeof(teStringInfo) + size;
			}

			TE_INLINE teString ToString() const
			{
				return teString(data);
			}
		};

		struct teTextureInfo
		{
			u8 flags;
			u8 renderBufferType;
			teVector2duh renderBufferSize;
			teStringInfo name;

			TE_INLINE void Clear()
			{
				flags = 0;
				renderBufferType = 0;
				name.Clear();
			}

			TE_INLINE u32 GetDataSize() const
			{
				return sizeof(teTextureInfo) + name.size;
			}
		};

		const u8 teSurfaceLayerSpriteIndex = 0;

		class teContentPack
		{
		public:
			teContentPack();
			~teContentPack();

			void Save(core::IBuffer * buffer);
			void Load(core::IBuffer * buffer);
			void Clear();

			void Finalize();
			void Invalidate();

			void UpdateSurfaceAABB(u32 surfaceIndex = u32Max, u32 surfaceOffset = u32Max);

			static TE_INLINE u32 GetStringSize(teStringInfo * stringInfo) {return stringInfo->size - sizeof(teStringInfo);}
			static TE_INLINE c8 * GetStringRaw(teStringInfo * stringInfo) {return stringInfo->data;}

			TE_INLINE u32 GetStringSize(u32 stringIndex) const {return GetStringSize((teStringInfo*)stringsData.At(stringIndex));}
			TE_INLINE c8 * GetStringRaw(u32 stringIndex) {return GetStringRaw((teStringInfo*)stringsData.At(stringIndex));}

			u1 finalized;
			teConstArray<video::teTexture> textures;
			teConstArray<u8> texturesData; // contains teTextureInfo
			teConstArray<video::teFrameBuffer> frameBuffers;
			teConstArray<video::teMaterial> materials;
			teConstArray<video::teSurfaceLayers> surfaceLayers;
			teConstArray<u8> surfaceData; // contains teSurfaceData
			teConstArray<teAABB3df> surfaceAABB;
			teConstArray<u8> skeletonData; // contains teSkeleton
			teConstArray<video::teAtlasSprite> atlasSprites;
			teConstArray<u8> fontData; // contains teFont
			teConstArray<c8> stringsData; // contains teStringInfo
			teConstArray<u8> configsData;
			teConstArray<sound::teSound> soundsData;

		private:
			teContentPack(const teContentPack & other){}
		};
	};
}

#endif