/*
 *  teFontTools.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 12/20/11.
 *  Copyright 2011 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEFONTTOOLS_H
#define TE_TEFONTTOOLS_H

#include "teFont.h"
#include "IBuffer.h"

namespace te
{
	namespace font
	{
		//! @brief		Load FNT font
		//! @details	Use TE_ALLOCATE for memory allocation for font
		//! @param[in] buffer buffer with font data
		//! @param[in] downscale downscale font metrics for distance field based fonts, default = 1.0f
		//! @return teFont* loaded font, return NULL if failed
		teFont * LoadFontFNT(core::IBuffer * buffer, f32 downscale = 1.0f); // use downscale for loading distance field based fonts, when we get metrics from bigger font than texture

		//! @brief		Sort font metrics and kernings for binary search
		//! @param[in] font
		void SortFontData(teFont * font);
	}
}

#endif