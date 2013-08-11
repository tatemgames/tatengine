/*
 *  teTexture.cpp
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 11/25/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#include "teTexture.h"
#include "teRender.h"
#include "teRenderGLExtensions.h"
#include "teLogManager.h"

namespace te
{
	namespace video
	{
		#ifdef TE_RENDER_GL

		void teTexture::Bind(u32 layer) const
		{
			switch(textureType)
			{
			case TT_2D: tglActiveTexture(GL_TEXTURE0 + layer); glBindTexture(GL_TEXTURE_2D, textureId); break;
			case TT_RENDERBUFFER: tglBindRenderbuffer(textureId); break;
			default: TE_ASSERT(0); break;
			}
		}

		void teTexture::Init(ETextureType setTextureType, u32 setFlags, image::teImage * image, ERenderBufferType renderBufferType, const teVector2duh & renderBufferSize)
		{
			textureType = setTextureType;
			flags = setFlags;

			#ifdef TE_RENDER_FORCE_TEX_FILTERED
				flags |= TF_FILTERED;
			#endif

			switch(textureType)
			{
			case TT_2D: glGenTextures(1, &textureId); break;
			case TT_RENDERBUFFER: tglGenRenderbuffers(1, &textureId); break;
			default: TE_ASSERT(0); break;
			}

			Bind();

			if(flags & TF_FILTERED)
			{
				if(flags & TF_MIPMAP)
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				else
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			}
			else
			{
				if(flags & TF_MIPMAP)
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
				else
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			}

			if(flags & TF_REPEAT)
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			}
			else
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}

			if((textureType == TT_2D) && image)
			{
				if(image->levelsCount > 0)
					size.SetXY(image->levels[0].width, image->levels[0].height);

				for(u32 i = 0; i < image->levelsCount; ++i)
				{
					GLuint format = GetTextureInternalFormat((image::EImagePixelFormat)image->pixelFormat);

					#ifdef TE_RENDER_PVRTC
					if(image->pixelFormat >= image::IPF_COMPRESSED)
						glCompressedTexImage2D(GL_TEXTURE_2D, i, format, image->levels[i].width, image->levels[i].height, 0, image->levels[i].bytes, image->GetPixels(i));
					else
					#endif
						glTexImage2D(GL_TEXTURE_2D, i, format, image->levels[i].width, image->levels[i].height, 0, format, GL_UNSIGNED_BYTE, image->GetPixels(i));
				}

				if(flags & TF_MIPMAP)
					tglGenerateMipmap(GL_TEXTURE_2D);
			}
			else if((renderBufferType != RBT_NONE) && (renderBufferSize.GetMaxComponent() > 0))
			{
				size.SetXY(renderBufferSize.x, renderBufferSize.y);

				GLenum t0 = GetRenderBufferInternalType(renderBufferType);
				GLenum t1 = GL_NONE;
				GLenum t2 = GL_NONE;

				#if defined(TE_PLATFORM_WIN) || defined(TE_PLATFORM_MAC)
				switch(t0)
				{
				case GL_RGBA8:
					//t0 = GL_RGBA32F;
					t1 = GL_RGBA;
					t2 = GL_UNSIGNED_BYTE;
					break;
				case GL_DEPTH_COMPONENT24:
					//t0 = GL_DEPTH_COMPONENT24;
					t1 = GL_DEPTH_COMPONENT;
					t2 = GL_UNSIGNED_INT;
					break;
				}
				#elif defined(TE_PLATFORM_IPHONE)
				switch(t0)
				{
					case GL_RGBA8_OES:
						t0 = GL_RGBA;
						t1 = GL_RGBA;
						t2 = GL_UNSIGNED_BYTE;
						break;
					case GL_DEPTH_COMPONENT24_OES:
						t0 = GL_DEPTH_COMPONENT;
						t1 = GL_DEPTH_COMPONENT;
						t2 = GL_UNSIGNED_INT;
						break;
				}
				#endif

				glTexImage2D(GL_TEXTURE_2D, 0, t0, renderBufferSize.x, renderBufferSize.y, 0, t1, t2, 0);

				//printf("-%x-\n", glGetError());
			}
			else if((textureType == TT_RENDERBUFFER) && (renderBufferType != RBT_NONE) && (renderBufferSize.GetMaxComponent() > 0))
			{
				size.SetXY(renderBufferSize.x, renderBufferSize.y);
				tglRenderbufferStorage(GetRenderBufferInternalType(renderBufferType), renderBufferSize.x, renderBufferSize.y);
			}
		}

		void teTexture::Deinit()
		{
			switch(textureType)
			{
			case TT_2D: glDeleteTextures(1, &textureId); break;
			case TT_RENDERBUFFER: tglDeleteRenderbuffers(1, &textureId); break;
			default: TE_ASSERT(0); break;
			}

			textureId = 0;
		}

		void teTexture::UpdateSize()
		{
			switch(textureType)
			{
			case TT_2D:
				// TODO
				break;
			case TT_RENDERBUFFER:
				{
					GLint width, height;
					tglGetRenderbufferParameteriv(GL_RENDERBUFFER_WIDTH, &width);
					tglGetRenderbufferParameteriv(GL_RENDERBUFFER_HEIGHT, &height);
					size.x = width;
					size.y = height;
					break;
				}
			default: TE_ASSERT(0); break;
			}
		}

		#endif

		#ifdef TE_RENDER_D3D9

		void teTexture::Bind(u32 layer) const
		{
			GetRender()->GetDevice()->SetTexture(layer, texture);
		}

		void teTexture::Init(ETextureType setTextureType, u32 setFlags, image::teImage * image, ERenderBufferType renderBufferType, const teVector2duh & renderBufferSize)
		{
			textureType = setTextureType;
			flags = setFlags;

			GetRender()->GetDevice()->CreateTexture(image->levels[0].width, image->levels[0].height, image->levelsCount, 0, GetTextureInternalFormat((image::EImagePixelFormat)image->pixelFormat), D3DPOOL_MANAGED, &texture, 0);

			for(u8 i = 0; i < image->levelsCount; ++i)
			{
				D3DLOCKED_RECT rect;
				texture->LockRect(i, &rect, NULL, 0);
				memcpy(rect.pBits, image->GetPixels(0), image->levels[i].bytes);
				texture->UnlockRect(i);
			}
		}

		void teTexture::Deinit()
		{
			texture->Release();
		}

		void teTexture::UpdateSize()
		{
			LPDIRECT3DSURFACE9 surface;
			texture->GetSurfaceLevel(0, &surface);

			D3DSURFACE_DESC desc;
			surface->GetDesc(&desc);
			size.x = desc.Width;
			size.y = desc.Height;

			surface->Release();
		}

		#endif
	}
}
