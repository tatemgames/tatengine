/*
 *  teRenderGL.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 11/24/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TERENDERES_H
#define TE_TERENDERES_H

#include "teTypes.h"
#include "teReferenceCounter.h"
#include "teRenderSystem.h"
#include "teRenderContext.h"
#include "teMatrix4.h"
#include "teSurfaceData.h"
#include "teContentPack.h"
#include "teShaderLibrary.h"

namespace te
{
	namespace video
	{
		enum EMatrixType
		{
			MT_PROJECTION,
			MT_MODEL,
			MT_VIEW,
			MT_TEXTURE
		};

		enum EDepthMode
		{
			DM_NEVER = 0,
			DM_LESS,
			DM_LEQUAL,
			DM_EQUAL,
			DM_GREATER,
			DM_NOTEQUAL,
			DM_GEQUAL,
			DM_ALWAYS
		};

		#if defined(TE_RENDER_GL)
		static TE_INLINE GLenum GetDepthModeGLType(EDepthMode type)
		{
			switch(type)
			{
			case DM_NEVER:			return GL_NEVER;
			case DM_LESS:			return GL_LESS;
			case DM_LEQUAL:			return GL_LEQUAL;
			case DM_EQUAL:			return GL_EQUAL;
			case DM_GREATER:		return GL_GREATER;
			case DM_NOTEQUAL:		return GL_NOTEQUAL;
			case DM_GEQUAL:			return GL_GEQUAL;
			case DM_ALWAYS:			return GL_ALWAYS;
			default: TE_ASSERT(false);	return GL_NONE;
			}
		}
		#endif

		struct teRenderStatistic
		{
			// TODO restore
			//u32 trianglesCount;
			u32 operationsCount;
			//u32 materialChange;
			//u32 texturesChange;
			//u32 matrixChange;
			//u32 shaderChange;
			u32 vboUploads;
			u32 vboUploadsSize;

			void Clear()
			{
				//trianglesCount = 0;
				operationsCount = 0;
				//materialChange = 0;
				//texturesChange = 0;
				//matrixChange = 0;
				//shaderChange = 0;
				vboUploads = 0;
				vboUploadsSize = 0;
			}
		};
		
		struct teViewport
		{
			teVector2dsh size;
			teVector2dsh position;

			teVector2dsh scissorSize;
			teVector2dsh scissorPosition;

			teViewport(teVector2dsh setSize = teVector2dsh(), teVector2dsh setPosition = teVector2dsh(), teVector2dsh setScissorSize = teVector2dsh(), teVector2dsh setScissorPosition = teVector2dsh())
				:size(setSize), position(setPosition), scissorSize(setScissorSize), scissorPosition(setScissorPosition)
			{
			}

			TE_INLINE u1 IsEqual(const teViewport & other) const {return (size == other.size) && (position == other.position) && (scissorSize == other.scissorSize) && (scissorPosition == other.scissorPosition);}
			TE_INLINE u1 operator == (const teViewport & other) const {return IsEqual(other);}
			TE_INLINE u1 operator != (const teViewport & other) const {return !IsEqual(other);}
		};

		#define TE_RENDER_GL_CACHE
		#define TE_RENDER_GL_VAO

		#ifdef TE_RENDER_GL_VAO
		const u32 teRenderGLVAOMax = u8Max;
		#endif

		#if defined(TE_RENDER_GL)

		class teRenderGL : public te::teReferenceCounter
		{
		public:
			teRenderGL();
			~teRenderGL();

			void SetContext(teRenderContext * Context);
			teRenderContext * GetContext();

			teShaderLibrary & GetShaderLibrary() {return library;}
			const teShaderLibrary & GetShaderLibrary() const {return library;}

			void SetViewportOptions(const teViewport & setViewport) {viewport = setViewport;}
			const teViewport & GetViewportOptions() const {return viewport;}

			teMatrix4f & GetMatrixProjection() {return matrixProjection;}
			const teMatrix4f & GetMatrixProjection() const {return matrixProjection;}
			
			teMatrix4f & GetMatrixModelView() {return matrixModelView;}
			const teMatrix4f & GetMatrixModelView() const {return matrixModelView;}
			
			teMatrix4f & GetMatrixView() {return matrixView;}
			const teMatrix4f & GetMatrixView() const {return matrixView;}

			void SetPass(EShaderPass setShaderPass) {shaderPass = setShaderPass;}
			EShaderPass GetPass() const {return shaderPass;}

			void ClearScreen(const teColor4f & color = teColor4f(0.25f, 0.25f, 0.25f, 1.0f), u1 clearColor = false, f32 depth = 1.0f, u1 clearDepth = true, s32 stencil = 0, u1 clearStencil = false);
			
			void Begin();
			void End();

			void Render(const scene::teContentPack & contentPack, teSurfaceData * surface, const teColor4u & overrideDiffuseColor = teColor4u(u8Max, u8Max, u8Max, u8Max));

			const teRenderStatistic & GetRenderStatistic() const {return statistic;}

		protected:
			teRenderContext * CurrentContext;

			teShaderLibrary library;

			teViewport viewport;

			teMatrix4f matrixProjection;
			teMatrix4f matrixModelView;
			teMatrix4f matrixView;

			EShaderPass shaderPass;

			teRenderStatistic statistic;

			#ifdef TE_RENDER_GL_CACHE
				teViewport cacheViewport;
				u32 cachedBlend;
				u32 cachedTextureID[teMaterialMaxTextures];
				u8 cachedStateFlags;
				u8 cachedShaderType;
				u8 cachedShaderPass;
				u8 cachedBindedVertexLayer;
				#ifdef TE_RENDER_GL_VAO
					u32 cachedVBO;
					u32 cachedVAO;
				#endif
				u1 forceCacheSetup;
			#endif

			#ifdef TE_RENDER_GL_VAO
				u32 vao[teRenderGLVAOMax];
				teSurfaceData * vaoSurfaces[teRenderGLVAOMax];
				teVector2d<u8> vaoSurfaceShaderBind[teRenderGLVAOMax]; // pass and type
				teVector2d<teptr_t> vaoSurfaceBoundCheck;
				u32 vbo;
			#endif
		};
		
		//! Get Render
		teRenderGL * GetRender();

		#endif
	}
}

#endif
