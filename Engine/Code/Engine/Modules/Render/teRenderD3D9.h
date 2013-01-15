/*
 *  teRenderD3D9.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 11/11/12.
 *  Copyright 2011 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TERENDERD3D9_H
#define TE_TERENDERD3D9_H

#include "teRenderGL.h"

namespace te
{
	namespace video
	{
		#if defined(TE_RENDER_D3D9)
		/*
		static TE_INLINE GLenum GetDepthModeD3D9Type(EDepthMode type)
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
		*/
		#endif

		#if defined(TE_RENDER_D3D9)

		class teRenderD3D9 : public te::teReferenceCounter
		{
		public:
			teRenderD3D9();
			~teRenderD3D9();

			LPDIRECT3D9 GetD3D() {return d3d;}
			LPDIRECT3DDEVICE9 GetDevice() {return device;}

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

			void Render(const scene::teContentPack & contentPack, teSurfaceData * surface);

			const teRenderStatistic & GetRenderStatistic() const {return renderStatistic;}

		protected:
			LPDIRECT3D9 d3d;
			LPDIRECT3DDEVICE9 device;

			teRenderContext * currentContext;

			teShaderLibrary library;

			teViewport viewport;

			teMatrix4f matrixProjection;
			teMatrix4f matrixModelView;
			teMatrix4f matrixView;

			EShaderPass shaderPass;

			teRenderStatistic renderStatistic;

			LPDIRECT3DVERTEXSHADER9 shaderVertex;
			LPDIRECT3DPIXELSHADER9 shaderPixel;

			LPDIRECT3DVERTEXBUFFER9 vbo;
			LPDIRECT3DINDEXBUFFER9 ibo;

			teConstArray<LPDIRECT3DVERTEXDECLARATION9> vertexLayouts;
		};
		
		//! Get Render
		teRenderD3D9 * GetRender();

		#endif
	}
}

#endif
