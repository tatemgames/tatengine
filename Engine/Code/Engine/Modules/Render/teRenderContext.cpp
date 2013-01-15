/*
 *  teRenderContext.cpp
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 12/3/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#include "teRenderContext.h"

#if defined(TE_PLATFORM_WIN) || defined(TE_PLATFORM_LINUX_MOBILE) || defined(TE_PLATFORM_ANDROID)

namespace te
{
	namespace video
	{
		#if defined(TE_PLATFORM_WIN) && defined(TE_RENDER_D3D9)
			#define TE_BEST_GAPI GAPI_D3D9
		#elif defined(TE_OPENGL_ES_20)
			#define TE_BEST_GAPI GAPI_OGL_ES_20
		#elif defined(TE_OPENGL_ES_11)
			#define TE_BEST_GAPI GAPI_OGL_ES_11
		#elif defined(TE_OPENGL_21)
			#define TE_BEST_GAPI GAPI_OGL_21
		#elif defined(TE_OPENGL_15)
			#define TE_BEST_GAPI GAPI_OGL_21
		#endif

		//! Constructor
		teRenderContext::teRenderContext()
			:CurrentVersion(TE_BEST_GAPI), ContextDevice(NULL)
		{
			#ifdef TE_RENDER_CONTEXT_SUPPORT_DEVICE_ORIENTATION
				OnInitVirtualOrientation();
			#endif
		}

		//! Constructor
		teRenderContext::teRenderContext(EGAPIVersion Version)
			:CurrentVersion(Version), ContextDevice(NULL)
		{
			#ifdef TE_RENDER_CONTEXT_SUPPORT_DEVICE_ORIENTATION
				OnInitVirtualOrientation();
			#endif
		}
		
		//! Destructor
		teRenderContext::~teRenderContext()
		{
			#ifdef TE_RENDER_CONTEXT_SUPPORT_DEVICE_ORIENTATION
				OnReleaseVirtualOrientation();
			#endif
		}
		
		//! Bind
		void teRenderContext::Bind()
		{
		}
		
		//! UnBind
		void teRenderContext::UnBind()
		{
		}
		
		//! Is Complete
		u1 teRenderContext::IsComplete()
		{
			return true;
		}
		
		//! Present Texture
		void teRenderContext::PresentCurrentTexture()
		{
		}
		
		//! Get GLContext
		void * teRenderContext::GetGLContext()
		{
			return NULL;
		}
	}
}

#endif
