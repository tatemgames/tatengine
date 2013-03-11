/*
 *  teRenderGLExtensions.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 12/11/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TERENDERGLEXTENSIONS_H
#define TE_TERENDERGLEXTENSIONS_H

#include "teRenderSystem.h"

#ifdef TE_RENDER_GL

namespace te
{
	namespace video
	{
		//! Init Extensions
		void InitExtensions();

		//! ------------------------------------------------------------------------------------ Framebuffers

		void tglGenFramebuffers(GLsizei Count, GLuint * Framebuffers);

		void tglDeleteFramebuffers(GLsizei Count, const GLuint * Framebuffers);

		void tglBindFramebuffer(GLuint Framebuffer);

		void tglFramebufferTexture2D(GLenum Attachment, GLenum TexureTarget, GLuint Texture, GLint Level);

		void tglFramebufferRenderbuffer(GLenum Attachment, GLenum RenderbufferTarget, GLuint Renderbuffer);

		GLenum tglCheckFramebufferStatus();

		//! ------------------------------------------------------------------------------------ Renderbuffers

		void tglGenRenderbuffers(GLsizei Count, GLuint * Renderbuffers);

		void tglDeleteRenderbuffers(GLsizei Count, const GLuint * Renderbuffers);

		void tglBindRenderbuffer(GLuint Renderbuffer);

		void tglRenderbufferStorage(GLenum InternalFormat, GLsizei Width, GLsizei Height);

		void tglGetRenderbufferParameteriv(GLenum ParameterName, GLint * Parameters);

		//! ------------------------------------------------------------------------------------ Buffers

		void tglGenBuffers(GLsizei Count, GLuint * Buffers);

		void tglDeleteBuffers(GLsizei Count, const GLuint * Buffers);

		void tglBufferData(GLenum Target, GLsizeiptr Size, const GLvoid * Data, GLenum Usage);

		void tglBufferSubData(GLenum Target, GLsizeiptr Offset, GLsizeiptr Size, const GLvoid * Data);
		
		void tglBindBuffer(GLenum Target, GLuint Buffer);

		GLvoid * tglMapBuffer(GLenum Target, GLenum Access);
		
		GLboolean tglUnmapBuffer(GLenum Target);

		void tglGetBufferPointerv(GLenum Target, GLenum ParameterName, GLvoid ** Parameters);

		void tglGenVertexArrays(GLsizei n, GLuint * vaos);
		
		void tglBindVertexArray(GLuint vao);
		
		//! ------------------------------------------------------------------------------------ Shaders

		#if defined(TE_OPENGL_ES_20) || defined(TE_OPENGL_21)

		GLuint tglCreateShader(GLenum Type);

		void tglDeleteShader(GLuint Shader);

		void tglShaderSource(GLuint Shader, GLsizei Count, const GLchar ** String, const GLint * Length);

		void tglCompileShader(GLuint Shader);

		void tglGetShaderiv(GLuint Shader, GLenum ParameterName, GLint * Parameters);

		void tglGetShaderInfoLog(GLuint Shader, GLsizei BufferSize, GLsizei * Length, GLchar * InfoLog);

		GLuint tglCreateProgram();

		void tglDeleteProgram(GLuint Program);

		void tglAttachShader(GLuint Program, GLuint Shader);

		void tglDetachShader(GLuint Program, GLuint Shader);

		void tglLinkProgram(GLuint Program);

		void tglValidateProgram(GLuint Program);

		void tglGetProgramiv(GLuint Program, GLenum ParameterName, GLint * Parameters);

		void tglGetProgramInfoLog(GLuint Program, GLsizei BufferSize, GLsizei * Length, GLchar * InfoLog);

		void tglUseProgram(GLuint Program);

		int tglGetAttribLocation(GLuint Program, const GLchar * Name);

		int tglGetUniformLocation(GLuint Program, const GLchar * Name);

		void tglUniform1i(GLint Location, GLint X);

		void tglUniform2i(GLint Location, GLint X, GLint Y);

		void tglUniform3i(GLint Location, GLint X, GLint Y, GLint Z);

		void tglUniform4i(GLint Location, GLint X, GLint Y, GLint Z, GLint W);

		void tglUniform1f(GLint Location, GLfloat X);

		void tglUniform2f(GLint Location, GLfloat X, GLfloat Y);

		void tglUniform3f(GLint Location, GLfloat X, GLfloat Y, GLfloat Z);

		void tglUniform4f(GLint Location, GLfloat X, GLfloat Y, GLfloat Z, GLfloat W);

		void tglUniform1iv(GLint Location, GLsizei Count, const GLint * Values);

		void tglUniform2iv(GLint Location, GLsizei Count, const GLint * Values);

		void tglUniform3iv(GLint Location, GLsizei Count, const GLint * Values);

		void tglUniform4iv(GLint Location, GLsizei Count, const GLint * Values);

		void tglUniform1fv(GLint Location, GLsizei Count, const GLfloat * Values);

		void tglUniform2fv(GLint Location, GLsizei Count, const GLfloat * Values);

		void tglUniform3fv(GLint Location, GLsizei Count, const GLfloat * Values);

		void tglUniform4fv(GLint Location, GLsizei Count, const GLfloat * Values);

		void tglUniformMatrix2fv(GLint Location, GLsizei Count, GLboolean Transpose, const GLfloat * Value);

		void tglUniformMatrix3fv(GLint Location, GLsizei Count, GLboolean Transpose, const GLfloat * Value);

		void tglUniformMatrix4fv(GLint Location, GLsizei Count, GLboolean Transpose, const GLfloat * Value);

		void tglGetUniformfv(GLuint Program, GLint Location, GLfloat * Params);

		void tglGetUniformiv(GLuint Program, GLint Location, GLint * Params);

		void tglEnableVertexAttribArray(GLuint Index);

		void tglDisableVertexAttribArray(GLuint Index);

		void tglVertexAttrib4f(GLuint Index, GLfloat X, GLfloat Y, GLfloat Z, GLfloat W);

		void tglVertexAttribPointer(GLuint Index, GLint Size, GLenum Type, GLboolean Normalized, GLsizei Stride, const GLvoid * Ptr);

		#endif

		//! ------------------------------------------------------------------------------------

		void tglActiveTexture(GLenum Texture);

		void tglGenerateMipmap(GLenum Texture);

		#if (defined(TE_PLATFORM_IPHONE) || defined(TE_PLATFORM_LINUX_MOBILE) || defined(TE_PLATFORM_ANDROID))
			// well, actually GL_XXX_OES = GL_XXX in ES 1.1 and 2.0
			#if!defined(TE_OPENGL_ES_20)
				#define GL_FRAMEBUFFER GL_FRAMEBUFFER_OES
				#define GL_COLOR_ATTACHMENT0 GL_COLOR_ATTACHMENT0_OES
				#define GL_DEPTH_ATTACHMENT GL_DEPTH_ATTACHMENT_OES
				#define GL_FRAMEBUFFER_COMPLETE GL_FRAMEBUFFER_COMPLETE_OES

				#define GL_RENDERBUFFER GL_RENDERBUFFER_OES
				#define GL_RENDERBUFFER_WIDTH GL_RENDERBUFFER_WIDTH_OES
				#define GL_RENDERBUFFER_HEIGHT GL_RENDERBUFFER_HEIGHT_OES
	
				#define GL_DEPTH_COMPONENT16 GL_DEPTH_COMPONENT16_OES
			#endif

			#define GL_WRITE_ONLY GL_WRITE_ONLY_OES
			#define GL_BUFFER_MAP_POINTER GL_BUFFER_MAP_POINTER_OES
		#endif
		
//		#ifdef TE_PLATFORM_MAC
//			// oh, very usefull glext.h from apple, thank you !
//			#define GL_FRAMEBUFFER GL_FRAMEBUFFER_EXT
//			#define GL_COLOR_ATTACHMENT0 GL_COLOR_ATTACHMENT0_EXT
//			#define GL_DEPTH_ATTACHMENT GL_DEPTH_ATTACHMENT_EXT
//			#define GL_FRAMEBUFFER_COMPLETE GL_FRAMEBUFFER_COMPLETE_EXT
//
//			#define GL_RENDERBUFFER GL_RENDERBUFFER_EXT
//			#define GL_RENDERBUFFER_WIDTH GL_RENDERBUFFER_WIDTH_EXT
//			#define GL_RENDERBUFFER_HEIGHT GL_RENDERBUFFER_HEIGHT_EXT
//		#endif
	}
}

#endif
#endif