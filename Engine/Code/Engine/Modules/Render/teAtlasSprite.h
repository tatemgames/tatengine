/*
 *  teAtlasSprite.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 04/15/11.
 *  Copyright 2012 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEATLASSPRITE_H
#define TE_TEATLASSPRITE_H

#include "teVector2d.h"

namespace te
{
	namespace video
	{
		struct teAtlasSprite
		{
			teVector2df origin;
			teVector2duh size;
			teVector2duh uv1;
			teVector2duh uv2;
			u16 textureIndex;
			u1 texelToPixel;

			TE_INLINE void Clear()
			{
				origin.SetXY(0.0f, 0.0f);
				size.SetXY(0, 0);
				uv1.SetXY(0, 0);
				uv2.SetXY(u16Max, u16Max);
				textureIndex = u16Max;
				texelToPixel = false;
			}

			TE_INLINE void SetUV(const teVector2duh & imgSize, const teVector2duh & pixelA, const teVector2duh & pixelB, u1 setTexelToPixel = true, u1 overrideSize = false, const teVector2duh & setSize = teVector2duh(0, 0))
			{
				texelToPixel = setTexelToPixel;

				teVector2df uv1f;
				teVector2df uv2f;

				if(texelToPixel)
				{
					uv1f = (teVector2df(pixelA) + 0.5f) / teVector2df(imgSize);
					uv2f = (teVector2df(pixelB) + 0.5f) / teVector2df(imgSize);
				}
				else
				{
					uv1f = teVector2df(pixelA) / teVector2df(imgSize - 1);
					uv2f = teVector2df(pixelB) / teVector2df(imgSize - 1);
				}

				uv1 = uv1f * (f32)(u16Max - 1);
				uv2 = uv2f * (f32)(u16Max - 1);

				if(overrideSize)
					size = setSize;
				else
					size = pixelB - pixelA + 1;

				origin = teVector2df(size) / 2.0f;
			}

			TE_INLINE teVector2duh GetAtlasUV(const teVector2duh & localUV) const
			{
				teVector2df uv1f = teVector2df(uv1) / ((f32)u16Max - 1.0f);
				teVector2df uv2f = teVector2df(uv2) / ((f32)u16Max - 1.0f);
				teVector2df uvlf = teVector2df(localUV) / ((f32)u16Max - 1.0f);
				teVector2df resf = uv1f + (uv2f - uv1f) * uvlf;
				teVector2duh res = resf * ((f32)u16Max - 1.0f);
				return res;
			}
		};
	}
}

#endif