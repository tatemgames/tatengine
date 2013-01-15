/*
 *  teTexture.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 11/25/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TETEXTURE_H
#define TE_TETEXTURE_H

#include "teReferenceCounter.h"
#include "teRenderSystem.h"
#include "teVector2D.h"
#include "teImage.h"

namespace te
{
	namespace video
	{
		#if defined(TE_RENDER_GL)

		TE_FUNC GLuint GetTextureInternalFormat(image::EImagePixelFormat pixelFormat)
		{
			switch(pixelFormat)
			{
			case image::IPF_ALPHA: return GL_ALPHA;
			case image::IPF_LUMINANCE: return GL_LUMINANCE;
			case image::IPF_LUMINANCE_ALPHA: return GL_LUMINANCE_ALPHA;
			case image::IPF_RGB: return GL_RGB;
			case image::IPF_RGBA: return GL_RGBA;

			#ifdef TE_RENDER_PVRTC
			case image::IPF_RGB_PVRTC_2: return GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
			case image::IPF_RGBA_PVRTC_2: return GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
			case image::IPF_RGB_PVRTC_4: return GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
			case image::IPF_RGBA_PVRTC_4: return GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
			#endif

			default: TE_ASSERT(0); return 0;
			}
		}

		#elif defined(TE_RENDER_D3D9)

		TE_FUNC D3DFORMAT GetTextureInternalFormat(image::EImagePixelFormat pixelFormat)
		{
			switch(pixelFormat)
			{
			case image::IPF_ALPHA: return D3DFMT_A8;
			case image::IPF_LUMINANCE: return D3DFMT_L8;
			case image::IPF_LUMINANCE_ALPHA: return D3DFMT_A8L8;
			case image::IPF_RGB: return D3DFMT_R8G8B8;
			case image::IPF_RGBA: return D3DFMT_A8R8G8B8;
			default: TE_ASSERT(0); return D3DFMT_A8R8G8B8;
			}
		}

		#endif

		enum ERenderBufferType
		{
			RBT_NONE = 0,
			RBT_RGBA8,
			RBT_D24
		};

		#if defined(TE_RENDER_GL)

		TE_FUNC GLuint GetRenderBufferInternalType(ERenderBufferType renderBufferType)
		{
			switch(renderBufferType)
			{
			case RBT_NONE: return GL_NONE;
			#if defined (TE_PLATFORM_WIN) || defined (TE_PLATFORM_MAC)
			case RBT_RGBA8: return GL_RGBA8;
			case RBT_D24: return GL_DEPTH_COMPONENT24;
			#elif defined(TE_PLATFORM_IPHONE)
			case RBT_RGBA8: return GL_RGBA8_OES;
			case RBT_D24: return GL_DEPTH_COMPONENT24_OES;
			#endif
			default: TE_ASSERT(0); return 0;
			}
		}

		#elif defined(TE_RENDER_D3D9)

		/*
		TE_FUNC GLuint GetRenderBufferInternalType(ERenderBufferType renderBufferType)
		{
			switch(renderBufferType)
			{
			case RBT_NONE: return GL_NONE;
			case RBT_RGBA8: return GL_RGBA8;
			case RBT_D24: return GL_DEPTH_COMPONENT24;
			default: TE_ASSERT(0); return 0;
			}
		}
		*/

		#endif

		enum ETextureType
		{
			TT_2D,
			TT_RENDERBUFFER
		};

		enum ETextureFlags
		{
			TF_DEFAULT = 0x1,
			TF_FILTERED = 0x2,
			TF_REPEAT = 0x4,
			TF_MIPMAP = 0x8
		};

		struct teTexture
		{
			#if defined(TE_RENDER_GL)
			GLuint textureId;
			#elif defined(TE_RENDER_D3D9)
			LPDIRECT3DTEXTURE9 texture;
			#endif
			u32 flags;
			teVector2duh size;
			u8 textureType;

			void Bind(u32 layer = 0) const;

			void Init(ETextureType setTextureType, u32 setFlags, image::teImage * image = NULL, ERenderBufferType renderBufferType = RBT_NONE, const teVector2duh & renderBufferSize = teVector2duh(0, 0));
			void Deinit();
			void UpdateSize();
		};
	}
}

#endif
