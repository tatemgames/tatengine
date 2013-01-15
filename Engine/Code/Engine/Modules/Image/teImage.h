/*
 *  teImage.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 11/27/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEIMAGE_H
#define TE_TEIMAGE_H

#include "teTypes.h"
#include "teDebug.h"
#include "teVector2D.h"

namespace te
{
	//! @brief	Contains code related to image (bitmap) operations
	namespace image
	{
		//! @brief		Pixel Format
		//! @details	Used to set raw image structure.
		enum EImagePixelFormat
		{
			IPF_INVALID = 0,		//!< invalid pixel format (no pixel format)
			IPF_ALPHA,				//!< 8 bit alpha
			IPF_LUMINANCE,			//!< 8 bit luminance
			IPF_LUMINANCE_ALPHA,	//!< 16 bit luminance + alpha
			IPF_RGB,				//!< 24 bit RGB
			IPF_RGBA,				//!< 32 bit RGBA

			IPF_COMPRESSED,			//!< not format, used as compressed flag (format is compressed if format >= IPF_COMPRESSED)

			IPF_RGB_PVRTC_2 = IPF_COMPRESSED,	//!< RGB PVRTC 2 (only on PowerVR gpu)
			IPF_RGBA_PVRTC_2,					//!< RGBA PVRTC 2 (only on PowerVR gpu)
			IPF_RGB_PVRTC_4,					//!< RGB PVRTC 4 (only on PowerVR gpu)
			IPF_RGBA_PVRTC_4					//!< RGBA PVRTC 4 (only on PowerVR gpu)
		};

		//! @brief		Get pixel format size
		//! @param[in] pixelFormat pixel format
		//! @return u8 size in bytes
		TE_FUNC u8 GetPixelSizeFromFormat(EImagePixelFormat pixelFormat)
		{
			switch(pixelFormat)
			{
				case IPF_ALPHA: return 1;
				case IPF_LUMINANCE: return 1;
				case IPF_LUMINANCE_ALPHA: return 2;
				case IPF_RGB: return 3;
				case IPF_RGBA: return 4;

				case IPF_RGB_PVRTC_2:
				case IPF_RGBA_PVRTC_2:
				case IPF_RGB_PVRTC_4:
				case IPF_RGBA_PVRTC_4:
				default: TE_ASSERT(0); return 0;
			}
		}

		//! @class teImageLevel
		//! @brief	Image mipmap level structure
		//! @details Contains offset, size in bytes, width and height. Used in teImage.
		struct teImageLevel
		{
			u32 offset;		//!< offset in pixels buffer (stars after levels array in teImage)
			u32 bytes;		//!< raw data size in bytes
			u16 width;		//!< mipmap level width
			u16 height;		//!< mipmap level height
		};

		//! @class teImage
		//! @brief	Image structure
		//! @details Contains mipmap levels, and pixel format information.
		struct teImage
		{
		public:
			u8 pixelFormat;			//!< pixel format, contains value of EImagePixelFormat type
			u8 levelsCount;			//!< mipmap levels count
			u32 pixelsBytes;		//!< total size of all image raw data in bytes
			teImageLevel levels[];	//!< mipmap levels information

			//! @brief		Construct teImage
			//! @details	Use for in-place construction.
			//! @param[in] setPixelFormat setup pixel format
			//! @param[in] setLevelsCount set mip map levels count (from 1 to max - defined as log2(width or height))
			//! @param[in] setPixelsBytes set total size of all pixels data, needed for teContentPack - for defining image size in memory
			TE_INLINE teImage(EImagePixelFormat setPixelFormat = IPF_INVALID, u8 setLevelsCount = 1, u32 setPixelsBytes = 0):pixelFormat(setPixelFormat), levelsCount(setLevelsCount), pixelsBytes(setPixelsBytes) {}

			//! @brief		Return image size in bytes
			//! @details	Return total size of image in memory.
			//! @return u32 size in bytes
			TE_INLINE u32 GetImageDataSize() const
			{
				return sizeof(teImage) + levelsCount * sizeof(teImageLevel) + pixelsBytes;
			}

			//! @brief		Return raw bitmap pixels
			//! @param[in] level mipmap level, by default = 0
			//! @return u8* bitmap pixels
			TE_INLINE u8 * GetPixels(u8 level = 0)
			{
				return reinterpret_cast<u8*>(((uintptr_t)this) + sizeof(teImage) + levelsCount * sizeof(teImageLevel) + levels[level].offset);
			}

			//! @brief		Return raw bitmap pixels
			//! @param[in] level mipmap level, by default = 0
			//! @return u8* bitmap pixels
			TE_INLINE const u8 * GetPixels(u8 level = 0) const
			{
				return reinterpret_cast<const u8*>(((uintptr_t)this) + sizeof(teImage) + levelsCount * sizeof(teImageLevel) + levels[level].offset);
			}
		};
	}
}

#endif
