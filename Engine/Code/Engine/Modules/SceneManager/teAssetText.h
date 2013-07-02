/*
 *  teAssetText.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 12/1/11.
 *  Copyright 2011 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEASSETTEXT_H
#define TE_TEASSETTEXT_H

#include "teRenderAsset.h"
#include "teMatrix4.h"
#include "teColor.h"
#include "teFont.h"
#include "teSurfaceData.h"

namespace te
{
	namespace scene
	{
		//! @class teAssetText
		//! @brief Scene graph text asset
		struct teAssetText
		{
			teRenderAsset renderAsset;
			teColor4u color;
			u32 fontIndex;
			u32 stringIndex;
			font::teFontRenderingOptions options; // aligned to 4 bytes

			TE_INLINE void Clear()
			{
				renderAsset.Clear();
				color.SetRGBA(255, 255, 255, 255);
				fontIndex = u32Max;
				stringIndex = u32Max;
				options.Clear();
			}
		};

		// palette is 256 items array, for $pxx$ color resolve, by default all colors are black (0, 0, 0, 255)
		teColor4u * GetTextPalette();
		teColor4u * GetTextShadowPalette();

		class teContentPack;
		class teAssetPack;
		// textPosition shoud be u32Max by default
		u8 RenderTextToBatch(teContentPack & contentPack, const teAssetPack & assetPack, teAssetText & text, video::teSurfaceData * batch, const teMatrix4f & matView, const teVector2duh & viewportSize, u32 & textPosition, u1 shadowPass, teVector2df * getTextSize = NULL);
	}
}

#endif
