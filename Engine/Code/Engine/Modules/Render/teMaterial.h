/*
 *  teMaterial.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 04/15/11.
 *  Copyright 2012 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEMATERIAL_H
#define TE_TEMATERIAL_H

#include "teColor.h"
#include "teMath.h"
#include "teShader.h"

namespace te
{
	namespace video
	{
		const u8 teMaterialMaxTextures = 4;
		const u8 teMaterialMaxUniformUserData = 4;

		enum EBlendType
		{
			BT_DISABLED = 0,
			BT_ALPHA,
			BT_LIGHT,
			BT_SHADE,
			BT_ADDITIVE,
			BT_MULTIPLY
		};

		enum EMaterialFlags
		{
			MF_DEPTH_BUFFER_TEST = 0,
			MF_DEPTH_BUFFER_WRITE = 1,
		};

		struct teMaterial
		{
			u32 atlasSpriteIndex[teMaterialMaxTextures];

			// needed for batching, you can simply replace this material with meta material when you bake texture UV inside geometry
			// if you setup custom user data, be sure to set metaMaterial index to this material index
			u32 metaMaterial;
			teColor4u color;
			u8 blend;
			u8 shaderIndex;
			u8 flags;
			u8 _reserved;
			teUniformUserData userData[teMaterialMaxUniformUserData];

			TE_INLINE void Clear()
			{
				for(u8 i = 0; i < teMaterialMaxTextures; ++i)
					atlasSpriteIndex[i] = u32Max;
				metaMaterial = u32Max;
				color.SetRGBA(255, 255, 255, 255);
				blend = BT_ALPHA;
				shaderIndex = 0;
				flags = 0;
				for(u8 i = 0; i < teMaterialMaxUniformUserData; ++i)
					userData[i].Clear();
			}

			TE_INLINE u1 IsEqual(const teMaterial & other, u1 excludeAtlasSprite = false) const
			{
				if(excludeAtlasSprite)
				{
					u1 fields = (color == other.color) && (blend == other.blend) && (shaderIndex == other.shaderIndex) && (flags == other.flags);

					if(fields)
					{
						for(u8 i = 0; i < teMaterialMaxUniformUserData; ++i)
							if(!userData[i].IsEqual(other.userData[i]))
								return false;

						return true;
					}
					else
						return false;
				}
				else
					return !memcmp(this, &other, sizeof(teMaterial));
			}

			TE_INLINE void SetFlag(EMaterialFlags flag, u1 state)
			{
				TE_SET_BIT(flags, flag, state);
			}

			TE_INLINE u1 IsFlag(EMaterialFlags flag) const
			{
				return TE_GET_BIT(flags, flag);
			}
		};
	}
}

#endif