/*
 *  teAssetSurface.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 12/1/11.
 *  Copyright 2011 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEASSETSURFACE_H
#define TE_TEASSETSURFACE_H

#include "teRenderAsset.h"
#include "teColor.h"

namespace te
{
	namespace scene
	{
		struct teAssetSurface
		{
			teRenderAsset renderAsset;
			teColor4u color;
			u32 surfaceIndex;
			u16 skeletonLayer[2];
			u16 skeletonFrame[2];
			f32 skeletonDelta;

			TE_INLINE void Clear()
			{
				renderAsset.Clear();
				color.SetRGBA(255, 255, 255, 255);
				surfaceIndex = u32Max;
				skeletonLayer[0] = 0;
				skeletonLayer[1] = 0;
				skeletonFrame[0] = 0;
				skeletonFrame[1] = 0;
				skeletonDelta = 0.0f;
			}
		};
	}
}

#endif