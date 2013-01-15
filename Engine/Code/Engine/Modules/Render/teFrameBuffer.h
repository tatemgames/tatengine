/*
 *  teFrameBuffer.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 11/27/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEFRAMEBUFFER_H
#define TE_TEFRAMEBUFFER_H

#include "teTypes.h"
#include "teReferenceCounter.h"
#include "teRenderSystem.h"
#include "teTexture.h"

namespace te
{
	namespace video
	{
		enum EFrameBufferLayer
		{
			FBL_COLOR_0 = 0,
			FBL_DEPTH,
			FBL_MAX
		};
		
		struct teFrameBuffer
		{
			teTexture * textures[FBL_MAX];

			#if defined(TE_RENDER_GL)
			GLuint frameBufferId;
			#elif defined(TE_RENDER_D3D9)

			#endif

			void Bind() const;
			void UnBind() const;

			void Init();
			void Deinit();
			void SetTexture(EFrameBufferLayer layer, teTexture * texture, s32 level = 0);
			
			u1 IsValid() const;
		};
	}
}

#endif