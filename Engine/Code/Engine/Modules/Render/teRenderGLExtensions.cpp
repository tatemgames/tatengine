/*
 *  teRenderGLExtensions.cpp
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 12/11/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#include "teRenderGLExtensions.h"
#include "teRenderGL.h"
#include "teLogManager.h"

#ifdef TE_RENDER_GL

namespace te
{
	namespace video
	{
		#ifdef TE_PLATFORM_WIN
			// Get extension
			#define TE_GET_EXT(__Name, __Type)\
				if(!(__Name = (__Type)wglGetProcAddress(#__Name)))\
					if(!(__Name = (__Type)wglGetProcAddress(#__Name "ARB")))\
						if(!(__Name = (__Type)wglGetProcAddress(#__Name "EXT")))\
						{\
							TE_LOG_ERR("OpenGL Device Dont Support " #__Name); \
						}

			// Extension functions pointers
			PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
			PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
			PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
			PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
			PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;
			PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;

			PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
			PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers;
			PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer;
			PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage;
			PFNGLGETRENDERBUFFERPARAMETERIVPROC glGetRenderbufferParameteriv;

			PFNGLGENBUFFERSPROC glGenBuffers;
			PFNGLDELETEBUFFERSPROC glDeleteBuffers;
			PFNGLBUFFERDATAPROC glBufferData;
			PFNGLBUFFERSUBDATAPROC glBufferSubData;
			PFNGLBINDBUFFERPROC glBindBuffer;
			PFNGLMAPBUFFERPROC glMapBuffer;
			PFNGLUNMAPBUFFERPROC glUnmapBuffer;
			PFNGLGETBUFFERPOINTERVPROC glGetBufferPointerv;
			PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
			PFNGLBINDVERTEXARRAYPROC glBindVertexArray;

			#ifdef TE_OPENGL_21
				PFNGLCREATESHADERPROC glCreateShader;
				PFNGLDELETESHADERPROC glDeleteShader;
				PFNGLSHADERSOURCEPROC glShaderSource;
				PFNGLCOMPILESHADERPROC glCompileShader;
				PFNGLGETSHADERIVPROC glGetShaderiv;
				PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;

				PFNGLCREATEPROGRAMPROC glCreateProgram;
				PFNGLDELETEPROGRAMPROC glDeleteProgram;
				PFNGLATTACHSHADERPROC glAttachShader;
				PFNGLDETACHSHADERPROC glDetachShader;
				PFNGLLINKPROGRAMPROC glLinkProgram;
				PFNGLVALIDATEPROGRAMPROC glValidateProgram;
				PFNGLGETPROGRAMIVPROC glGetProgramiv;
				PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
				PFNGLUSEPROGRAMPROC glUseProgram;

				PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
				PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;

				PFNGLUNIFORM1IPROC glUniform1i;
				PFNGLUNIFORM2IPROC glUniform2i;
				PFNGLUNIFORM3IPROC glUniform3i;
				PFNGLUNIFORM4IPROC glUniform4i;

				PFNGLUNIFORM1FPROC glUniform1f;
				PFNGLUNIFORM2FPROC glUniform2f;
				PFNGLUNIFORM3FPROC glUniform3f;
				PFNGLUNIFORM4FPROC glUniform4f;

				PFNGLUNIFORM1IVPROC glUniform1iv;
				PFNGLUNIFORM2IVPROC glUniform2iv;
				PFNGLUNIFORM3IVPROC glUniform3iv;
				PFNGLUNIFORM4IVPROC glUniform4iv;

				PFNGLUNIFORM1FVPROC glUniform1fv;
				PFNGLUNIFORM2FVPROC glUniform2fv;
				PFNGLUNIFORM3FVPROC glUniform3fv;
				PFNGLUNIFORM4FVPROC glUniform4fv;

				PFNGLUNIFORMMATRIX2FVPROC glUniformMatrix2fv;
				PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv;
				PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;

				PFNGLGETUNIFORMFVPROC glGetUniformfv;
				PFNGLGETUNIFORMIVPROC glGetUniformiv;

				PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
				PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;

				PFNGLVERTEXATTRIB4FPROC glVertexAttrib4f;
				PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;

				//PFNGLBINDFRAGDATALOCATIONPROC glBindFragDataLocation;
			#endif

			//PFNGLDRAWBUFFERSPROC glDrawBuffers;
			//PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer;
			//PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC glRenderbufferStorageMultisample;

			PFNGLACTIVETEXTUREPROC glActiveTexture;
			PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
		#endif

		#if defined(TE_PLATFORM_IPHONE) || defined(TE_PLATFORM_LINUX_MOBILE) || defined(TE_PLATFORM_ANDROID)
			//! Current OpenGL Version
			EGAPIVersion CurrentOpenGLVersion;
		#endif


		//! Init Extensions
		void InitExtensions()
		{
			#ifdef TE_PLATFORM_WIN
				TE_GET_EXT(glGenFramebuffers, PFNGLGENFRAMEBUFFERSPROC)
				TE_GET_EXT(glDeleteFramebuffers, PFNGLDELETEFRAMEBUFFERSPROC)
				TE_GET_EXT(glBindFramebuffer, PFNGLBINDFRAMEBUFFERPROC)
				TE_GET_EXT(glFramebufferTexture2D, PFNGLFRAMEBUFFERTEXTURE2DPROC)
				TE_GET_EXT(glFramebufferRenderbuffer, PFNGLFRAMEBUFFERRENDERBUFFERPROC)
				TE_GET_EXT(glCheckFramebufferStatus, PFNGLCHECKFRAMEBUFFERSTATUSPROC)

				TE_GET_EXT(glGenRenderbuffers, PFNGLGENRENDERBUFFERSPROC)
				TE_GET_EXT(glDeleteRenderbuffers, PFNGLDELETERENDERBUFFERSPROC)
				TE_GET_EXT(glBindRenderbuffer, PFNGLBINDRENDERBUFFERPROC)
				TE_GET_EXT(glRenderbufferStorage, PFNGLRENDERBUFFERSTORAGEPROC)
				TE_GET_EXT(glGetRenderbufferParameteriv, PFNGLGETRENDERBUFFERPARAMETERIVPROC)

				TE_GET_EXT(glGenBuffers, PFNGLGENBUFFERSPROC)
				TE_GET_EXT(glDeleteBuffers, PFNGLDELETEBUFFERSPROC)
				TE_GET_EXT(glBufferData, PFNGLBUFFERDATAPROC)
				TE_GET_EXT(glBufferSubData, PFNGLBUFFERSUBDATAPROC)
				TE_GET_EXT(glBindBuffer, PFNGLBINDBUFFERPROC)
				TE_GET_EXT(glMapBuffer, PFNGLMAPBUFFERPROC)
				TE_GET_EXT(glUnmapBuffer, PFNGLUNMAPBUFFERPROC)
				TE_GET_EXT(glGetBufferPointerv, PFNGLGETBUFFERPOINTERVPROC)
				TE_GET_EXT(glGenVertexArrays, PFNGLGENVERTEXARRAYSPROC)
				TE_GET_EXT(glBindVertexArray, PFNGLBINDVERTEXARRAYPROC)

				#ifdef TE_OPENGL_21
					TE_GET_EXT(glCreateShader, PFNGLCREATESHADERPROC)
					TE_GET_EXT(glDeleteShader, PFNGLDELETESHADERPROC)
					TE_GET_EXT(glShaderSource, PFNGLSHADERSOURCEPROC)
					TE_GET_EXT(glCompileShader, PFNGLCOMPILESHADERPROC)
					TE_GET_EXT(glGetShaderiv, PFNGLGETSHADERIVPROC)
					TE_GET_EXT(glGetShaderInfoLog, PFNGLGETSHADERINFOLOGPROC)

					TE_GET_EXT(glCreateProgram, PFNGLCREATEPROGRAMPROC)
					TE_GET_EXT(glDeleteProgram, PFNGLDELETEPROGRAMPROC)
					TE_GET_EXT(glAttachShader, PFNGLATTACHSHADERPROC)
					TE_GET_EXT(glDetachShader, PFNGLDETACHSHADERPROC)
					TE_GET_EXT(glLinkProgram, PFNGLLINKPROGRAMPROC)
					TE_GET_EXT(glValidateProgram, PFNGLVALIDATEPROGRAMPROC)
					TE_GET_EXT(glGetProgramiv, PFNGLGETPROGRAMIVPROC)
					TE_GET_EXT(glGetProgramInfoLog, PFNGLGETPROGRAMINFOLOGPROC)
					TE_GET_EXT(glUseProgram, PFNGLUSEPROGRAMPROC)

					TE_GET_EXT(glGetAttribLocation, PFNGLGETATTRIBLOCATIONPROC)
					TE_GET_EXT(glGetUniformLocation, PFNGLGETUNIFORMLOCATIONPROC)

					TE_GET_EXT(glUniform1i, PFNGLUNIFORM1IPROC)
					TE_GET_EXT(glUniform2i, PFNGLUNIFORM2IPROC)
					TE_GET_EXT(glUniform3i, PFNGLUNIFORM3IPROC)
					TE_GET_EXT(glUniform4i, PFNGLUNIFORM4IPROC)

					TE_GET_EXT(glUniform1f, PFNGLUNIFORM1FPROC)
					TE_GET_EXT(glUniform2f, PFNGLUNIFORM2FPROC)
					TE_GET_EXT(glUniform3f, PFNGLUNIFORM3FPROC)
					TE_GET_EXT(glUniform4f, PFNGLUNIFORM4FPROC)

					TE_GET_EXT(glUniform1iv, PFNGLUNIFORM1IVPROC)
					TE_GET_EXT(glUniform2iv, PFNGLUNIFORM2IVPROC)
					TE_GET_EXT(glUniform3iv, PFNGLUNIFORM3IVPROC)
					TE_GET_EXT(glUniform4iv, PFNGLUNIFORM4IVPROC)

					TE_GET_EXT(glUniform1fv, PFNGLUNIFORM1FVPROC)
					TE_GET_EXT(glUniform2fv, PFNGLUNIFORM2FVPROC)
					TE_GET_EXT(glUniform3fv, PFNGLUNIFORM3FVPROC)
					TE_GET_EXT(glUniform4fv, PFNGLUNIFORM4FVPROC)

					TE_GET_EXT(glUniformMatrix2fv, PFNGLUNIFORMMATRIX2FVPROC)
					TE_GET_EXT(glUniformMatrix3fv, PFNGLUNIFORMMATRIX3FVPROC)
					TE_GET_EXT(glUniformMatrix4fv, PFNGLUNIFORMMATRIX4FVPROC)

					TE_GET_EXT(glGetUniformfv, PFNGLGETUNIFORMFVPROC)
					TE_GET_EXT(glGetUniformiv, PFNGLGETUNIFORMIVPROC)

					TE_GET_EXT(glEnableVertexAttribArray, PFNGLENABLEVERTEXATTRIBARRAYPROC)
					TE_GET_EXT(glDisableVertexAttribArray, PFNGLDISABLEVERTEXATTRIBARRAYPROC)

					TE_GET_EXT(glVertexAttrib4f, PFNGLVERTEXATTRIB4FPROC)
					TE_GET_EXT(glVertexAttribPointer, PFNGLVERTEXATTRIBPOINTERPROC)
				#endif

				TE_GET_EXT(glActiveTexture, PFNGLACTIVETEXTUREPROC)
				TE_GET_EXT(glGenerateMipmap, PFNGLGENERATEMIPMAPPROC)
			#endif

			#if defined(TE_PLATFORM_IPHONE) || defined(TE_PLATFORM_LINUX_MOBILE) || defined(TE_PLATFORM_ANDROID)
//				CurrentOpenGLVersion = GetRender()->GetContext()->GetVersion();
			
			CurrentOpenGLVersion = GAPI_OGL_ES_20;
			#endif
		}
		
		// oh, ok, just for what this hell of defines ?
		// this defines needed to support win\mac\iphone\linux in one time
		// iphone extensions : __Name or __NameOES
		// mac extensions : __NameEXT or etc, cause there static ogl lib
		// win extensions : dynamic (very useful)
		
		// Core extension run define
		#if defined(TE_PLATFORM_MAC) || defined(TE_PLATFORM_IPHONE) || defined(TE_PLATFORM_LINUX_MOBILE) || defined(TE_PLATFORM_ANDROID)
			// so on mac\iphone we just run ext without anything
			#define TE_EXT_CORE(__Name, __Args) __Name __Args;
			#define TE_EXT_RET_CORE(__Name, __Args) return TE_EXT_CORE(__Name, __Args)
		#endif

		#ifdef TE_PLATFORM_WIN
			// on win we check aviability at first, 'cause ext is pointer
			#define TE_EXT_CORE(__Name, __Args) if(__Name) __Name __Args;
			#define TE_EXT_RET_CORE(__Name, __Args) \
				if(__Name)\
					return __Name __Args;\
				else\
					return 0;
		#endif
		
		#if defined(TE_PLATFORM_MAC) || defined(TE_PLATFORM_WIN)
			// on win\mac we have ARB and EXT
			#define TE_EXT(__Name, __Type, __Args) TE_EXT_CORE(__Name##__Type, __Args)
			#define TE_EXT_RET(__Name, __Type, __Args) TE_EXT_RET_CORE(__Name##__Type, __Args)
		#endif

		#if defined(TE_PLATFORM_IPHONE) || defined(TE_PLATFORM_LINUX_MOBILE) || defined(TE_PLATFORM_ANDROID)
			// on iphone only OES, but there 2.0 and 1.1
			#if defined(TE_OPENGL_ES_11) && defined(TE_OPENGL_ES_20) // support to both
				#define TE_EXT(__Name, __Type, __Args) \
					if(CurrentOpenGLVersion == GAPI_OGL_ES_20)\
					{\
						TE_EXT_CORE(__Name, __Args)\
					}\
					else\
					{\
						TE_EXT_CORE(__Name##OES, __Args)\
					}
				#define TE_EXT_RET(__Name, __Type, __Args) \
					if(CurrentOpenGLVersion == GAPI_OGL_ES_20)\
					{\
						TE_EXT_RET_CORE(__Name, __Args)\
					}\
					else\
					{\
						TE_EXT_RET_CORE(__Name##OES, __Args)\
					}
			#endif
		
			#if defined(TE_OPENGL_ES_11) && !defined(TE_OPENGL_ES_20) // only 1.1
				#define TE_EXT(__Name, __Type, __Args) TE_EXT_CORE(__Name##OES, __Args)
				#define TE_EXT_RET(__Name, __Type, __Args) TE_EXT_RET_CORE(__Name##OES, __Args)
			#endif
		
			#if !defined(TE_OPENGL_ES_11) && defined(TE_OPENGL_ES_20) // only 1.1
				#define TE_EXT(__Name, __Type, __Args) TE_EXT_CORE(__Name, __Args)
				#define TE_EXT_RET(__Name, __Type, __Args) TE_EXT_RET_CORE(__Name, __Args)
			#endif
		#endif
		
		// ok, there hell starting, mac have static ogl, and we can only get EXT functions, in win we can get EXT with clear name
		#ifdef TE_PLATFORM_MAC
			#define TE_EXT_FRAMEBUFFER(__Name, __Args) TE_EXT(__Name, EXT, __Args)
			#define TE_EXT_FRAMEBUFFER_RET(__Name, __Args) TE_EXT_RET(__Name, EXT, __Args)
			#define TE_EXT_RENDERBUFFER(__Name, __Args) TE_EXT(__Name, EXT, __Args)
			#define TE_EXT_RENDERBUFFER_RET(__Name, __Args) TE_EXT_RET(__Name, EXT, __Args)
		#else
			#define TE_EXT_FRAMEBUFFER(__Name, __Args) TE_EXT(__Name, , __Args)
			#define TE_EXT_FRAMEBUFFER_RET(__Name, __Args) TE_EXT_RET(__Name, , __Args)
			#define TE_EXT_RENDERBUFFER(__Name, __Args) TE_EXT(__Name, , __Args)
			#define TE_EXT_RENDERBUFFER_RET(__Name, __Args) TE_EXT_RET(__Name, , __Args)
		#endif

		#define TE_EXT_BUFFER(__Name, __Args) TE_EXT_CORE(__Name, __Args)
		#define TE_EXT_BUFFER_RET(__Name, __Args) TE_EXT_RET_CORE(__Name, __Args)

		// TODO rewrite for win
		#if defined(TE_PLATFORM_IPHONE) || defined(TE_PLATFORM_LINUX_MOBILE) || defined(TE_PLATFORM_ANDROID)
			#define TE_EXT_MAPBUFFER(__Name, __Args) TE_EXT_CORE(__Name##OES , __Args)
			#define TE_EXT_MAPBUFFER_RET(__Name, __Args) TE_EXT_RET_CORE(__Name##OES , __Args)
		#else
			#define TE_EXT_MAPBUFFER(__Name, __Args) TE_EXT_CORE(__Name , __Args)
			#define TE_EXT_MAPBUFFER_RET(__Name, __Args) TE_EXT_RET_CORE(__Name , __Args)
		#endif

		#if defined(TE_PLATFORM_IPHONE) || defined(TE_PLATFORM_LINUX_MOBILE) || defined(TE_PLATFORM_ANDROID)
			#define TE_EXT_VAO(__Name, __Args) TE_EXT_CORE(__Name##OES , __Args)
			#define TE_EXT_VAO_RET(__Name, __Args) TE_EXT_RET_CORE(__Name##OES , __Args)
		#elif defined(TE_PLATFORM_MAC)
			#define TE_EXT_VAO(__Name, __Args) TE_EXT_CORE(__Name##APPLE , __Args)
			#define TE_EXT_VAO_RET(__Name, __Args) TE_EXT_RET_CORE(__Name##APPLE , __Args)
		#else
			#define TE_EXT_VAO(__Name, __Args) TE_EXT_CORE(__Name , __Args)
			#define TE_EXT_VAO_RET(__Name, __Args) TE_EXT_RET_CORE(__Name , __Args)
		#endif

		#define TE_EXT_SHADER(__Name, __Args) TE_EXT_CORE(__Name, __Args)
		#define TE_EXT_SHADER_RET(__Name, __Args) TE_EXT_RET_CORE(__Name, __Args)

		#define TE_EXT_MIPMAP(__Name, __Args) TE_EXT(__Name, , __Args)

		//! ------------------------------------------------------------------------------------ Framebuffers
		
		//! Generate Framebuffers
		void tglGenFramebuffers(GLsizei Count, GLuint * Framebuffers)
		{
			TE_EXT_FRAMEBUFFER(glGenFramebuffers, (Count, Framebuffers))
		}

		//! Delete Framebuffers
		void tglDeleteFramebuffers(GLsizei Count, const GLuint * Framebuffers)
		{
			TE_EXT_FRAMEBUFFER(glDeleteFramebuffers, (Count, Framebuffers))
		}

		//! Bind Framebuffer
		void tglBindFramebuffer(GLuint Framebuffer)
		{
			TE_EXT_FRAMEBUFFER(glBindFramebuffer, (GL_FRAMEBUFFER, Framebuffer))
		}

		//! Setup Framebuffer Texture 2D Attachment
		void tglFramebufferTexture2D(GLenum Attachment, GLenum TexureTarget, GLuint Texture, GLint Level)
		{
			TE_EXT_FRAMEBUFFER(glFramebufferTexture2D, (GL_FRAMEBUFFER, Attachment, TexureTarget, Texture, Level))
		}

		//! Setup Framebuffer Renderbuffer Attachment
		void tglFramebufferRenderbuffer(GLenum Attachment, GLenum RenderbufferTarget, GLuint Renderbuffer)
		{
			TE_EXT_FRAMEBUFFER(glFramebufferRenderbuffer, (GL_FRAMEBUFFER, Attachment, RenderbufferTarget, Renderbuffer))
		}

		//! Check Framebuffer Status
		GLenum tglCheckFramebufferStatus()
		{
			TE_EXT_FRAMEBUFFER_RET(glCheckFramebufferStatus, (GL_FRAMEBUFFER))
		}

		//! ------------------------------------------------------------------------------------ Renderbuffers

		//! Generate Renderbuffers
		void tglGenRenderbuffers(GLsizei Count, GLuint * Renderbuffers)
		{
			TE_EXT_RENDERBUFFER(glGenRenderbuffers, (Count, Renderbuffers))
		}

		//! Delete Renderbuffers
		void tglDeleteRenderbuffers(GLsizei Count, const GLuint * Renderbuffers)
		{
			TE_EXT_RENDERBUFFER(glDeleteRenderbuffers, (Count, Renderbuffers))
		}

		//! Bind Renderbuffer
		void tglBindRenderbuffer(GLuint Renderbuffer)
		{
			TE_EXT_RENDERBUFFER(glBindRenderbuffer, (GL_RENDERBUFFER, Renderbuffer))
		}

		//! Setup Renderbuffer Storage
		void tglRenderbufferStorage(GLenum InternalFormat, GLsizei Width, GLsizei Height)
		{
			TE_EXT_RENDERBUFFER(glRenderbufferStorage, (GL_RENDERBUFFER, InternalFormat, Width, Height))
		}

		//! Get Renderbuffer Parameter
		void tglGetRenderbufferParameteriv(GLenum ParameterName, GLint * Parameters)
		{
			TE_EXT_RENDERBUFFER(glGetRenderbufferParameteriv, (GL_RENDERBUFFER, ParameterName, Parameters))
		}

		//! ------------------------------------------------------------------------------------ Buffers

		//! Generate Buffers
		void tglGenBuffers(GLsizei Count, GLuint * Buffers)
		{
			TE_EXT_BUFFER(glGenBuffers, (Count, Buffers))
		}

		//! Delete Buffers
		void tglDeleteBuffers(GLsizei Count, const GLuint * Buffers)
		{
			TE_EXT_BUFFER(glDeleteBuffers, (Count, Buffers))
		}

		//! Buffer Data
		void tglBufferData(GLenum Target, GLsizeiptr Size, const GLvoid * Data, GLenum Usage)
		{
			TE_EXT_BUFFER(glBufferData, (Target, Size, Data, Usage))
		}

		void tglBufferSubData(GLenum Target, GLsizeiptr Offset, GLsizeiptr Size, const GLvoid * Data)
		{
			TE_EXT_BUFFER(glBufferSubData, (Target, Offset, Size, Data))
		}
		
		//! Bind Buffer
		void tglBindBuffer(GLenum Target, GLuint Buffer)
		{
			TE_EXT_BUFFER(glBindBuffer, (Target, Buffer))
		}

		//! Map Buffer
		GLvoid * tglMapBuffer(GLenum Target, GLenum Access)
		{
			TE_EXT_MAPBUFFER_RET(glMapBuffer, (Target, Access))
		}
		
		//! Unmap Buffer
		GLboolean tglUnmapBuffer(GLenum Target)
		{
			TE_EXT_MAPBUFFER_RET(glUnmapBuffer, (Target))
		}

		//! Get Buffer Pointer
		void tglGetBufferPointerv(GLenum Target, GLenum ParameterName, GLvoid ** Parameters)
		{
			TE_EXT_MAPBUFFER(glGetBufferPointerv, (Target, ParameterName, Parameters))
		}
		
		void tglGenVertexArrays(GLsizei n, GLuint * vaos)
		{
			TE_EXT_VAO(glGenVertexArrays, (n, vaos));
		}
		
		void tglBindVertexArray(GLuint vao)
		{
			TE_EXT_VAO(glBindVertexArray, (vao));
		}

		//! ------------------------------------------------------------------------------------ Shaders

		#if defined(TE_OPENGL_ES_20) || defined(TE_OPENGL_21)

		//! Create Shader
		GLuint tglCreateShader(GLenum Type)
		{
			TE_EXT_SHADER_RET(glCreateShader, (Type))
		}

		//! Delete Shader
		void tglDeleteShader(GLuint Shader)
		{
			TE_EXT_SHADER(glDeleteShader, (Shader))
		}

		//! Set Shader Source
		void tglShaderSource(GLuint Shader, GLsizei Count, const GLchar ** String, const GLint * Length)
		{
			TE_EXT_SHADER(glShaderSource, (Shader, Count, String, Length))
		}

		//! Compile Shader
		void tglCompileShader(GLuint Shader)
		{
			TE_EXT_SHADER(glCompileShader, (Shader))
		}

		//! Get Shader Parameter
		void tglGetShaderiv(GLuint Shader, GLenum ParameterName, GLint * Parameters)
		{
			TE_EXT_SHADER(glGetShaderiv, (Shader, ParameterName, Parameters))
		}

		//! Get Shader Log
		void tglGetShaderInfoLog(GLuint Shader, GLsizei BufferSize, GLsizei * Length, GLchar * InfoLog)
		{
			TE_EXT_SHADER(glGetShaderInfoLog, (Shader, BufferSize, Length, InfoLog))
		}

		//! Create Program
		GLuint tglCreateProgram()
		{
			TE_EXT_SHADER_RET(glCreateProgram, ())
		}

		//! Delete Program
		void tglDeleteProgram(GLuint Program)
		{
			TE_EXT_SHADER(glDeleteProgram, (Program))
		}

		//! Attach Shader
		void tglAttachShader(GLuint Program, GLuint Shader)
		{
			TE_EXT_SHADER(glAttachShader, (Program, Shader))
		}

		//! Detach Shader
		void tglDetachShader(GLuint Program, GLuint Shader)
		{
			TE_EXT_SHADER(glDetachShader, (Program, Shader))
		}

		//! Link Program
		void tglLinkProgram(GLuint Program)
		{
			TE_EXT_SHADER(glLinkProgram, (Program))
		}

		//! Validate Program
		void tglValidateProgram(GLuint Program)
		{
			TE_EXT_SHADER(glValidateProgram, (Program))
		}

		//! Get Program Parameter
		void tglGetProgramiv(GLuint Program, GLenum ParameterName, GLint * Parameters)
		{
			TE_EXT_SHADER(glGetProgramiv, (Program, ParameterName, Parameters))
		}

		//! Get Program Log
		void tglGetProgramInfoLog(GLuint Program, GLsizei BufferSize, GLsizei * Length, GLchar * InfoLog)
		{
			TE_EXT_SHADER(glGetProgramInfoLog, (Program, BufferSize, Length, InfoLog))
		}

		//! Use Program
		void tglUseProgram(GLuint Program)
		{
			TE_EXT_SHADER(glUseProgram, (Program))
		}

		//! Get Attribute Location
		int tglGetAttribLocation(GLuint Program, const GLchar * Name)
		{
			TE_EXT_SHADER_RET(glGetAttribLocation, (Program, Name))
		}

		//! Get Uniform Location
		int tglGetUniformLocation(GLuint Program, const GLchar * Name)
		{
			TE_EXT_SHADER_RET(glGetUniformLocation, (Program, Name))
		}

		//! Set Uniform
		void tglUniform1i(GLint Location, GLint X)
		{
			TE_EXT_SHADER(glUniform1i, (Location, X))
		}

		//! Set Uniform
		void tglUniform2i(GLint Location, GLint X, GLint Y)
		{
			TE_EXT_SHADER(glUniform2i, (Location, X, Y))
		}

		//! Set Uniform
		void tglUniform3i(GLint Location, GLint X, GLint Y, GLint Z)
		{
			TE_EXT_SHADER(glUniform3i, (Location, X, Y, Z))
		}

		//! Set Uniform
		void tglUniform4i(GLint Location, GLint X, GLint Y, GLint Z, GLint W)
		{
			TE_EXT_SHADER(glUniform4i, (Location, X, Y, Z, W))
		}

		//! Set Uniform
		void tglUniform1f(GLint Location, GLfloat X)
		{
			TE_EXT_SHADER(glUniform1f, (Location, X))
		}

		//! Set Uniform
		void tglUniform2f(GLint Location, GLfloat X, GLfloat Y)
		{
			TE_EXT_SHADER(glUniform2f, (Location, X, Y))
		}

		//! Set Uniform
		void tglUniform3f(GLint Location, GLfloat X, GLfloat Y, GLfloat Z)
		{
			TE_EXT_SHADER(glUniform3f, (Location, X, Y, Z))
		}

		//! Set Uniform
		void tglUniform4f(GLint Location, GLfloat X, GLfloat Y, GLfloat Z, GLfloat W)
		{
			TE_EXT_SHADER(glUniform4f, (Location, X, Y, Z, W))
		}

		//! Set Uniform
		void tglUniform1iv(GLint Location, GLsizei Count, const GLint * Values)
		{
			TE_EXT_SHADER(glUniform1iv, (Location, Count, Values))
		}

		//! Set Uniform
		void tglUniform2iv(GLint Location, GLsizei Count, const GLint * Values)
		{
			TE_EXT_SHADER(glUniform2iv, (Location, Count, Values))
		}

		//! Set Uniform
		void tglUniform3iv(GLint Location, GLsizei Count, const GLint * Values)
		{
			TE_EXT_SHADER(glUniform3iv, (Location, Count, Values))
		}

		//! Set Uniform
		void tglUniform4iv(GLint Location, GLsizei Count, const GLint * Values)
		{
			TE_EXT_SHADER(glUniform4iv, (Location, Count, Values))
		}

		//! Set Uniform
		void tglUniform1fv(GLint Location, GLsizei Count, const GLfloat * Values)
		{
			TE_EXT_SHADER(glUniform1fv, (Location, Count, Values))
		}

		//! Set Uniform
		void tglUniform2fv(GLint Location, GLsizei Count, const GLfloat * Values)
		{
			TE_EXT_SHADER(glUniform2fv, (Location, Count, Values))
		}

		//! Set Uniform
		void tglUniform3fv(GLint Location, GLsizei Count, const GLfloat * Values)
		{
			TE_EXT_SHADER(glUniform3fv, (Location, Count, Values))
		}

		//! Set Uniform
		void tglUniform4fv(GLint Location, GLsizei Count, const GLfloat * Values)
		{
			TE_EXT_SHADER(glUniform4fv, (Location, Count, Values))
		}

		//! Set Uniform Matrix
		void tglUniformMatrix2fv(GLint Location, GLsizei Count, GLboolean Transpose, const GLfloat * Value)
		{
			TE_EXT_SHADER(glUniformMatrix2fv, (Location, Count, Transpose, Value))
		}

		//! Set Uniform Matrix
		void tglUniformMatrix3fv(GLint Location, GLsizei Count, GLboolean Transpose, const GLfloat * Value)
		{
			TE_EXT_SHADER(glUniformMatrix3fv, (Location, Count, Transpose, Value))
		}

		//! Set Uniform Matrix
		void tglUniformMatrix4fv(GLint Location, GLsizei Count, GLboolean Transpose, const GLfloat * Value)
		{
			TE_EXT_SHADER(glUniformMatrix4fv, (Location, Count, Transpose, Value))
		}

		//! Get Uniform
		void tglGetUniformfv(GLuint Program, GLint Location, GLfloat * Params)
		{
			TE_EXT_SHADER(glGetUniformfv, (Program, Location, Params))
		}

		//! Get Uniform
		void tglGetUniformiv(GLuint Program, GLint Location, GLint * Params)
		{
			TE_EXT_SHADER(glGetUniformiv, (Program, Location, Params))
		}

		//! Enable Vertex Attribue
		void tglEnableVertexAttribArray(GLuint Index)
		{
			TE_EXT_SHADER(glEnableVertexAttribArray, (Index))
		}

		//! Disable Vertex Attribue
		void tglDisableVertexAttribArray(GLuint Index)
		{
			TE_EXT_SHADER(glDisableVertexAttribArray, (Index))
		}

		//! Set Vertex Attribute
		void tglVertexAttrib4f(GLuint Index, GLfloat X, GLfloat Y, GLfloat Z, GLfloat W)
		{
			TE_EXT_SHADER(glVertexAttrib4f, (Index, X, Y, Z, W))
		}

		//! Set Vertex Attribute Pointer
		void tglVertexAttribPointer(GLuint Index, GLint Size, GLenum Type, GLboolean Normalized, GLsizei Stride, const GLvoid * Ptr)
		{
			TE_EXT_SHADER(glVertexAttribPointer, (Index, Size, Type, Normalized, Stride, Ptr))
		}

		#endif

		//! ------------------------------------------------------------------------------------

		//! Set Active Texture
		void tglActiveTexture(GLenum Texture)
		{
			TE_EXT_CORE(glActiveTexture, (Texture))
		}

		//! Generate Mipmap
		void tglGenerateMipmap(GLenum Texture)
		{
			TE_EXT_MIPMAP(glGenerateMipmap, (Texture))
		}
	}
}

#endif
