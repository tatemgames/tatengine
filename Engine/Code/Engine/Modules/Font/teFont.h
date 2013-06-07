/*
 *  teFont.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 03/3/10.
 *  Copyright 2010 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEFONT_H
#define TE_TEFONT_H

#include "teVector2D.h"
#include "teConstArray.h"
#include "teString.h"

namespace te
{
	//! @brief	Contains code related to bitmap (raster) fonts
	namespace font
	{
		//! @brief		Text horizontal align
		//! @details	Used in font rendering options.
		enum ETextHorizontalAlign
		{
			THA_LEFT,		//!< align text to left border
			THA_CENTER,		//!< align text to center
			THA_RIGHT		//!< align text to right border
		};

		//! @class		teFontRenderingOptions
		//! @brief		Text rendering options
		//! @details	Used in teAssetText.
		//! @see		teAssetText
		struct teFontRenderingOptions
		{
			f32 leading;			//!< Leading in text (proportional distance between lines). Default = 1.0f.
			f32 extraTracking;		//!< Extra tracking between characters. Default = 0.0f.
			f32 size;				//!< Size of text in font pt, use -1 for disable auto font scaling. Default = -1.0f;
			u8 align;				//!< Text align value of ETextHorizontalAlign. Default = THA_LEFT.
			u8 origin;				//!< @brief Text rectangle origin (value of ERectangleOriginCorner). Default = ROC_LEFT_UP. @details Text rendered in axis aligned bounding box rectangle, so this settings set origin of this rectangle.
			u1 ignoreKerning;		//!< If true - ignore kerning data in text rendering. Default = false;
			u1 drawShadow;			//!< If true - draw shadow

			//! @brief	Default constructor
			TE_INLINE teFontRenderingOptions()
				:leading(1.0f), extraTracking(0.0f), align(THA_LEFT), origin(ROC_LEFT_UP), ignoreKerning(false)
			{
			}

			//! @brief	Default constructor
			TE_INLINE teFontRenderingOptions(ETextHorizontalAlign setAlign, ERectangleOriginCorner setOrigin = ROC_LEFT_UP, u1 setIgnoreKerning = false)
				:leading(1.0f), extraTracking(0.0f), align(setAlign), origin(setOrigin), ignoreKerning(setIgnoreKerning)
			{
			}

			//! @brief	Default constructor
			TE_INLINE teFontRenderingOptions(ETextHorizontalAlign setAlign, ERectangleOriginCorner setOrigin, f32 setLeading, f32 setExtraTracking, u1 setIgnoreKerning = false)
				:leading(setLeading), extraTracking(setExtraTracking), align(setAlign), origin(setOrigin), ignoreKerning(setIgnoreKerning)
			{
			}

			//! @brief Clear structure, setup default values.
			TE_INLINE void Clear()
			{
				leading = 1.0f;
				extraTracking = 0.0f;
				size = 0.0f;
				align = THA_LEFT;
				origin = ROC_LEFT_UP;
				ignoreKerning = false;
			}
		};

		//! @class		teCharacterMetrics
		//! @brief		Contains character metric : UTF-32 character code, UV in bitmap font texture, size of character, shifts.
		//! @details	Used in teAssetText.
		struct teCharacterMetrics
		{
			u32 character;				//!< Character UTF-32 code.
			teVector2df uv1;			//!< Left-top UV on texture.
			teVector2df uv2;			//!< Right-bottom UV on texture.
			teVector2df size;			//!< Size of character in pixels.
			teVector2df shift;			//!< Shift of character glyph from baseline.
			teVector2df shiftCharacter;	//!< Shift of cursor on baseline.

			//! @brief	Default constructor
			TE_INLINE teCharacterMetrics()
				:character(u32Max)
			{
			}

			//! @brief	Default constructor
			TE_INLINE teCharacterMetrics(u32 setCharacter)
				:character(setCharacter)
			{
			}
		};

		//! @class		teCharacterKerning
		//! @brief		Contains character pair metric : kerning.
		//! @details	Used in teAssetText.
		struct teCharacterKerning
		{
			u32 characterLeft;	//!< Left character UTF-32 code.
			u32 characterRight;	//!< Right character UTF-32 code.
			f32 kerning;		//!< Kerning value.

			//! @brief	Default constructor
			TE_INLINE teCharacterKerning()
				:characterLeft(u32Max), characterRight(u32Max), kerning(0)
			{
			}
		};

		//! @class		teFont
		//! @brief		Contains font params, characters metrics and kernings.
		struct teFont
		{
			u32 materialIndex;		//!< Material index for rendering bitmap font. Use special shader for distance field based fonts.
			f32 height;				//!< Font size in pt. Used for text scaling. See teFontRenderingOptions::size.
			u32 metricsCount;		//!< Count of character metrics in this font.
			u32 kerningsCount;		//!< Count of characters kernings in this font.

			//! @brief		Get total font structure size in bytes
			//! @return u32	Font size in bytes.
			TE_INLINE u32 GetFontDataSize() const
			{
				return sizeof(teFont) + metricsCount * sizeof(teCharacterMetrics) + kerningsCount * sizeof(teCharacterKerning);
			}

			//! @brief Get character metric
			//! @param[in] index index in metrics array
			//! @return teCharacterMetrics& character metrics
			TE_INLINE teCharacterMetrics & GetMetric(u32 index)
			{
				TE_ASSERT(index < metricsCount);
				return *reinterpret_cast<teCharacterMetrics*>((teptr_t)this + sizeof(teFont) + index * sizeof(teCharacterMetrics));
			}

			//! @brief Get character metric
			//! @param[in] index index in metrics array
			//! @return teCharacterMetrics& character metrics
			TE_INLINE const teCharacterMetrics & GetMetric(u32 index) const
			{
				TE_ASSERT(index < metricsCount);
				return *reinterpret_cast<teCharacterMetrics*>((teptr_t)this + sizeof(teFont) + index * sizeof(teCharacterMetrics));
			}

			//! @brief Get characters pair kerning
			//! @param[in] index index in kernings array
			//! @return teCharacterKerning& characters pair kerning
			TE_INLINE teCharacterKerning & GetKerning(u32 index)
			{
				TE_ASSERT(index < kerningsCount);
				return *reinterpret_cast<teCharacterKerning*>((teptr_t)this + sizeof(teFont) + metricsCount * sizeof(teCharacterMetrics) + index * sizeof(teCharacterKerning));
			}

			//! @brief Get characters pair kerning
			//! @param[in] index index in kernings array
			//! @return teCharacterKerning& characters pair kerning
			TE_INLINE const teCharacterKerning & GetKerning(u32 index) const
			{
				TE_ASSERT(index < kerningsCount);
				return *reinterpret_cast<teCharacterKerning*>((teptr_t)this + sizeof(teFont) + metricsCount * sizeof(teCharacterMetrics) + index * sizeof(teCharacterKerning));
			}

			//! @brief Get character metric index
			//! @param[in] character UTF-32 code
			//! @return u32 index in metrics array, or u32Max if cant find character
			TE_INLINE u32 GetMetricForCharacter(u32 character) const
			{
				if(!metricsCount)
					return u32Max;

				// fix loading bad fonts, needs add sorting to fnt loader
				// replace binary search for linear
				for(u32 i = 0; i < metricsCount; ++i)
					if(GetMetric(i).character == character)
						return i;
				return u32Max;

				/*
				u32 first = 0;
				u32 last = metricsCount - 1;
				u32 mid;

				while(first < last)
				{
					mid = first + (last - first) / 2;

					if(character <= GetMetric(mid).character)
						last = mid;
					else
						first = mid + 1;
				}

				if(GetMetric(first).character == character)
					return first;
				else
					return u32Max;
					*/
			}

			//! @brief Get characters pair kerning value
			//! @param[in] characterLeft left character UTF-32 code
			//! @param[in] characterRight right character UTF-32 code
			//! @return f32 characters pair kerning value, or 0.0f if cant find kerning for this pair
			TE_INLINE f32 GetKerningForCharacters(u32 characterLeft, u32 characterRight) const
			{
				if(!kerningsCount)
					return 0.0f;

				// fix loading bad fonts, needs add sorting to fnt loader
				// replace binary search for linear
				for(size_t i = 0; i < kerningsCount; ++i)
					if((GetKerning(i).characterLeft == characterLeft) && (GetKerning(i).characterRight == characterRight))
						return GetKerning(i).kerning;
				return 0.0f;

				/*
				u32 first = 0;
				u32 last = kerningsCount - 1;
				u32 mid;

				u64 val = characterRight | ((u64)characterLeft << 32);

				while(first < last)
				{
					mid = first + (last - first) / 2;
					
					if(val <= (GetKerning(mid).characterRight | ((u64)GetKerning(mid).characterLeft << 32)))
						last = mid;
					else
						first = mid + 1;
				}

				if((GetKerning(mid).characterLeft == characterLeft) && (GetKerning(mid).characterRight == characterRight))
					return GetKerning(first).kerning;
				else
					return 0;
					*/
			}
		};
	}
}

#endif
