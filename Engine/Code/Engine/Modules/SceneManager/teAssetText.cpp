/*
 *  teAssetText.cpp
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 12/16/11.
 *  Copyright 2011 Tatem Games. All rights reserved.
 *
 */


#include "teAssetText.h"
#include "teContentPack.h"
#include "teAssetPack.h"
#include "teFastScene.h"
#include "teFont.h"

namespace te
{
	namespace scene
	{
		teColor4u textPalette[256];
		teColor4u textShadowPalette[256];
		teVector2df textShadowOffset(0.0f, -2.0f);

		teColor4u * GetTextPalette() {return textPalette;}
		teColor4u * GetTextShadowPalette() {return textShadowPalette;}
		teVector2df & GetTextShadowOffset() {return textShadowOffset;}

		enum ERenderTextState
		{
			RTS_NORMAL = 0,
			RTS_NOT_RENDER_AT_BEGIN,
			RTS_NOT_RENDER_TO_END,
		};

		u8 RenderTextToBatch(teContentPack & contentPack, const teAssetPack & assetPack, teAssetText & text, video::teSurfaceData * batch, const teMatrix4f & matView, const teVector2duh & viewportSize, u32 & textPosition, u1 shadowPass, teVector2df * getTextSize)
		{
			// ---------------------------------------------------------------------------------------

			TE_ASSERT(batch);

			if(!text.renderAsset.IsValid())
				return RTBE_INVALID;

			const font::teFont * font = reinterpret_cast<const font::teFont*>(contentPack.fontData.At(text.fontIndex));
			const teStringInfo * textInfo = reinterpret_cast<const teStringInfo*>(contentPack.stringsData.At(text.stringIndex));
			const teString textString = textInfo->ToString();

			if(!font->metricsCount)
				return RTBE_OK;

			const u32 textBytes = textInfo->size;

			if((textBytes == 0) || (textBytes == 1)) // if bytes == 1 this is null string
				return RTBE_OK;

			u32 charsPerBatch = teRenderBatchSize / (4 * sizeof(teSpriteVertex) + 6 * sizeof(teSpriteIndex));

			// ---------------------------------------------------------------------------------------

			video::teMaterial & material = contentPack.materials[text.renderAsset.materialIndex];

			if(batch->IsEmpty())
			{
				batch->operationType = video::ROT_TRIANGLES;
				batch->layersIndex = teSurfaceLayerSpriteIndex;
				batch->indexesOffset = (teRenderBatchSize * 4 * sizeof(teSpriteVertex)) / (4 * sizeof(teSpriteVertex) + 6 * sizeof(teSpriteIndex)); // TODO
				batch->materialIndex = material.metaMaterial;//text.renderAsset.materialIndex;
			}
			else
			{
				if(batch->materialIndex != material.metaMaterial)//text.renderAsset.materialIndex)
					return RTBE_MATERIAL_MISS;

				if(batch->layersIndex != teSurfaceLayerSpriteIndex)
					return RTBE_VERTEX_STRUCT_MISS;

				if((batch->vertexCount + 4) >= (charsPerBatch * 4))
					return RTBE_NO_MEMORY;
			}

			const teMatrix4f & mat = assetPack.global[text.renderAsset.transformIndex];

			const video::teAtlasSprite * atlasSprite = NULL;
			if(material.atlasSpriteIndex[0] != u32Max)
				atlasSprite = contentPack.atlasSprites.At(material.atlasSpriteIndex[0]);

			// ---------------------------------------------------------------------------------------

			teVector2df cursorPosition;
			teColor4u charColor(255, 255, 255, 255);

			if(shadowPass)
			{
				cursorPosition += textShadowOffset;
				charColor.SetRGBA(16, 16, 16, 255);
			}

			teVector2df currentTextSize;
			teVector2df currentLineSize;
			u1 newLine = true;
			u32 beginTextVertex = batch->vertexCount;
			u32 beginTextIndex = batch->indexCount;
			u32 beginLineVertex = beginTextVertex;

			const c8 * tempString = textString.c_str();
			u32 currentCharacter = u32Max;
			u32 currentTextPosition = 0;

			u8 state = (textPosition == u32Max) ? RTS_NORMAL : RTS_NOT_RENDER_AT_BEGIN;
			u1 lastLineFixAlign = (textPosition == u32Max) ? true : false;
			u1 notRenderAtBegin = (state == RTS_NOT_RENDER_AT_BEGIN);

			while(*tempString != NULL)
			{
				switch(state)
				{
				case RTS_NORMAL:
					{
						if((batch->vertexCount + 4) >= (charsPerBatch * 4))
						{
							textPosition = currentTextPosition;
							state = RTS_NOT_RENDER_TO_END;
						}

						break;
					}
				case RTS_NOT_RENDER_AT_BEGIN:
					{
						if(currentTextPosition >= textPosition)
						{
							state = RTS_NORMAL;
							lastLineFixAlign = true;
						}

						break;
					}
				case RTS_NOT_RENDER_TO_END:
				default:
					break;
				}

				teSpriteVertex * vertexes = reinterpret_cast<teSpriteVertex*>(batch->Get(contentPack.surfaceLayers[batch->layersIndex], video::SLT_POSITION, batch->vertexCount));
				teSpriteIndex * indexes = reinterpret_cast<teSpriteIndex*>(batch->Get(contentPack.surfaceLayers[batch->layersIndex], video::SLT_INDEXES, batch->indexCount));

				u8 readedChars = teUTF8toUTF32(tempString, currentCharacter);
				currentTextPosition += readedChars;
				tempString += readedChars;

				if((currentCharacter == '\r') || (currentCharacter == '\n') || (currentCharacter == '\0'))
				{
					if(currentCharacter == '\r')
					{
						u8 tempSkip = teUTF8toUTF32(tempString, currentCharacter);
						if(currentCharacter == '\n') // \r\n skip
							tempString += tempSkip;
					}

					cursorPosition.x = 0;
					cursorPosition.y -= currentLineSize.y;

					currentTextSize.x = teMax(currentTextSize.x, currentLineSize.x);
					currentTextSize.y += currentLineSize.y;

					if((state == RTS_NORMAL) || lastLineFixAlign)
					{
						teSpriteVertex * vertexesLine = reinterpret_cast<teSpriteVertex*>(batch->Get(contentPack.surfaceLayers[batch->layersIndex], video::SLT_POSITION, beginLineVertex));

						if(text.options.align == font::THA_CENTER)
							for(u32 i = 0; i < batch->vertexCount - beginLineVertex + 1; ++i)
								vertexesLine[i].pos.x -= currentLineSize.x / 2.0f;
						else if(text.options.align == font::THA_RIGHT)
							for(u32 i = 0; i < batch->vertexCount - beginLineVertex + 1; ++i)
								vertexesLine[i].pos.x -= currentLineSize.x;

						if(state != RTS_NORMAL)
							lastLineFixAlign = false;
					}

					currentLineSize.SetXY(0.0f, 0.0f);
					beginLineVertex = batch->vertexCount;

					newLine = true;

					continue;
				}
				else if((currentCharacter == '$') && (((teptr_t)tempString - (teptr_t)textString.c_str() + 3) < textBytes) && (tempString[3] == '$'))
				{
					c8 temp[4];
					temp[0] = tempString[0];
					temp[1] = tempString[1];
					temp[2] = tempString[2];
					temp[3] = '\0';

					s32 value = 0;
					if((temp[0] == 'p') && sscanf(temp + 1, "%x", &value)) // pallete mode
					{
						if(value < 255)
						{
							if(!shadowPass)
								charColor = textPalette[value];
							else
								charColor = textShadowPalette[value];

							tempString += 4;
							continue;
						}
					}
					else if(sscanf(temp, "%x", &value))
					{
						u8 R = ((value >> 8) & 0xF);
						R = R | (R << 4);
						u8 G = ((value >> 4) & 0xF);
						G = G | (G << 4);
						u8 B = ((value >> 0) & 0xF);
						B = B | (B << 4);

						if(!shadowPass)
							charColor.SetRGBA(R, G, B, 0xFF);

						tempString += 4;
						continue;
					}
				}

				// calculate char rectangle
				u32 metricsIndex = font->GetMetricForCharacter(currentCharacter);

				if(metricsIndex == u32Max)
					metricsIndex = 0;

				const font::teCharacterMetrics & metric = font->GetMetric(metricsIndex);

				teVector2df plu = cursorPosition + teVector2df(metric.shift.x, -metric.shift.y);	// left up
				teVector2df prd = plu + teVector2df(metric.size.x, -metric.size.y);				// right down
				teVector2df pld(plu.x, prd.y);
				teVector2df pru(prd.x, plu.y);

				currentLineSize.x = teMax(currentLineSize.x, teAbs((f32)prd.x));
				currentLineSize.y = font->height * text.options.leading;

				teVector2duh uv1 = atlasSprite ? atlasSprite->GetAtlasUV(metric.uv1) : (teVector2duh)metric.uv1;
				teVector2duh uv2 = atlasSprite ? atlasSprite->GetAtlasUV(metric.uv2) : (teVector2duh)metric.uv2;

				if(state == RTS_NORMAL)
				{
					vertexes[0].Set(plu, teVector2duh(uv1.x, uv1.y), teColor4u(teColor4f(charColor) * teColor4f(text.color)));
					vertexes[1].Set(pld, teVector2duh(uv1.x, uv2.y), teColor4u(teColor4f(charColor) * teColor4f(text.color)));
					vertexes[2].Set(pru, teVector2duh(uv2.x, uv1.y), teColor4u(teColor4f(charColor) * teColor4f(text.color)));
					vertexes[3].Set(prd, teVector2duh(uv2.x, uv2.y), teColor4u(teColor4f(charColor) * teColor4f(text.color)));

					indexes[0] = batch->vertexCount + 0;
					indexes[1] = batch->vertexCount + 1;
					indexes[2] = batch->vertexCount + 2;
					indexes[3] = batch->vertexCount + 1;
					indexes[4] = batch->vertexCount + 3;
					indexes[5] = batch->vertexCount + 2;

					batch->vertexCount += 4;
					batch->indexCount += 6;
				}

				// move cursor
				cursorPosition += metric.shiftCharacter;
				cursorPosition.x += text.options.extraTracking;

				if((*tempString != NULL) && (!newLine) && (!text.options.ignoreKerning))
				{
					u32 secondCharacter = u32Max;
					teUTF8toUTF32(tempString, secondCharacter);
					cursorPosition.x += font->GetKerningForCharacters(currentCharacter, secondCharacter);
				}

				newLine = false;
			}

			if(!newLine) // align last line if needed
			{
				currentTextSize.x = teMax(currentTextSize.x, currentLineSize.x);
				currentTextSize.y += currentLineSize.y;

				if((state == RTS_NORMAL) || lastLineFixAlign)
				{
					teSpriteVertex * vertexesLine = reinterpret_cast<teSpriteVertex*>(batch->Get(contentPack.surfaceLayers[batch->layersIndex], video::SLT_POSITION, beginLineVertex));
					if(text.options.align == font::THA_CENTER)
						for(u32 i = 0; i < batch->vertexCount - beginLineVertex + 1; ++i)
							vertexesLine[i].pos.x -= currentLineSize.x / 2.0f;
					else if(text.options.align == font::THA_RIGHT)
						for(u32 i = 0; i < batch->vertexCount - beginLineVertex + 1; ++i)
							vertexesLine[i].pos.x -= currentLineSize.x;

					if(state != RTS_NORMAL)
						lastLineFixAlign = false;
				}
			}

			teVector2df originProportion;
			GetRectangleProportion((ERectangleOriginCorner)text.options.origin, originProportion.x, originProportion.y);

			if(text.options.align == font::THA_CENTER)
				originProportion.x = 0.5f - originProportion.x;
			else if(text.options.align == font::THA_RIGHT)
				originProportion.x = 1.0f - originProportion.x;

			originProportion *= currentTextSize;

			teSpriteVertex * vertexesText = reinterpret_cast<teSpriteVertex*>(batch->Get(contentPack.surfaceLayers[batch->layersIndex], video::SLT_POSITION, beginTextVertex));

			if(originProportion.GetDistanceQR() > 0.0f)
				for(u32 i = 0; i < batch->vertexCount - beginTextVertex + 1; ++i)
					vertexesText[i].pos += originProportion;

			f32 scaleSize = 1.0f;

			if(text.options.size >= 0.0f)
			{
				scaleSize = text.options.size / font->height;

				f32 renderableScaleSize = scaleSize * mat.Get(0, 0);

				f32 smooth_offset = 1.0f / (powf(renderableScaleSize, 1.18f) * 11.0f); // magic approx

				material.metaMaterial = text.renderAsset.materialIndex;

				material.userData[0].uniformType = video::UT_USER0;
				material.userData[0].userDataType = video::UUT_F32;
				material.userData[0].objectsCount = 1;
				material.userData[0].dataPtr = *reinterpret_cast<teptr_t*>(&smooth_offset);
			}

			for(u32 i = 0; i < batch->vertexCount - beginTextVertex; ++i)
			{
				vertexesText[i].pos = mat.MultiplyMatrixOnVector3D(vertexesText[i].pos * scaleSize);

				if(!shadowPass)
				{
					if((i == 0) && (!notRenderAtBegin))
						text.renderAsset.aabb.SetEdges(vertexesText[i].pos, vertexesText[i].pos);
					else
						text.renderAsset.aabb.Unite(teAABB3df(vertexesText[i].pos, vertexesText[i].pos));
				}

				// restore for pixel pefrect fonts, but with distance field we doesnt need this anymore
				//vertexesText[i].pos.x = teFloor(vertexesText[i].pos.x);
				//vertexesText[i].pos.y = teFloor(vertexesText[i].pos.y);
				//if(viewportSize.x % 2)
				//	vertexesText[i].pos.x += 0.5f;
				//if(viewportSize.y % 2)
				//	vertexesText[i].pos.y += 0.5f;
				vertexesText[i].pos = matView.MultiplyMatrixOnVector3D(vertexesText[i].pos);
			}

			if(getTextSize)
				*getTextSize = currentTextSize;

			if(state == RTS_NORMAL)
			{
				textPosition = u32Max;
				return RTBE_OK;
			}
			else if(state == RTS_NOT_RENDER_AT_BEGIN)
			{
				textPosition = u32Max;
				return RTBE_INVALID;
			}
			else if(state == RTS_NOT_RENDER_TO_END)
			{
				return RTBE_OUT_OF_MEMORY;
			}
			else
				return RTBE_INVALID;
		}
	}
}
