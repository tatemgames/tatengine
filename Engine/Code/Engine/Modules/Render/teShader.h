/*
 *  teShader.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 11/25/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TESHADER_H
#define TE_TESHADER_H

#include "teRenderGLExtensions.h"
#include "teSurfaceData.h"
#include "teVector2D.h"
#include "teVector3D.h"
#include "teQuaternion.h"
#include "teColor.h"
#include "teMatrix4.h"

#ifdef TE_RENDER_GL_SHADERS
#include "teString.h"
#endif

namespace te
{
	namespace video
	{
		const u8 teMaxTextures = 4;

		enum EUniformType
		{
			UT_INVALID = 0,

			UT_TEX_0,
			UT_TEX_1,
			UT_TEX_2,
			UT_TEX_3,

			UT_C_DIFFUSE, // diffuse color

			UT_MAT_MVP, // model * view * project matrix
			UT_MAT_MV, // model * view matrix
			UT_MAT_V, // view matrix
			UT_MAT_N, // normal matrix

			UT_MAT_BONES,

			UT_TIME,

			UT_USER0,
			UT_USER1,
			UT_USER2,
			UT_USER3,

			UT_MAX
		};

		enum EShaderPass
		{
			SP_GEN_DIFFUSE = 0,

			SP_USER_0,
			SP_USER_1,
			SP_USER_2,
			SP_USER_3,

			SP_MAX
		};

		enum EUniformUserDataType
		{
			UUT_INVALID = 0,

			UUT_F32,		// f32, use dataPtr as f32 value, not ptr
			UUT_S32,		// s32, use dataPtr as s32 value, not ptr
			UUT_VEC2F,		// teVector2df
			UUT_VEC2I,		// teVector2di
			UUT_VEC3F,		// teVector3df
			UUT_VEC3I,		// teVector3di
			UUT_QUATF,		// teQuaternionf
			UUT_COLORF,		// teColor4f
			UUT_MATRIXF,	// teMatrix4f

			UUT_MAX
		};

		struct teUniformUserData
		{
			u8 uniformType;
			u8 userDataType;
			u16 objectsCount;
			teptr_t dataPtr;

			TE_INLINE void Clear()
			{
				uniformType = UT_INVALID;
				userDataType = UUT_INVALID;
				objectsCount = 0;
				dataPtr = NULL;
			}

			TE_INLINE u1 IsValid() const {return uniformType != UT_INVALID;}

			TE_INLINE u1 IsEqual(const teUniformUserData & other) const {return (uniformType == other.uniformType) && (userDataType == other.userDataType) && (objectsCount == other.objectsCount) && (dataPtr == other.dataPtr);}
		};

		class teShader
		{
		public:
			teShader();
			~teShader();

			#ifdef TE_RENDER_GL_SHADERS

			void SetSource(const teString & setSource);
			TE_INLINE teString & GetSource() {return source;}
			TE_INLINE const teString & GetSource() const {return source;}

			TE_INLINE GLuint GetProgram(EShaderPass passType) const {return pass[passType].program;}
			TE_INLINE GLuint GetAttributes(EShaderPass passType, ESurfaceLayerType type) const {return pass[passType].attributes[type];}
			TE_INLINE GLuint GetUniform(EShaderPass passType, EUniformType type) const {return pass[passType].uniforms[type];}

			#endif

			TE_INLINE void Bind(EShaderPass passType)
			{
				#ifdef TE_RENDER_GL_SHADERS

				currentPass = passType;

				if(!pass[currentPass].program)
					BuildPass(currentPass); // TODO need to compile shaders before rendering

				TE_ASSERT(pass[currentPass].program);
				tglUseProgram(pass[currentPass].program);

				#endif
			}

			void BindAttributes(teSurfaceData * surface, const teSurfaceLayers & layers, u1 localOffsets = false, uintptr_t basePtr = 0);

			void DisableAttributes();

			TE_FORCE_INLINE u1 IsAvailable(EUniformType type) const {return pass[currentPass].uniforms[type] != -1;}

			#ifdef TE_RENDER_GL_SHADERS

			TE_INLINE void SetUniform(EUniformType type, f32 x)							{if(IsAvailable(type)) tglUniform1f(pass[currentPass].uniforms[type], x);}
			TE_INLINE void SetUniform(EUniformType type, f32 x, f32 y)					{if(IsAvailable(type)) tglUniform2f(pass[currentPass].uniforms[type], x, y);}
			TE_INLINE void SetUniform(EUniformType type, f32 x, f32 y, f32 z)			{if(IsAvailable(type)) tglUniform3f(pass[currentPass].uniforms[type], x, y, z);}
			TE_INLINE void SetUniform(EUniformType type, f32 x, f32 y, f32 z, f32 w)	{if(IsAvailable(type)) tglUniform4f(pass[currentPass].uniforms[type], x, y, z, w);}
			TE_INLINE void SetUniform(EUniformType type, s32 x)							{if(IsAvailable(type)) tglUniform1i(pass[currentPass].uniforms[type], x);}
			TE_INLINE void SetUniform(EUniformType type, s32 x, s32 y)					{if(IsAvailable(type)) tglUniform2i(pass[currentPass].uniforms[type], x, y);}
			TE_INLINE void SetUniform(EUniformType type, s32 x, s32 y, s32 z)			{if(IsAvailable(type)) tglUniform3i(pass[currentPass].uniforms[type], x, y, z);}
			TE_INLINE void SetUniform(EUniformType type, s32 x, s32 y, s32 z, s32 w)	{if(IsAvailable(type)) tglUniform4i(pass[currentPass].uniforms[type], x, y, z, w);}
			TE_INLINE void SetUniform(EUniformType type, u32 matrixCount, u8 matrixSize, u1 transpose, const f32 * data)
			{
				if(IsAvailable(type))
				{
					switch(matrixSize)
					{
					case 2: tglUniformMatrix2fv(pass[currentPass].uniforms[type], matrixCount, transpose, data); break;
					case 3: tglUniformMatrix3fv(pass[currentPass].uniforms[type], matrixCount, transpose, data); break;
					case 4: tglUniformMatrix4fv(pass[currentPass].uniforms[type], matrixCount, transpose, data); break;
					default: TE_ASSERT(0);
					}
				}
			}
			TE_INLINE void SetUniform(EUniformType type, u32 objectsCount, u8 variablesPerObject, const f32 * data)
			{
				if(IsAvailable(type))
				{
					switch(variablesPerObject)
					{
					case 1: tglUniform1fv(pass[currentPass].uniforms[type], objectsCount, data); break;
					case 2: tglUniform2fv(pass[currentPass].uniforms[type], objectsCount, data); break;
					case 3: tglUniform3fv(pass[currentPass].uniforms[type], objectsCount, data); break;
					case 4: tglUniform4fv(pass[currentPass].uniforms[type], objectsCount, data); break;
					default: TE_ASSERT(0); break;
					}
				}
			}
			TE_INLINE void SetUniform(EUniformType type, u32 objectsCount, u8 variablesPerObject, const s32 * data)
			{
				if(IsAvailable(type))
				{
					switch(variablesPerObject)
					{
					case 1: tglUniform1iv(pass[currentPass].uniforms[type], objectsCount, data); break;
					case 2: tglUniform2iv(pass[currentPass].uniforms[type], objectsCount, data); break;
					case 3: tglUniform3iv(pass[currentPass].uniforms[type], objectsCount, data); break;
					case 4: tglUniform4iv(pass[currentPass].uniforms[type], objectsCount, data); break;
					default: TE_ASSERT(0); break;
					}
				}
			}

			#endif

			#ifdef TE_RENDER_D3D9

			TE_INLINE void SetUniform(EUniformType type, f32 x)							{}
			TE_INLINE void SetUniform(EUniformType type, f32 x, f32 y)					{}
			TE_INLINE void SetUniform(EUniformType type, f32 x, f32 y, f32 z)			{}
			TE_INLINE void SetUniform(EUniformType type, f32 x, f32 y, f32 z, f32 w)	{}
			TE_INLINE void SetUniform(EUniformType type, s32 x)							{}
			TE_INLINE void SetUniform(EUniformType type, s32 x, s32 y)					{}
			TE_INLINE void SetUniform(EUniformType type, s32 x, s32 y, s32 z)			{}
			TE_INLINE void SetUniform(EUniformType type, s32 x, s32 y, s32 z, s32 w)	{}
			TE_INLINE void SetUniform(EUniformType type, u32 matrixCount, u8 matrixSize, u1 transpose, const f32 * data) {}
			TE_INLINE void SetUniform(EUniformType type, u32 objectsCount, u8 variablesPerObject, const f32 * data) {}
			TE_INLINE void SetUniform(EUniformType type, u32 objectsCount, u8 variablesPerObject, const s32 * data) {}

			#endif

			TE_INLINE void SetUniform(EUniformType type, const teVector2df & vector) {SetUniform(type, vector.x, vector.y);}
			TE_INLINE void SetUniform(EUniformType type, const teVector3df & vector) {SetUniform(type, vector.x, vector.y, vector.z);}
			TE_INLINE void SetUniform(EUniformType type, const teQuaternionf & vector) {SetUniform(type, vector.x, vector.y, vector.z, vector.w);}
			TE_INLINE void SetUniform(EUniformType type, const teColor4f & color) {SetUniform(type, color.r, color.g, color.b, color.a);}
			TE_INLINE void SetUniform(EUniformType type, const teVector2di & vector) {SetUniform(type, vector.x, vector.y);}
			TE_INLINE void SetUniform(EUniformType type, const teVector3di & vector) {SetUniform(type, vector.x, vector.y, vector.z);}
			TE_INLINE void SetUniform(EUniformType type, const teMatrix4f & matrix) {SetUniform(type, 1, 4, false, matrix.m);};

		protected:
			#ifdef TE_RENDER_GL_SHADERS

			teString source;

			struct tePassInfo
			{
				GLuint program;
				GLint attributes[SLT_MAX];
				GLint uniforms[UT_MAX];

				tePassInfo()
				{
					program = NULL;

					for(u8 i = 0; i < SLT_MAX; ++i)
						attributes[i] = 0;

					for(u8 i = 0; i < UT_MAX; ++i)
						uniforms[i] = 0;
				}

				~tePassInfo()
				{
					if(program)
						tglDeleteProgram(program);
				}
			};

			tePassInfo pass[SP_MAX];
			EShaderPass currentPass;

			void BuildPass(EShaderPass passType);

			void LogErrors(const c8 * stageName, GLuint shader = 0, GLuint program = 0);

			#endif
		};
	}
}

#endif
