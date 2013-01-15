 /*
 *  teShader.cpp
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 11/25/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#include "teShader.h"
#include "teLogManager.h"
#include "teRenderGL.h"

namespace te
{
	namespace video
	{
		teShader::teShader()
		{
		}

		teShader::~teShader()
		{
		}

		#ifdef TE_RENDER_GL_SHADERS

		void teShader::SetSource(const teString & setSource)
		{
			source = setSource;

			BuildPass(SP_GEN_DIFFUSE);
		}

		#endif

		void teShader::BindAttributes(teSurfaceData * surface, const teSurfaceLayers & layers, u1 localOffsets, uintptr_t basePtr)
		{
			#ifdef TE_RENDER_GL_SHADERS

			TE_ASSERT(pass[currentPass].program);

			for(u8 i = 0; i < SLT_INDEXES; ++i)
			{
				GLuint uniform = pass[currentPass].attributes[i];

				if(uniform == -1)
					continue;

				if(layers.variablesPerObject[i])
				{
					tglEnableVertexAttribArray(uniform);
					tglVertexAttribPointer(uniform,
						layers.variablesPerObject[i],
						layers.GetVariableGLType(i),
						TE_GET_BIT(layers.normalized, i),
						layers.stride[i],
						(const GLvoid*)(surface->data - (localOffsets ? basePtr : 0) + layers.offset[i]));
				}
				else
				{
					tglDisableVertexAttribArray(uniform);

					//if(i == SLT_COLOR)
						tglVertexAttrib4f(uniform, 1.0f, 1.0f, 1.0f, 1.0f); // TODO Default color is white !
				}
			}

			//TE_ASSERT(glGetError() == GL_NO_ERROR);

			#endif
		}

		void teShader::DisableAttributes()
		{
			#ifdef TE_RENDER_GL_SHADERS

			TE_ASSERT(pass[currentPass].program);

			for(u8 i = 0; i < SLT_INDEXES; ++i)
			{
				GLuint uniform = pass[currentPass].attributes[i];

				if(uniform == -1)
					continue;

				tglDisableVertexAttribArray(uniform);
			}

			//TE_ASSERT(glGetError() == GL_NO_ERROR);

			#endif
		}

		#ifdef TE_RENDER_GL_SHADERS

		void teShader::BuildPass(EShaderPass passType)
		{
			const c8 * passTypeString = NULL;

			switch(passType)
			{
			case SP_GEN_DIFFUSE: passTypeString = "#define TE_PASS_DIFFUSE\n"; break;
			case SP_USER_0: passTypeString = "#define TE_PASS_USER_0\n"; break;
			case SP_USER_1: passTypeString = "#define TE_PASS_USER_1\n"; break;
			case SP_USER_2: passTypeString = "#define TE_PASS_USER_2\n"; break;
			case SP_USER_3: passTypeString = "#define TE_PASS_USER_3\n"; break;
			default: TE_ASSERT_NODEBUG(0); break;
			}

			const u8 serviceStrings = 6;

			GLuint shaderVS = tglCreateShader(GL_VERTEX_SHADER);
			const c8 * sourceVSc[serviceStrings];
			GLint sourceVSl[serviceStrings];

			#if defined (TE_PLATFORM_IPHONE)
			sourceVSc[0] = "\n";
			#else
			sourceVSc[0] = "#version 120\n";
			#endif

			sourceVSc[1] = "#define TE_VS\n";

			#if defined (TE_PLATFORM_IPHONE)
			sourceVSc[2] = "precision mediump int;\nprecision mediump float;\n#define TE_TEXP highp\n";
			#else
			sourceVSc[2] = "#define TE_TEXP\n";
			#endif

			sourceVSc[3] = passTypeString;

			sourceVSc[4] = source.c_str();

			sourceVSc[5] = "\n";

			for(u8 i = 0; i < serviceStrings; ++i)
				sourceVSl[i] = strlen(sourceVSc[i]);

			tglShaderSource(shaderVS, serviceStrings, sourceVSc, sourceVSl);
			tglCompileShader(shaderVS);
			LogErrors("compile VS", shaderVS);

			GLuint shaderFS = tglCreateShader(GL_FRAGMENT_SHADER);
			const c8 * sourceFSc[serviceStrings];
			GLint sourceFSl[serviceStrings];

			#if defined (TE_PLATFORM_IPHONE)
			sourceFSc[0] = "\n";
			#else
			sourceFSc[0] = "#version 120\n";
			#endif

			sourceFSc[1] = "#define TE_FS\n";

			#if defined (TE_PLATFORM_IPHONE)
			sourceFSc[2] = "precision lowp int;\nprecision lowp float;\n#define TE_TEXP highp\n";
			#else
			sourceFSc[2] = "#define TE_TEXP\n";
			#endif

			sourceFSc[3] = passTypeString;

			sourceFSc[4] = source.c_str();

			sourceFSc[5] = "\n";

			for(u8 i = 0; i < serviceStrings; ++i)
				sourceFSl[i] = strlen(sourceFSc[i]);

			tglShaderSource(shaderFS, serviceStrings, sourceFSc, sourceFSl);
			tglCompileShader(shaderFS);
			LogErrors("compile FS", shaderFS);

			if(pass[passType].program)
				tglDeleteProgram(pass[passType].program);

			pass[passType].program = tglCreateProgram();

			tglAttachShader(pass[passType].program, shaderVS);
			tglAttachShader(pass[passType].program, shaderFS);

			tglLinkProgram(pass[passType].program);

			tglDeleteShader(shaderVS);
			tglDeleteShader(shaderFS);

			LogErrors("link", 0, pass[passType].program);

			tglUseProgram(pass[passType].program);

			pass[passType].attributes[SLT_POSITION] =	tglGetAttribLocation(pass[passType].program, "tePosition");
			pass[passType].attributes[SLT_NORMAL] =		tglGetAttribLocation(pass[passType].program, "teNormal");
			pass[passType].attributes[SLT_UV_0] =		tglGetAttribLocation(pass[passType].program, "teUV0");
			pass[passType].attributes[SLT_UV_1] =		tglGetAttribLocation(pass[passType].program, "teUV1");
			pass[passType].attributes[SLT_UV_2] =		tglGetAttribLocation(pass[passType].program, "teUV2");
			pass[passType].attributes[SLT_UV_3] =		tglGetAttribLocation(pass[passType].program, "teUV3");
			pass[passType].attributes[SLT_COLOR] =		tglGetAttribLocation(pass[passType].program, "teColor");
			pass[passType].attributes[SLT_TANGENT] =	tglGetAttribLocation(pass[passType].program, "teTangent");
			pass[passType].attributes[SLT_BONES_IDS] =	tglGetAttribLocation(pass[passType].program, "teBonesIds");
			pass[passType].attributes[SLT_BONES_W] =	tglGetAttribLocation(pass[passType].program, "teBonesW");
			pass[passType].attributes[SLT_USER_0] =		tglGetAttribLocation(pass[passType].program, "teUser0");
			pass[passType].attributes[SLT_USER_1] =		tglGetAttribLocation(pass[passType].program, "teUser1");
			pass[passType].attributes[SLT_USER_2] =		tglGetAttribLocation(pass[passType].program, "teUser2");

			for(u8 i = 0; i < teMaxTextures; ++i)
			{
				c8 name[] = "teTexture ";
				name[9] = '0' + i;
				pass[passType].uniforms[UT_TEX_0 + i] = tglGetUniformLocation(pass[passType].program, name);
			}

			pass[passType].uniforms[UT_C_DIFFUSE] = tglGetUniformLocation(pass[passType].program, "teDiffuseColor");
			pass[passType].uniforms[UT_MAT_V] =		tglGetUniformLocation(pass[passType].program, "teVmat");
			pass[passType].uniforms[UT_MAT_MV] =	tglGetUniformLocation(pass[passType].program, "teMVmat");
			pass[passType].uniforms[UT_MAT_MVP] =	tglGetUniformLocation(pass[passType].program, "teMVPmat");
			pass[passType].uniforms[UT_MAT_N] =		tglGetUniformLocation(pass[passType].program, "teNmat");
			pass[passType].uniforms[UT_MAT_BONES] =	tglGetUniformLocation(pass[passType].program, "teBoneMatrixes");
			pass[passType].uniforms[UT_TIME] =		tglGetUniformLocation(pass[passType].program, "teTime");
			pass[passType].uniforms[UT_USER0] =		tglGetUniformLocation(pass[passType].program, "teUser0");
			pass[passType].uniforms[UT_USER1] =		tglGetUniformLocation(pass[passType].program, "teUser1");
			pass[passType].uniforms[UT_USER2] =		tglGetUniformLocation(pass[passType].program, "teUser2");
			pass[passType].uniforms[UT_USER3] =		tglGetUniformLocation(pass[passType].program, "teUser3");

			currentPass = passType;

			for(u8 i = 0; i < teMaxTextures; ++i)
				SetUniform((EUniformType)(UT_TEX_0 + i), (s32)i);

			tglValidateProgram(pass[passType].program);

			LogErrors("validate", 0, pass[passType].program);
		}

		void teShader::LogErrors(const c8 * stageName, GLuint shader, GLuint program)
		{
			#ifdef TE_DEBUG

			GLint logLength = 0;
			GLint status = 0;

			if(shader)
			{
				tglGetShaderiv(shader, GL_COMPILE_STATUS, &status);
				tglGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
			}
			else if(program)
			{
				tglGetProgramiv(program, GL_LINK_STATUS, &status);
				tglGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
			}

			if(status)
				return;

			if(logLength > 0)
			{
				GLchar * log = (GLchar*)TE_ALLOCATE(logLength);

				if(shader)
					tglGetShaderInfoLog(shader, logLength, &logLength, log);
				else if(program)
					tglGetProgramInfoLog(program, logLength, &logLength, log);

				if(logLength > 0)
				{
					TE_LOG_ERR("shader stage %s error : %s", stageName, log);
				}

				TE_FREE(log);
			}

			#endif
		}

		#endif
	}
}
