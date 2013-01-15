/*
 *  teRenderD3D9.cpp
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 11/11/12.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#include "teRenderD3D9.h"

#ifdef TE_RENDER_D3D9
#pragma comment (lib, "d3d9.lib")
#include "teLogManager.h"
#include "teTimer.h"
#include "teApplicationManager.h"
#include "teWinDevice.h"

//#include "shader_default_vs.h"
//#include "shader_default_ps.h"

#include "teAssetSprite.h"

const BYTE g_vs20_vs_main[] =
{
	0,   2, 254, 255, 254, 255, 
	35,   0,  67,  84,  65,  66, 
	28,   0,   0,   0,  87,   0, 
	0,   0,   0,   2, 254, 255, 
	1,   0,   0,   0,  28,   0, 
	0,   0,  16,   1,   0,   0, 
	80,   0,   0,   0,  48,   0, 
	0,   0,   2,   0,   0,   0, 
	4,   0,   0,   0,  64,   0, 
	0,   0,   0,   0,   0,   0, 
	87, 111, 114, 108, 100,  86, 
	105, 101, 119,  80, 114, 111, 
	106,   0, 171, 171,   3,   0, 
	3,   0,   4,   0,   4,   0, 
	1,   0,   0,   0,   0,   0, 
	0,   0, 118, 115,  95,  50, 
	95,  48,   0,  77, 105,  99, 
	114, 111, 115, 111, 102, 116, 
	32,  40,  82,  41,  32,  72, 
	76,  83,  76,  32,  83, 104, 
	97, 100, 101, 114,  32,  67, 
	111, 109, 112, 105, 108, 101, 
	114,  32,  57,  46,  50,  55, 
	46,  57,  53,  50,  46,  51, 
	48,  50,  50,   0,  31,   0, 
	0,   2,   0,   0,   0, 128, 
	0,   0,  15, 144,  31,   0, 
	0,   2,   5,   0,   0, 128, 
	1,   0,  15, 144,   9,   0, 
	0,   3,   0,   0,   1, 192, 
	0,   0, 228, 144,   0,   0, 
	228, 160,   9,   0,   0,   3, 
	0,   0,   2, 192,   0,   0, 
	228, 144,   1,   0, 228, 160, 
	9,   0,   0,   3,   0,   0, 
	4, 192,   0,   0, 228, 144, 
	2,   0, 228, 160,   9,   0, 
	0,   3,   0,   0,   8, 192, 
	0,   0, 228, 144,   3,   0, 
	228, 160,   1,   0,   0,   2, 
	0,   0,   3, 224,   1,   0, 
	228, 144, 255, 255,   0,   0
};


const BYTE g_ps20_ps_main[] =
{
	0,   2, 255, 255, 254, 255, 
	33,   0,  67,  84,  65,  66, 
	28,   0,   0,   0,  79,   0, 
	0,   0,   0,   2, 255, 255, 
	1,   0,   0,   0,  28,   0, 
	0,   0,  16,   1,   0,   0, 
	72,   0,   0,   0,  48,   0, 
	0,   0,   3,   0,   0,   0, 
	1,   0,   0,   0,  56,   0, 
	0,   0,   0,   0,   0,   0, 
	84, 101, 120,  48,   0, 171, 
	171, 171,   4,   0,  12,   0, 
	1,   0,   1,   0,   1,   0, 
	0,   0,   0,   0,   0,   0, 
	112, 115,  95,  50,  95,  48, 
	0,  77, 105,  99, 114, 111, 
	115, 111, 102, 116,  32,  40, 
	82,  41,  32,  72,  76,  83, 
	76,  32,  83, 104,  97, 100, 
	101, 114,  32,  67, 111, 109, 
	112, 105, 108, 101, 114,  32, 
	57,  46,  50,  55,  46,  57, 
	53,  50,  46,  51,  48,  50, 
	50,   0,  31,   0,   0,   2, 
	0,   0,   0, 128,   0,   0, 
	3, 176,  31,   0,   0,   2, 
	0,   0,   0, 144,   0,   8, 
	15, 160,  66,   0,   0,   3, 
	0,   0,  15, 128,   0,   0, 
	228, 176,   0,   8, 228, 160, 
	1,   0,   0,   2,   0,   8, 
	15, 128,   0,   0, 228, 128, 
	255, 255,   0,   0
};


namespace te
{
	namespace video
	{
		teRenderD3D9 * currentRender = NULL;

		teRenderD3D9::teRenderD3D9()
			:d3d(NULL), device(NULL)
		{
			currentContext = NULL;

			shaderPass = SP_GEN_DIFFUSE;

			currentRender = this;
		}

		teRenderD3D9::~teRenderD3D9()
		{
			SetContext(NULL);
			currentRender = NULL;
		}

		void teRenderD3D9::SetContext(teRenderContext * Context)
		{
			TE_SAFE_DROP(currentContext);
			currentContext = Context;
			TE_SAFE_GRAB(currentContext);

			//InitExtensions();
			//library.BuildShaders();

			if(d3d)
			{
				shaderVertex->Release();
				shaderPixel->Release();

				device->Release();
				d3d->Release();
			}

			if(currentContext)
			{
				d3d = Direct3DCreate9(D3D_SDK_VERSION);

				HWND hWnd = game::GetWinDevice()->GetWindowHandle();

				D3DPRESENT_PARAMETERS d3dpp;
				ZeroMemory(&d3dpp, sizeof(d3dpp));
				d3dpp.Windowed = TRUE;
				d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
				d3dpp.hDeviceWindow = hWnd;

				d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &device);

				device->CreateVertexShader((DWORD*)g_vs20_vs_main, &shaderVertex);
				device->CreatePixelShader((DWORD*)g_ps20_ps_main, &shaderPixel);

				device->CreateVertexBuffer(128 * 1024, 0, 0, D3DPOOL_MANAGED, &vbo, NULL);
				device->CreateIndexBuffer(128 * 1024, 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &ibo, NULL);
			}
		}

		teRenderContext * teRenderD3D9::GetContext()
		{
			return currentContext;
		}

		void teRenderD3D9::ClearScreen(const teColor4f & color, u1 clearColor, f32 depth, u1 clearDepth, s32 stencil, u1 clearStencil)
		{
			device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(64, 64, 64), 1.0f, 0);

			/*
			if(clearColor)
				glClearColor(color.r, color.g, color.b, color.a);

			#if defined(TE_PLATFORM_IPHONE) || defined(TE_PLATFORM_LINUX_MOBILE) || defined(TE_PLATFORM_ANDROID)
				if(clearDepth)
					glClearDepthf(depth);
			#else
				if(clearDepth)
					glClearDepth(depth);
			#endif

			if(clearDepth)
			{
				glEnable(GL_DEPTH_TEST);
				glDepthMask(true);
				//glDepthFunc(GetDepthModeGLType(DM_LEQUAL));
			}

			if(clearStencil)
				glClearStencil(stencil);

			glClear((clearColor ? GL_COLOR_BUFFER_BIT : 0) | (clearDepth ? GL_DEPTH_BUFFER_BIT : 0) | (clearStencil ? GL_STENCIL_BUFFER_BIT : 0));
			*/
		}

		void teRenderD3D9::Begin()
		{
			D3DVIEWPORT9 viewport;
			viewport.X = 0;
			viewport.Y = 0;
			viewport.Width = 1024;
			viewport.Height = 768;
			viewport.MinZ = -1.0f;
			viewport.MaxZ = 1.0f;
			device->SetViewport(&viewport);

			device->BeginScene();

			/*
			//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

			glEnable(GL_CULL_FACE);
			glFrontFace(GL_CCW);
			glCullFace(GL_BACK);
			
			#ifndef TE_PLATFORM_IPHONE
				glEnable(GL_TEXTURE_2D);
			#endif

			//if(viewport.size.x != viewWidth)
			{
				//viewWidth = viewport.size.x;
				//viewHeight = viewport.size.y;
				glViewport(viewport.position.x, viewport.position.y, viewport.size.x, viewport.size.y);

				if(viewport.scissorSize.GetMaxComponent() > 0)
				{
					glEnable(GL_SCISSOR_TEST);
					glScissor(viewport.scissorPosition.x, viewport.scissorPosition.y, viewport.scissorSize.x, viewport.scissorSize.y);
				}
				else
					glDisable(GL_SCISSOR_TEST);
			}
			*/
		}

		void teRenderD3D9::End()
		{
			device->EndScene();
			device->Present(NULL, NULL, NULL, NULL);
		}

		void teRenderD3D9::Render(const scene::teContentPack & contentPack, teSurfaceData * surface)
		{
			if(!vertexLayouts.GetAlive())
			{
				vertexLayouts.Reserve(contentPack.surfaceLayers.GetAlive());
				vertexLayouts.Request(vertexLayouts.GetSize());

				for(u32 i = 0; i < contentPack.surfaceLayers.GetAlive(); ++i)
				{
					if(i == 0)
					{
						D3DVERTEXELEMENT9 vertexDecl[] =
						{
							{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
							{0, 12, D3DDECLTYPE_USHORT2N, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
							{0, 16, D3DDECLTYPE_UBYTE4N, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
							D3DDECL_END()
						};

						device->CreateVertexDeclaration(vertexDecl, &vertexLayouts.GetPool()[0]);
					}
				}
			}

			if(!surface->IsMaterialValid())
				return;

			const video::teMaterial & states = contentPack.materials[surface->materialIndex];
			
			const teSurfaceLayers & suraceLayers = contentPack.surfaceLayers[surface->layersIndex];

			scene::teSpriteVertex * ptrVertexes;
			vbo->Lock(0, surface->vertexCount * suraceLayers.stride[SLT_POSITION], (void**)&ptrVertexes, 0);
			memcpy(ptrVertexes, surface->data, surface->vertexCount * suraceLayers.stride[SLT_POSITION]);
			vbo->Unlock();

			u16 * ptrIndeces;
			ibo->Lock(0, surface->indexCount * suraceLayers.GetVariableSize(SLT_INDEXES), (void**)&ptrIndeces, 0);
			memcpy(ptrIndeces, surface->data + surface->indexesOffset, surface->indexCount * suraceLayers.GetVariableSize(SLT_INDEXES));
			ibo->Unlock();

			teMatrix4f matrixModelViewProjection;
			matrixModelViewProjection = matrixProjection * matrixModelView;

			teMatrix4f mat2;
			matrixModelViewProjection.Transpose(mat2);

			device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);

			for(u8 i = 0; i < teMaterialMaxTextures; ++i)
				if(states.atlasSpriteIndex[i] != u32Max)
					contentPack.textures[contentPack.atlasSprites[states.atlasSpriteIndex[i]].textureIndex].Bind(i);


			device->SetVertexShaderConstantF(0, matrixModelViewProjection.m, 4);
			device->SetIndices(ibo);
			device->SetVertexDeclaration(vertexLayouts[0]);
			device->SetVertexShader(shaderVertex);
			device->SetPixelShader(shaderPixel);
			device->SetStreamSource(0, vbo, 0, sizeof(scene::teSpriteVertex));
			device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, surface->vertexCount, 0, surface->indexCount);



			/*
			const video::teMaterial & states = contentPack.materials[surface->materialIndex];

			//if(viewport.size.x != viewWidth)
			{
				//viewWidth = viewport.size.x;
				//viewHeight = viewport.size.y;
				glViewport(viewport.position.x, viewport.position.y, viewport.size.x, viewport.size.y);

				if(viewport.scissorSize.GetMaxComponent() > 0)
				{
					glEnable(GL_SCISSOR_TEST);
					glScissor(viewport.scissorPosition.x, viewport.scissorPosition.y, viewport.scissorSize.x, viewport.scissorSize.y);
				}
				else
					glDisable(GL_SCISSOR_TEST);
			}
			
			//if(curBlend != states.blend)
			{
				switch(states.blend) // TODO cache state --OK
				{
				case BT_DISABLED: glDisable(GL_BLEND); break;
				case BT_ALPHA: glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); break;
				case BT_LIGHT: glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA ,GL_ONE); break;
				case BT_SHADE: glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ZERO); break;
				case BT_ADDITIVE: glEnable(GL_BLEND); glBlendFunc(GL_ONE, GL_ONE); break;
				case BT_MULTIPLY: glEnable(GL_BLEND); glBlendFunc(GL_DST_COLOR, GL_ZERO); break;
				default: TE_ASSERT(0); break;
				}
				
				//curBlend = states.blend;
			}

			for(u8 i = 0; i < teMaterialMaxTextures; ++i)
				if(states.atlasSpriteIndex[i] != u32Max)
				{
					//if(curTextureID != contentPack.atlasSprites[states.atlasSpriteIndex[i]].textureIndex)
					{
						contentPack.textures[contentPack.atlasSprites[states.atlasSpriteIndex[i]].textureIndex].Bind(i);
						
						//curTextureID = contentPack.atlasSprites[states.atlasSpriteIndex[i]].textureIndex;
					}
				}
			
			
			//if (curDepthFlag != states.IsFlag(MF_DEPTH_BUFFER))
			{
				glDepthMask(states.IsFlag(MF_DEPTH_BUFFER_WRITE));

				if(states.IsFlag(MF_DEPTH_BUFFER_TEST))
				{
					glEnable(GL_DEPTH_TEST);
					glDepthFunc(GetDepthModeGLType(DM_LEQUAL));
				}
				else
				{
					glDepthFunc(GetDepthModeGLType(DM_ALWAYS));
					glDisable(GL_DEPTH_TEST);
				}
				
				//curDepthFlag = states.IsFlag(MF_DEPTH_BUFFER);
			}
			
			
			teShader & shader = library.GetShader((EShaderType)states.shaderIndex);
			//teShader & shader = library.GetShader(ST_FAILSAFE);

			shader.Bind(shaderPass);

			shader.SetUniform(UT_TIME, TE_TIME_32);

			for(u8 i = 0; i < teMaterialMaxUniformUserData; ++i)
			{
				if(!states.userData[i].IsValid())
					continue;

				EUniformType type = (EUniformType)states.userData[i].uniformType;

				if(states.userData[i].objectsCount == 1)
				{
					switch(states.userData[i].userDataType)
					{
					case UUT_F32: shader.SetUniform(type, *reinterpret_cast<const f32*>(&states.userData[i].dataPtr)); break;
					case UUT_S32: shader.SetUniform(type, *reinterpret_cast<const s32*>(&states.userData[i].dataPtr)); break;
					case UUT_VEC2F: shader.SetUniform(type, *reinterpret_cast<teVector2df*>(states.userData[i].dataPtr)); break;
					case UUT_VEC2I: shader.SetUniform(type, *reinterpret_cast<teVector2di*>(states.userData[i].dataPtr)); break;
					case UUT_VEC3F: shader.SetUniform(type, *reinterpret_cast<teVector3df*>(states.userData[i].dataPtr)); break;
					case UUT_VEC3I: shader.SetUniform(type, *reinterpret_cast<teVector3di*>(states.userData[i].dataPtr)); break;
					case UUT_QUATF: shader.SetUniform(type, *reinterpret_cast<teQuaternionf*>(states.userData[i].dataPtr)); break;
					case UUT_COLORF: shader.SetUniform(type, *reinterpret_cast<teColor4f*>(states.userData[i].dataPtr)); break;
					case UUT_MATRIXF: shader.SetUniform(type, *reinterpret_cast<teMatrix4f*>(states.userData[i].dataPtr)); break;
					default:
						break;
					}
				}
				else
				{
					// TODO
					TE_ASSERT(0);
				}
			}

			{

				//tglBindVertexArray(0);

				//tglBindBuffer(GL_ARRAY_BUFFER, 0);
				//tglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
				
				shader.BindAttributes(surface, contentPack.surfaceLayers[surface->layersIndex]);

				teMatrix4f matrixModelViewProjection;
				matrixModelViewProjection = matrixProjection * matrixModelView;

				shader.SetUniform(UT_MAT_MVP, matrixModelViewProjection);
				shader.SetUniform(UT_C_DIFFUSE, states.color);

				u32 skeletonIndex = surface->skeletonIndex;

				if(skeletonIndex != u32Max)
				{
					scene::teSkeleton * skeleton = (scene::teSkeleton*)contentPack.skeletonData.At(skeletonIndex);
					shader.SetUniform(UT_MAT_BONES, skeleton->bonesCount * 2, 4, reinterpret_cast<const f32*>(skeleton->GetSkin()));
				}

				glDrawElements(surface->GetOperationGLType(),
					surface->indexCount,
					contentPack.surfaceLayers[surface->layersIndex].GetVariableGLType(SLT_INDEXES),
					surface->GetIndexes(contentPack.surfaceLayers[surface->layersIndex]));

				shader.DisableAttributes();
			}
			*/
		}

		teRenderD3D9 * GetRender()
		{
			return currentRender;
		}
	}
}

#endif