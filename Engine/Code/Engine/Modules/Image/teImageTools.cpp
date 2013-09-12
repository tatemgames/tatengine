/*
 *  teImageTools.cpp
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 01/20/12.
 *  Copyright 2012 Tatem Games. All rights reserved.
 *
 */

#include "teImageTools.h"
#include "teLogManager.h"
#include "teColor.h"
#include "png.h"
#include "pngstruct.h"

namespace te
{
	namespace image
	{
		teImage * LoadTGA(core::IBuffer * buffer)
		{
			#pragma pack(push, 1)

			struct teTGAHeader
			{
				u8 identSize;
				u8 colourMapType;
				u8 imageType;
				u16 colourMapStart;
				u16 colourMapLength;
				u8 colourMapBits;
				u16 xStart;
				u16 yStart;
				u16 width;
				u16 height;
				u8 bitsPerPixel;
				u8 descriptor;
			};

			#pragma pack(pop)

			TE_ASSERT(buffer);

			teTGAHeader header;
			buffer->Lock(core::BLT_READ);
			buffer->SetStreamMode(true);
			buffer->SetPosition(0);

			buffer->Read(&header, sizeof(teTGAHeader));

			TE_ASSERT((header.colourMapType == 0) && (header.imageType == 2 || header.imageType == 3));

			EImagePixelFormat pixelFormat;

			if(header.imageType == 3)
				pixelFormat = IPF_LUMINANCE;
			else
			{
				switch(header.bitsPerPixel)
				{
				case 16: pixelFormat = IPF_LUMINANCE_ALPHA; break;
				case 24: pixelFormat = IPF_RGB; break;
				case 32: pixelFormat = IPF_RGBA; break;
				}
			}

			u32 pixelsBytes = header.width * header.height * GetPixelSizeFromFormat(pixelFormat);

			teImage * image = (teImage*)TE_ALLOCATE(sizeof(teImage) + sizeof(teImageLevel) + pixelsBytes);

			image->pixelFormat = pixelFormat;
			image->levelsCount = 1;
			image->pixelsBytes = pixelsBytes;

			image->levels[0].offset = 0;
			image->levels[0].bytes = pixelsBytes;
			image->levels[0].width = header.width;
			image->levels[0].height = header.height;

			buffer->Read(image->GetPixels(), pixelsBytes);

			buffer->Unlock();

			return image;
		}

		/*
		teImage * LoadDDS(core::IBuffer * buffer)
		{
			return NULL;
		}
		 */

		teImage * LoadPVR(core::IBuffer * buffer)
		{
			#pragma pack(push, 1)
			
			struct tePVRHeader
			{
				u32 headerLength;
				u32 height;
				u32 width;
				u32 numMipmaps;
				u32 flags;
				u32 dataLength;
				u32 bpp;
				u32 bitmaskRed;
				u32 bitmaskGreen;
				u32 bitmaskBlue;
				u32 bitmaskAlpha;
				u32 pvrTag;
				u32 numSurfs;
			};
			
			#pragma pack(pop)
			
			enum EPVRTextureType
			{
				PTT_PVRTC_2 = 24,
				PTT_PVRTC_4
			};
			
			TE_ASSERT(buffer);
			
			tePVRHeader header;
			buffer->Lock(core::BLT_READ);
			buffer->SetStreamMode(true);
			buffer->SetPosition(0);
			
			buffer->Read(&header, sizeof(tePVRHeader));
			
			u32 formatFlags = header.flags & 0xff;
			
			EImagePixelFormat pixelFormat = (formatFlags == PTT_PVRTC_4) ? (header.bitmaskAlpha ? IPF_RGBA_PVRTC_4 : IPF_RGB_PVRTC_4) : (header.bitmaskAlpha ? IPF_RGBA_PVRTC_2 : IPF_RGB_PVRTC_2);
			
			u32 pixelsBytes = (header.dataLength ? header.dataLength : header.headerLength - sizeof(header));
			
			if(header.numMipmaps == 0)
				header.numMipmaps = 1;
			
			teImage * image = (teImage*)TE_ALLOCATE(sizeof(teImage) + sizeof(teImageLevel) * header.numMipmaps + pixelsBytes);
			
			image->pixelFormat = pixelFormat;
			image->levelsCount = header.numMipmaps;
			image->pixelsBytes = pixelsBytes;
			
			teVector2duh size(header.width, header.height);
			
			for(u8 i = 0; i < header.numMipmaps; ++i)
			{
				u32 bytes = size.x * size.y;
				
				switch(pixelFormat)
				{
				case IPF_RGB_PVRTC_2: bytes /= 4; break;
				case IPF_RGBA_PVRTC_2: bytes /= 4; break;
				case IPF_RGB_PVRTC_4: bytes /= 2; break;
				case IPF_RGBA_PVRTC_4: bytes /= 2; break;
				default:
					continue;
				}
				
				if(bytes < 32)
					bytes = 32;
				
				if(i > 0)
					image->levels[i].offset = image->levels[i - 1].offset + image->levels[i - 1].bytes;
				else
					image->levels[i].offset = 0;

				image->levels[i].bytes = bytes;
				image->levels[i].width = size.x;
				image->levels[i].height = size.y;
				
				size /= 2;
			}
			
			buffer->Read(image->GetPixels(), pixelsBytes);
			buffer->Unlock();

			return image;
		}

		void LibPngError(png_structp png_ptr, png_const_charp msg)
		{
			TE_LOG_ERR("libpng fatal error : %s", msg);
			longjmp(png_ptr->png_jmpbuf, 1);
		}

		void LibPngRead(png_structp png_ptr, png_bytep data, png_size_t length) {core::IBuffer * buffer = static_cast<core::IBuffer*>(png_ptr->io_ptr); buffer->Read(data, sizeof(c8) * length);}

		teImage * LoadPNG(core::IBuffer * buffer)
		{
			buffer->Lock(core::BLT_READ);
			buffer->SetStreamMode(true);
			buffer->SetPosition(0);

			png_byte pngHeader[8];
			buffer->Read(pngHeader, 8 * sizeof(png_byte));

			if(png_sig_cmp(pngHeader, 0, 8))
			{
				TE_LOG_ERR("wrong png header");
				buffer->Unlock();
				return NULL;
			}

			png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, (png_error_ptr)LibPngError, NULL);
			if (!png_ptr)
			{
				TE_LOG_ERR("libpng error");
				buffer->Unlock();
				return NULL;
			}

			png_infop info_ptr = png_create_info_struct(png_ptr);
			if (!info_ptr)
			{
				TE_LOG_ERR("libpng error");
				buffer->Unlock();
				return NULL;
			}

			if (setjmp((*png_set_longjmp_fn((png_ptr), (png_longjmp_ptr)longjmp, sizeof(jmp_buf)))))
			{
				TE_LOG_ERR("libpng error");
				png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
				buffer->Unlock();
				return NULL;
			}

			png_set_read_fn(png_ptr, buffer, LibPngRead);
			png_set_sig_bytes(png_ptr, 8);
			png_read_info(png_ptr, info_ptr);

			teVector2du size;
			s32 bitDepth;
			s32 colorType;

			png_get_IHDR(png_ptr, info_ptr, reinterpret_cast<png_uint_32*>(&size.x), reinterpret_cast<png_uint_32*>(&size.y), &bitDepth, &colorType, NULL, NULL, NULL);

			if(colorType == PNG_COLOR_TYPE_PALETTE)
				png_set_palette_to_rgb(png_ptr);

			if(bitDepth < 8)
			{
				if((colorType == PNG_COLOR_TYPE_GRAY) || (colorType == PNG_COLOR_TYPE_GRAY_ALPHA))
					png_set_gray_to_rgb(png_ptr);
				else
					png_set_packing(png_ptr);
			}

			if(png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
				png_set_tRNS_to_alpha(png_ptr);

			if(bitDepth == 16)
				png_set_strip_16(png_ptr);

			if((colorType == PNG_COLOR_TYPE_GRAY) || (colorType == PNG_COLOR_TYPE_GRAY_ALPHA))
				png_set_gray_to_rgb(png_ptr);

			if(colorType == PNG_COLOR_TYPE_RGB)
				png_set_add_alpha(png_ptr, 0xFF, PNG_FILLER_AFTER);

			png_read_update_info(png_ptr, info_ptr);
			png_get_IHDR(png_ptr, info_ptr, reinterpret_cast<png_uint_32*>(&size.x), reinterpret_cast<png_uint_32*>(&size.y), &bitDepth, &colorType, NULL, NULL, NULL);

			EImagePixelFormat pixelFormat = IPF_INVALID;

			switch(colorType)
			{
			case PNG_COLOR_TYPE_GRAY: pixelFormat = IPF_LUMINANCE; break;
			case PNG_COLOR_TYPE_GRAY_ALPHA: pixelFormat = IPF_LUMINANCE_ALPHA; break;
			//case PNG_COLOR_TYPE_RGB: pixelFormat = IPF_RGB; break;
			case PNG_COLOR_TYPE_RGB_ALPHA: pixelFormat = IPF_RGBA; break;
			default: TE_ASSERT(0); break;
			}

			u32 pixelsBytes = size.x * size.y * GetPixelSizeFromFormat(pixelFormat);

			teImage * image = (teImage*)TE_ALLOCATE(sizeof(teImage) + sizeof(teImageLevel) + pixelsBytes);

			image->pixelFormat = pixelFormat;
			image->levelsCount = 1;
			image->pixelsBytes = pixelsBytes;

			image->levels[0].offset = 0;
			image->levels[0].bytes = pixelsBytes;
			image->levels[0].width = size.x;
			image->levels[0].height = size.y;

			u8 ** rowPointers = NULL;
			TE_NEW_A(rowPointers, png_bytep, size.y)

			for(u32 i = 0; i < size.y; ++i)
				//rowPointers[i] = image->GetPixels(0) + (size.y - 1 - i) * size.x * GetPixelSizeFromFormat(pixelFormat);
				rowPointers[i] = image->GetPixels(0) + i * size.x * GetPixelSizeFromFormat(pixelFormat);

			if(setjmp((*png_set_longjmp_fn((png_ptr), (png_longjmp_ptr)longjmp, sizeof(jmp_buf)))))
			{
				png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
				TE_DELETE_A(rowPointers)

				TE_FREE(image);

				buffer->Unlock();
				return NULL;
			}

			png_read_image(png_ptr, rowPointers);
			png_read_end(png_ptr, NULL);

			buffer->Unlock();
			TE_DELETE_A(rowPointers)
			png_destroy_read_struct(&png_ptr, &info_ptr, 0);

			return image;
		}

		void SaveTGA(core::IBuffer * buffer, teImage * image)
		{
			#pragma pack(push, 1)

			struct teTGAHeader
			{
				u8 identSize;
				u8 colourMapType;
				u8 imageType;
				u16 colourMapStart;
				u16 colourMapLength;
				u8 colourMapBits;
				u16 xStart;
				u16 yStart;
				u16 width;
				u16 height;
				u8 bitsPerPixel;
				u8 descriptor;

				void Set(u16 setWidth, u16 setHeight, u8 pixelFormat)
				{
					identSize = 0;

					colourMapType = 0;
					imageType = 2;

					colourMapStart = 0;
					colourMapLength = 0;
					colourMapBits =0;

					xStart = 0;
					yStart = 0;
					width = setWidth;
					height = setHeight;

					if(pixelFormat == IPF_RGB)
						bitsPerPixel = 24;
					else if(pixelFormat == IPF_RGBA)
						bitsPerPixel = 32;
					else
						TE_ASSERT(0);

					descriptor = 32; // 00000100 bit
					

				}
			};

			#pragma pack(pop)

			teTGAHeader header;
			header.Set(image->levels[0].width, image->levels[0].height, image->pixelFormat);

			if(image->pixelFormat == IPF_RGBA)
			{
				teColor4u temp;
				for(u32 i = 0; i < (u32)(image->levels[0].width * image->levels[0].height); ++i)
				{
					teColor4u & color = *reinterpret_cast<teColor4u*>(image->GetPixels() + i * sizeof(teColor4u));
					temp = color;
					//color.SetRGBA(temp.a, temp.r, temp.g, temp.b);
				}
			}

			buffer->Lock(core::BLT_WRITE);
			buffer->Allocate(sizeof(teTGAHeader) + image->levels[0].bytes);
			buffer->SetStreamMode(true);
			buffer->SetPosition(0);
			buffer->Write(&header, sizeof(teTGAHeader));
			buffer->Write(image->GetPixels(), image->levels[0].bytes);
			buffer->Unlock();

			if(image->pixelFormat == IPF_RGBA)
			{
				teColor4u temp;
				for(u32 i = 0; i < (u32)(image->levels[0].width * image->levels[0].height); ++i)
				{
					teColor4u & color = *reinterpret_cast<teColor4u*>(image->GetPixels() + i * sizeof(teColor4u));
					temp = color;
					//color.SetRGBA(temp.g, temp.b, temp.a, temp.r);
				}
			}
		}
	}
}
