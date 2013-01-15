/*
 *  teRenderContext.mm
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 11/27/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#include "teRenderContext.h"
#include "teMemoryManager.h"
#include "teLogManager.h"

#ifdef TE_PLATFORM_IPHONE

#include "teRenderGLExtensions.h"

#include <OpenGLES/EAGL.h>
#include <OpenGLES/EAGLDrawable.h>

namespace te
{
	namespace video
	{
		//! Context Device struct
		struct teRenderContext::teRenderContextDevice
		{
			//! Drawable context
			EAGLContext * DrawableContext;
			
			//! Constructor
			teRenderContextDevice()
				:DrawableContext(NULL)
			{
			}
			
			//! Destructor
			~teRenderContextDevice()
			{
			}
		};

		//! Constructor
		teRenderContext::teRenderContext()
			#ifdef TE_OPENGL_ES_11
			:CurrentVersion(GAPI_OGL_ES_11),
			#else
			:CurrentVersion(GAPI_OGL_ES_20),
			#endif
			ContextDevice(NULL)
		{
			TE_NEW(ContextDevice, teRenderContextDevice)

			switch(CurrentVersion)
			{
				#ifdef TE_OPENGL_ES_11
				case GAPI_OGL_ES_11:
					ContextDevice->DrawableContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
					break;
				#endif
				#ifdef TE_OPENGL_ES_20
				case GAPI_OGL_ES_20:
					ContextDevice->DrawableContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
					break;
				#endif
				default:
					break;
			}
			
			if(!ContextDevice->DrawableContext)
			{
				//TE_LOG_E(teStringc("Cant create OpenGL context version ") + teStringc(CurrentVersion))
				TE_DELETE(ContextDevice)
				ContextDevice = NULL;
			}

			#ifdef TE_RENDER_CONTEXT_SUPPORT_DEVICE_ORIENTATION
				OnInitVirtualOrientation();
			#endif
		}

		//! Constructor
		teRenderContext::teRenderContext(EGAPIVersion Version)
			:CurrentVersion(Version), ContextDevice(NULL)
		{
			TE_NEW(ContextDevice, teRenderContextDevice)
			
			switch(CurrentVersion)
			{
			#ifdef TE_OPENGL_ES_11
			case GAPI_OGL_ES_11:
				ContextDevice->DrawableContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
				break;
			#endif
			#ifdef TE_OPENGL_ES_20
			case GAPI_OGL_ES_20:
				ContextDevice->DrawableContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
				break;
			#endif
			default:
				break;
			}
			
			if(!ContextDevice->DrawableContext)
			{
				//TE_LOG_E(teStringc("Cant create OpenGL context version ") + teStringc(CurrentVersion))
				TE_DELETE(ContextDevice)
				ContextDevice = NULL;
			}

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

			if(ContextDevice)
			{
				if([EAGLContext currentContext] == ContextDevice->DrawableContext)
					[EAGLContext setCurrentContext:nil];
				
				[ContextDevice->DrawableContext release];
				
				TE_DELETE(ContextDevice)
			}
		}
			
		//! Bind
		void teRenderContext::Bind()
		{
			[EAGLContext setCurrentContext:ContextDevice->DrawableContext];
		}
			
		//! UnBind
		void teRenderContext::UnBind()
		{
		}
			
		//! Is Complete
		u1 teRenderContext::IsComplete()
		{
			return ContextDevice;
		}
		
		//! Present Texture
		void teRenderContext::PresentCurrentTexture()
		{
			[ContextDevice->DrawableContext presentRenderbuffer:GL_RENDERBUFFER];
		}
		
		//! Get GLContext
		void * teRenderContext::GetGLContext()
		{
			return ContextDevice->DrawableContext;
		}
	}
}

#elif defined(TE_PLATFORM_MAC)

namespace te
{
	namespace video
	{
		//! Constructor
		teRenderContext::teRenderContext()
			#if defined(TE_OPENGL_15) && !defined(TE_OPENGL_21)
				:CurrentVersion(GAPI_OGL_15),
			#else
				:CurrentVersion(GAPI_OGL_21),
			#endif
			ContextDevice(NULL)
		{
            #ifdef TE_RENDER_CONTEXT_SUPPORT_DEVICE_ORIENTATION
                OnInitVirtualOrientation();
            #endif
		}

		//! Constructor
		teRenderContext::teRenderContext(EGAPIVersion Version)
			:CurrentVersion(Version), ContextDevice(NULL)
		{
			switch(CurrentVersion)
			{
			case GAPI_OGL_15:
				break;
			case GAPI_OGL_21:
				break;
			default:
				break;
			}

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
