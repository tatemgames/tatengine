/*
 *  teRenderSystem.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 11/27/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TERENDERSYSTEM_H
#define TE_TERENDERSYSTEM_H

#include "TatEngineCoreConfig.h"

#if defined(TE_PLATFORM_LINUX_MOBILE) || defined(TE_PLATFORM_ANDROID)
	typedef char GLchar;
#endif

#ifdef TE_OPENGL_ES_11
	#if defined(TE_PLATFORM_LINUX_MOBILE) || defined(TE_PLATFORM_ANDROID)
		#include <GLES/gl.h>
		#define GL_GLEXT_PROTOTYPES
		#include <GLES/glext.h>
	#else
		#include <OpenGLES/ES1/gl.h>
		#define GL_GLEXT_PROTOTYPES
		#include <OpenGLES/ES1/glext.h>
	#endif
#endif

#ifdef TE_OPENGL_ES_20
	#if defined(TE_PLATFORM_LINUX_MOBILE) || defined(TE_PLATFORM_ANDROID)
		#include <GLES2/gl2.h>
		#define GL_GLEXT_PROTOTYPES
		#include <GLES2/gl2ext.h>
	#else
		#include <OpenGLES/ES2/gl.h>
		#include <OpenGLES/ES2/glext.h>
	#endif
#endif

#if defined(TE_OPENGL_15) || defined(TE_OPENGL_21)
	#ifdef TE_PLATFORM_MAC
		#include <OpenGL/gl.h>
		#include <OpenGL/glext.h>
	#endif

	#ifdef TE_PLATFORM_WIN
		#include <windows.h>
		#include <GL/gl.h>
		#include "glext.h"
	#endif
#endif

#if defined(TE_RENDER_D3D9)
	#include <d3d9.h>
#endif

namespace te
{
	namespace video
	{
		enum EGAPIVersion
		{
			GAPI_D3D9 = 9,

			GAPI_OGL_ES_11 = 11,
			GAPI_OGL_15 = 15,

			GAPI_OGL_ES_20 = 20,
			GAPI_OGL_21 = 21
		};
	}
}

#endif
