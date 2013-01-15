/*
 *  teRenderAsset.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 12/1/11.
 *  Copyright 2011 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TERENDERASSET_H
#define TE_TERENDERASSET_H

#include "teTypes.h"
#include "teAABB3d.h"

namespace te
{
	namespace scene
	{
		struct teRenderAsset
		{
			u32 transformIndex;
			u32 materialIndex;
			teAABB3df aabb;
			s16 layer;

			inline void Clear()
			{
				transformIndex = u32Max;
				materialIndex = u32Max;
				aabb.Flush();
				layer = 0;
			}

			TE_INLINE u1 IsValid() const
			{
				return (transformIndex != u32Max) && (materialIndex != u32Max);
			}

			inline void UpdateIndexes(u32 baseTransformIndex)
			{
				if(transformIndex != u32Max)
					transformIndex += baseTransformIndex;
			}
		};
	}
}

#endif