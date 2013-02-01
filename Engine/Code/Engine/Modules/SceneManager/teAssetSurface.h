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
		/*
		blending tree modes

		bti = blendTimes[i]
		bpi = blendPairs[i]
		state = anim(bpi) // animate skeleton
		state = blend(state1, state2, time) // interpolation between 

		1) mode = 0x1
			result = anim(bp0)

		2) mode = 0x2
			result = anim(bp1)

		3) mode = 0x4
			result = anim(bp2)

		4) mode = 0x8
			result = anim(bp3)

		5) mode = a + b
			result = blend(anim(a), anim(b), bt0)

		6) mode = a + b + c
			result = blend(blend(anim(a), anim(b), bt0), anim(c), bt1);

		7) mode = a + b + c + d
			result = blend(blend(anim(a), anim(b), bt0), blend(anim(c), anim(d), bt1), bt2);
		*/

		struct teAssetSurface
		{
			teRenderAsset renderAsset;
			teColor4u color;
			u32 surfaceIndex;
			f32 blendTimes[3]; // value from 0 to 1
			teVector2duh blendPairs[4]; // x = layer, y = frame
			u8 blendMode;

			TE_INLINE void Clear()
			{
				renderAsset.Clear();
				color.SetRGBA(255, 255, 255, 255);
				surfaceIndex = u32Max;
				for(u8 i = 0; i < 3; ++i)
					blendTimes[i] = 0.0f;
				for(u8 i = 0; i < 4; ++i)
					blendPairs[i].Flush();
				blendMode = 0;
			}
		};
	}
}

#endif