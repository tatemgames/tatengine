/*
 *  teFrameBuffer.cpp
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 11/27/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#include "teFrameBuffer.h"
#include "teRenderGLExtensions.h"
#include "teLogManager.h"

namespace te
{
	namespace video
	{
		#ifdef TE_RENDER_GL

		void teFrameBuffer::Bind() const
		{
			tglBindFramebuffer(frameBufferId);
		}

		void teFrameBuffer::UnBind() const
		{
			tglBindFramebuffer(0);
		}

		void teFrameBuffer::Init()
		{
			for(u32 i = 0; i < FBL_MAX; ++i)
				textures[i] = NULL;
			tglGenFramebuffers(1, &frameBufferId);
		}

		void teFrameBuffer::Deinit()
		{
			tglDeleteFramebuffers(1, &frameBufferId);
			frameBufferId = 0;
		}

		void teFrameBuffer::SetTexture(EFrameBufferLayer layer, teTexture * texture, s32 level)
		{
			GLenum layerGL;

			switch(layer)
			{
			case FBL_COLOR_0: layerGL = GL_COLOR_ATTACHMENT0; break;
			case FBL_DEPTH: layerGL = GL_DEPTH_ATTACHMENT; break;
			default: TE_ASSERT(0); break;
			}

			texture->Bind();

			switch(texture->textureType)
			{
			case TT_2D: tglFramebufferTexture2D(layerGL, GL_TEXTURE_2D, texture->textureId, level); break;
			case TT_RENDERBUFFER: tglFramebufferRenderbuffer(layerGL, GL_RENDERBUFFER, texture->textureId); break;
			default: TE_ASSERT(0); break;
			}

			textures[layer] = texture;
		}

		u1 teFrameBuffer::IsValid() const
		{
			if(tglCheckFramebufferStatus() != GL_FRAMEBUFFER_COMPLETE)
			{
				TE_LOG_ERR("failed to make complete framebuffer object, opengl error : %x", tglCheckFramebufferStatus());
				return false;
			}
			else
				return true;
		}

		#endif

		#ifdef TE_RENDER_D3D9

		void teFrameBuffer::Bind() const
		{
		}

		void teFrameBuffer::UnBind() const
		{
		}

		void teFrameBuffer::Init()
		{
			for(u32 i = 0; i < FBL_MAX; ++i)
				textures[i] = NULL;
		}

		void teFrameBuffer::Deinit()
		{
		}

		void teFrameBuffer::SetTexture(EFrameBufferLayer layer, teTexture * texture, s32 level)
		{
			textures[layer] = texture;
		}

		u1 teFrameBuffer::IsValid() const
		{
			return false;
		}

		#endif
	}
}
