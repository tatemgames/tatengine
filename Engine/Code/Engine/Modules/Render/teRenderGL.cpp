/*
 *  teRenderGL.cpp
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 11/24/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#include "teRenderGL.h"

#ifdef TE_RENDER_GL

#include "teRenderGLExtensions.h"
#include "teLogManager.h"
#include "teTimer.h"
#include "teApplicationManager.h"

namespace te
{
	namespace video
	{
		/*
		u32 vao[256];
		teSurfaceData * vaoSurf[256];
		u32 vbo = 0;
		u1 vboFisrtFrame = true;
		*/
		
		teRenderGL * currentRender = NULL;

		teRenderGL::teRenderGL()
		{
			CurrentContext = NULL;

			shaderPass = SP_GEN_DIFFUSE;

			//contentVBO = 0;

			currentRender = this;

			/*
			for(u32 i = 0; i < 256; ++i)
				vao[i] = u32Max;

			//----- cashes
			viewWidth = 0; viewHeight = 0;
			curBlend = u32Max;
			curTextureID = u32Max;
			curDepthFlag = u8Max;
			*/
		}

		teRenderGL::~teRenderGL()
		{
			TE_SAFE_DROP(CurrentContext);
			currentRender = NULL;
		}

		void teRenderGL::SetContext(teRenderContext * Context)
		{
			TE_SAFE_DROP(CurrentContext);
			CurrentContext = Context;
			TE_SAFE_GRAB(CurrentContext);

			InitExtensions();
			library.BuildShaders();
		}

		teRenderContext * teRenderGL::GetContext()
		{
			return CurrentContext;
		}

		void teRenderGL::ClearScreen(const teColor4f & color, u1 clearColor, f32 depth, u1 clearDepth, s32 stencil, u1 clearStencil)
		{
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
		}

		void teRenderGL::Begin()
		{
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

			/*
			#ifndef TE_PLATFORM_MAC
			if(!vbo)
				tglGenBuffers(1, &vbo);
			
			vboFisrtFrame = true;
			#endif
			*/
		}

		void teRenderGL::End()
		{
			#ifdef TE_DEBUG
			switch(glGetError())
			{
			case GL_NO_ERROR: break;
			case GL_INVALID_ENUM: TE_LOG_ERR("OpenGL Error : Invalid enum"); break;
			case GL_INVALID_VALUE: TE_LOG_ERR("OpenGL Error : Invalid value"); break;
			case GL_INVALID_OPERATION: TE_LOG_ERR("OpenGL Error : Invalid operation"); break;
			case GL_OUT_OF_MEMORY: TE_LOG_ERR("OpenGL Error : Out of memory"); break;
			#if defined(TE_OPENGL_ES_11) || defined(TE_OPENGL_15)
			case GL_STACK_OVERFLOW: TE_LOG_ERR("OpenGL Error : Stack overflow"); break;
			case GL_STACK_UNDERFLOW: TE_LOG_ERR("OpenGL Error : Stack underflow"); break;
			#endif
			default: TE_LOG_ERR("OpenGL Error : Unknown error") break;
			}
			#endif

			#if defined (TE_PLATFORM_IPHONE)
				const GLenum discards[]  = {GL_DEPTH_ATTACHMENT};
				te::app::GetApplicationManager()->GetFrameBuffer()->Bind();
				glDiscardFramebufferEXT(GL_FRAMEBUFFER, 1, discards);
			#else
				glFlush();
				glFinish();
			#endif

			if(CurrentContext)
				CurrentContext->PresentCurrentTexture();
		}

		void teRenderGL::Render(const scene::teContentPack & contentPack, teSurfaceData * surface)
		{
			if(!surface->IsMaterialValid())
				return;

			/*
#ifndef TE_PLATFORM_MAC

			if(vboFisrtFrame && contentPack.surfaceData.GetAlive())
			{
				static u8 temp = 0;

				if(temp)
					--temp;
				else
				{
					temp = 0; // wip
					tglBindBuffer(GL_ARRAY_BUFFER, vbo);
					tglBufferData(GL_ARRAY_BUFFER, contentPack.surfaceData.GetAlive(), contentPack.surfaceData.GetPool(), GL_DYNAMIC_DRAW);
					vboFisrtFrame = false;
				}
			}
#endif

			if(vao[0] == u32Max)
			{
				u32 offset = 0;
				u32 counter = 0;
				
				while(offset < contentPack.surfaceData.GetAlive())
				{
					
					//----- save surface ptr
					teSurfaceData * surface = (teSurfaceData*)contentPack.surfaceData.At(offset);
					
					if(!surface->IsMaterialValid())
					{
						offset += surface->dataSize + sizeof(teSurfaceData);
						continue;
					}
					
					vaoSurf[counter] = surface;
					
					//------ layer for cur surface
					const teSurfaceLayers & layers = contentPack.surfaceLayers[surface->layersIndex];
					
					//------ shader for cur sarface material
					teShader & shader = library.GetShader((EShaderType)contentPack.materials[surface->materialIndex].shaderIndex);
					shader.Bind(SP_GEN_DIFFUSE);
					 
					
					//----- Create and bind VAO
					tglGenVertexArrays(1, &vao[counter]);//glGenVertexArraysOES(1, &vao[counter]);
					tglBindVertexArray(vao[counter]);
					
#ifndef TE_PLATFORM_MAC
					tglBindBuffer(GL_ARRAY_BUFFER, vbo);
					tglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo);
#endif


					for(u8 i = 0; i < SLT_INDEXES; ++i)
					{
						//--- get attribut 
						GLuint uniform = shader.GetAttributes(SP_GEN_DIFFUSE, (ESurfaceLayerType)i);
												
						if(uniform == -1)
							continue;
						
						if(layers.variablesPerObject[i])
						{
						
							teptr_t a = (teptr_t)surface->data;
							teptr_t b = (teptr_t)contentPack.surfaceData.GetPool();
							
#if defined(TE_PLATFORM_MAC)
							teptr_t c = a ;//- b;
#else
							teptr_t c = a - b;
#endif
							
							// ------ Configure the attributes in the VAO.
							tglEnableVertexAttribArray(uniform);
							tglVertexAttribPointer(uniform,                         //glVertexAttribPointer, (Index, Size, Type, Normalized, Stride, Ptr)
												   layers.variablesPerObject[i],
												   layers.GetVariableGLType(i),
												   TE_GET_BIT(layers.normalized, i),
												   layers.stride[i],
												   (const GLvoid*)(c + layers.offset[i]));
						}
						else
						{
							tglDisableVertexAttribArray(uniform); //!
						}
					}

					
					tglBindVertexArray(0);
					
					offset += surface->dataSize + sizeof(teSurfaceData);

					//printf("%d \n", counter);
					counter++; // inc for each surface
				}
				

			}
			else
			{
			}
			*/
			
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

			/*
			//----- search sufface
			u32 surfID = u32Max;
			for (int i = 0; i<256; i++)
			{
				if(vaoSurf[i] == surface)
				{
					surfID = i;
					break;
				}
			}
			
			if (surfID < u32Max)  //---- with VAO
			{
				tglBindVertexArray(vao[surfID]);

				const teSurfaceLayers & layers = contentPack.surfaceLayers[surface->layersIndex];
		
				teMatrix4f matrixModelViewProjection;
				matrixModelViewProjection = matrixProjection * matrixModelView;

				teMatrix4f temp, temp2;
				temp.SetIdentity();
				temp2.SetIdentity();

				matrixModelView.Inverse33(temp);
				temp.Transpose(temp2);
				
				shader.SetUniform(UT_MAT_MVP, matrixModelViewProjection);
				shader.SetUniform(UT_MAT_MV, matrixModelView);
				shader.SetUniform(UT_MAT_V, matrixView);
				shader.SetUniform(UT_MAT_N, temp2);

				shader.SetUniform(UT_C_DIFFUSE, states.color);
				
				u32 skeletonIndex = surface->skeletonIndex;
				
				if(skeletonIndex != u32Max)
				{
					scene::teSkeleton *
					skeleton = (scene::teSkeleton*)contentPack.skeletonData.At(skeletonIndex);
					shader.SetUniform(UT_MAT_BONES, skeleton->bonesCount * 2, 4, reinterpret_cast<const f32*>(skeleton->GetSkin()));
				}

#if defined(TE_PLATFORM_MAC)				
				glDrawElements(surface->GetOperationGLType(),
							   surface->indexCount,
							   contentPack.surfaceLayers[surface->layersIndex].GetVariableGLType(SLT_INDEXES),
							   surface->GetIndexes(contentPack.surfaceLayers[surface->layersIndex]));

#else

				uintptr_t a = (uintptr_t) surface->GetIndexes(contentPack.surfaceLayers[surface->layersIndex]);

				uintptr_t b = (uintptr_t) contentPack.surfaceData.GetPool();

				glDrawElements(surface->GetOperationGLType(),
					surface->indexCount,
					contentPack.surfaceLayers[surface->layersIndex].GetVariableGLType(SLT_INDEXES),
					(const GLvoid*)(a - b));
#endif				
				 
			}
			else // ----- without VAO
			*/
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
		}

		teRenderGL * GetRender()
		{
			return currentRender;
		}
	}
}

#endif