/*
 *  teRenderGLSystem.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 11/27/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TERENDERGLSYSTEM_H
#define TE_TERENDERGLSYSTEM_H

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

namespace te
{
	namespace video
	{
		//! OpenGL Version
		enum EOpenGLVersion
		{
			//! OpenGL ES 1.1
			OGLV_ES_11 = 11,

			//! OpenGL ES 2.0
			OGLV_ES_20 = 20,
			
			//! OpenGL 1.5
			OGLV_15 = 15,

			//! OpenGL 2.1
			OGLV_21 = 21
		};
	}
}

#endif
