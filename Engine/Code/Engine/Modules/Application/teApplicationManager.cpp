/*
 *  teApplicationManager.cpp
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 09/15/10.
 *  Copyright 2010 Tatem Games. All rights reserved.
 *
 */

#include "teApplicationManager.h"
#include "teLogManager.h"
#include "tePlatform.h"
#include "teFpsCounter.h"
#include "teFileManager.h"

#include "teFileCompress.h"

#ifdef TE_MODULE_INPUT
	#include "teInputManager.h"
#endif

#ifdef TE_MODULE_RENDER
	#include "teRender.h"
	#include "teRenderContext.h"
#endif

#ifdef TE_MODULE_REMOTEDEBUGGER
	#include "teRemoteDebugger.h"
#endif

#ifdef TE_MODULE_SOUND
	#include "teSoundManager.h"
#endif

#ifdef TE_MODULE_NETWORK
	#include "teHTTPNetwork.h"
#endif

#ifdef TE_MODULE_PUBLISHING
	#include "tePublishingManager.h"
#endif

namespace te
{
	namespace app
	{
		teApplicationManager * CurrentApplicationManager = NULL;

		teApplicationManager::teApplicationManager(u1 NeedInitApplication)
		{
			#ifdef TE_DEBUG
				CurrentTick = 0;
			#endif

			firstTick = true;

			CurrentApplication = NULL;

			InitModules();

			CurrentApplicationManager = this;

			if(NeedInitApplication)
				InitApplication();
		}

		teApplicationManager::~teApplicationManager()
		{
			CurrentApplication->Drop();

			DeInitModules();

			CurrentApplicationManager = NULL;
		}

		void teApplicationManager::InitApplication()
		{
			CurrentApplication = teCreateApplication();
		}

		void teApplicationManager::OnTick()
		{
			if(firstTick)
			{
				u32 currentVersion = 0;
				core::IBuffer * versionBuffer = core::GetFileManager()->OpenFile(core::GetFileManager()->GetPathConcate().Add(core::GetPlatform()->GetFileSystem()->GetPath(core::FPT_RESOURCES_UNPACKED).c_str()).Add("data0.lz4.version").BakeToString().c_str(), core::CFileBuffer::FWM_READ, false, false);
				if(versionBuffer)
				{
					versionBuffer->Lock(core::BLT_READ);
					versionBuffer->SetPosition(0);
					versionBuffer->Read(&currentVersion, sizeof(u32));
					versionBuffer->Unlock();
				}
				TE_SAFE_DROP(versionBuffer);

				u32 newVersion = 0;
				versionBuffer = core::GetFileManager()->OpenFile("data0.lz4.version", core::CFileBuffer::FWM_READ, true, false);
				if(versionBuffer)
				{
					versionBuffer->Lock(core::BLT_READ);
					versionBuffer->SetPosition(0);
					versionBuffer->Read(&newVersion, sizeof(u32));
					versionBuffer->Unlock();
				}
				TE_SAFE_DROP(versionBuffer);

				//if(!currentVersion || (currentVersion != newVersion))
				{
					#if defined(TE_MODULE_SCENEMANAGER)
						core::teDecodeFile("data0.lz4", "data0.zip");
//						core::GetFileManager()->AddPak("data0.zip");
					#endif

					core::GetFileManager()->UnZipFile("data0.zip");

					core::IBuffer * versionBuffer = core::GetFileManager()->OpenFile(core::GetFileManager()->GetPathConcate().Add(core::GetPlatform()->GetFileSystem()->GetPath(core::FPT_RESOURCES_UNPACKED).c_str()).Add("data0.lz4.version").BakeToString().c_str(), core::CFileBuffer::FWM_WRITE, false, false);
					if(versionBuffer)
					{
						versionBuffer->Lock(core::BLT_WRITE);
						versionBuffer->SetPosition(0);
						versionBuffer->Write(&newVersion, sizeof(u32));
						versionBuffer->Unlock();
					}
					TE_SAFE_DROP(versionBuffer);
				}

				firstTick = false;
			}

			#ifdef TE_DEBUG
				++CurrentTick;
			#endif

			#ifdef TE_MODULE_NETWORK
				u32 temp;
				net::GetHTTPNetwork()->Read(temp);
			#endif

			if(CurrentApplication)
				CurrentApplication->OnUpdate();

			#if defined(TE_MODULE_INPUT) && defined(TE_INPUT_SUPPORT_KEYBOARD)
				input::GetInputManager()->FlushKeysHits();
			#endif

			#if defined(TE_MODULE_RENDER)
				if(CurrentApplication)
					CurrentApplication->OnRender();
			#endif

			#ifdef TE_MODULE_REMOTEDEBUGGER
				dbg::GetRemoteDebugger()->OnTick();
			#endif

			core::GetFpsCounter()->OnTick();
		}

		void teApplicationManager::OnPause(u1 Enabled)
		{
			if(CurrentApplication)
				CurrentApplication->OnPause(Enabled);
		}

		u1 teApplicationManager::IsRun()
		{
			return CurrentApplication->IsRun();
		}

		void teApplicationManager::OnMemoryWarning()
		{
			TE_LOG_WRN("Low memory warning")
		}

		u32 teApplicationManager::GetTick()
		{
			#ifdef TE_DEBUG
				return CurrentTick;
			#else
				return 0;
			#endif
		}

		// ------------------------------------------------------------------------------------ Application

		void teApplicationManager::InitModules()
		{
			core::GetFileManager()->AddToLookupPaths(core::GetPlatform()->GetFileSystem()->GetPath(core::FPT_USER_RESOURCES));
			core::GetFileManager()->AddToLookupPaths(core::GetPlatform()->GetFileSystem()->GetPath(core::FPT_RESOURCES));
			core::GetFileManager()->AddToLookupPaths(core::GetPlatform()->GetFileSystem()->GetPath(core::FPT_RESOURCES_UNPACKED));
			core::GetFileManager()->AddToLookupPaths(core::GetPlatform()->GetFileSystem()->GetPath(core::FPT_CONTENT));
			core::GetFileManager()->AddToLookupPaths(core::GetPlatform()->GetFileSystem()->GetPath(core::FPT_DEFAULT));

			#ifdef TE_MODULE_RENDER
				InitRender();
			#endif

			#ifdef TE_MODULE_INPUT
				if(!input::GetInputManager())
					TE_NEW_S(input::teInputManager)
				else
					input::GetInputManager()->Grab();
			#endif

			#ifdef TE_MODULE_REMOTEDEBUGGER
				TE_ASSERT_NODEBUG(!dbg::GetRemoteDebugger());
				TE_NEW_S(dbg::teRemoteDebugger)
			#endif

			#ifdef TE_MODULE_SOUND
				TE_NEW_S(sound::teSoundManager)
			#endif

			#ifdef TE_MODULE_NETWORK
				TE_NEW_S(net::teHTTPNetwork);
			#endif

			#ifdef TE_MODULE_PUBLISHING
				TE_NEW_S(pbl::tePublishingManager)
			#endif
		}

		void teApplicationManager::DeInitModules()
		{
			#ifdef TE_MODULE_REMOTEDEBUGGER
				TE_DELETE(dbg::GetRemoteDebugger())
			#endif

			#ifdef TE_MODULE_INPUT
				input::GetInputManager()->Drop();
			#endif

			#ifdef TE_MODULE_RENDER
				DeInitRender();
			#endif

			#ifdef TE_MODULE_SOUND
				TE_DELETE(sound::GetSoundManager())
			#endif

			#ifdef TE_MODULE_NETWORK
				TE_DELETE(net::GetHTTPNetwork())
			#endif

			#ifdef TE_MODULE_PUBLISHING
				TE_DELETE(pbl::GetPublishingManager())
			#endif
		}


		// ------------------------------------------------------------------------------------ Render

#ifdef TE_MODULE_RENDER

		void teApplicationManager::InitRender()
		{
			#ifdef TE_RENDER_GL

			if(!video::GetRender())
			{
				video::EGAPIVersion PreferableVersion, SafeModeVersion;

				#if defined(TE_OPENGL_21)
					PreferableVersion = video::GAPI_OGL_21;

					#if defined(TE_OPENGL_15)
						SafeModeVersion = video::GAPI_OGL_15;
					#endif
				#elif defined(TE_OPENGL_15)
					PreferableVersion = video::GAPI_OGL_15;
				#endif

				#if defined(TE_OPENGL_ES_20)
					PreferableVersion = video::GAPI_OGL_ES_20;

					#if defined(TE_OPENGL_ES_11)
						SafeModeVersion = video::GAPI_OGL_ES_11;
					#endif
				#elif defined(TE_OPENGL_ES_11)
					PreferableVersion = video::GAPI_OGL_ES_11;
				#endif

				video::teRenderContext * RenderContext = NULL;

				TE_NEW(RenderContext, video::teRenderContext(PreferableVersion))

				if(!RenderContext->IsComplete())
				{
					RenderContext->Drop();

					TE_NEW(RenderContext, video::teRenderContext(SafeModeVersion))

					if(!RenderContext->IsComplete())
					{
						TE_LOG_ERR("Cant init render")
						RenderContext->Drop();
					}
				}

				RenderContext->Bind();

				TE_NEW_S(video::teRenderGL)

				video::GetRender()->SetContext(RenderContext);
				RenderContext->Drop();
			}
			else
				video::GetRender()->Grab();

			#ifdef TE_PLATFORM_IPHONE
				TE_NEW(CurrentFrameBuffer, te::video::teFrameBuffer)
				CurrentFrameBuffer->Init();
				CurrentFrameBuffer->Bind();

				video::teTexture * ColorTexture = NULL;
				TE_NEW(ColorTexture, video::teTexture);
				ColorTexture->Init(video::TT_RENDERBUFFER, video::TF_DEFAULT);
				CurrentFrameBuffer->SetTexture(video::FBL_COLOR_0, ColorTexture);
			
				video::teTexture * DepthTexture = NULL;
				TE_NEW(DepthTexture, video::teTexture);
				DepthTexture->Init(video::TT_RENDERBUFFER, video::TF_DEFAULT);
				CurrentFrameBuffer->SetTexture(video::FBL_DEPTH, DepthTexture);
			
				CurrentFrameBuffer->UnBind();
			#else
				CurrentFrameBuffer = NULL;
			#endif

			#endif

			#ifdef TE_RENDER_D3D9
			if(!video::GetRender())
			{
				video::teRenderContext * renderContext = NULL;
				TE_NEW(renderContext, video::teRenderContext);
				TE_NEW_S(video::teRenderD3D9)
				video::GetRender()->SetContext(renderContext);
				renderContext->Drop();
			}
			else
				video::GetRender()->Grab();

			#endif
		}

		void teApplicationManager::DeInitRender()
		{
			#ifdef TE_RENDER_GL

			if(CurrentFrameBuffer)
				CurrentFrameBuffer->Deinit();

			#endif

			video::GetRender()->Drop();
		}

		void teApplicationManager::RenderBegin()
		{
			video::GetRender()->Begin();
			video::GetRender()->ClearScreen(teColor4u(64, 64, 64, 255), true, 255.0f, true);
		}

		void teApplicationManager::RenderEnd()
		{
			#ifdef TE_RENDER_GL

			#ifdef TE_PLATFORM_IPHONE
				CurrentFrameBuffer->Bind();
				CurrentFrameBuffer->textures[video::FBL_COLOR_0]->Bind();
			#endif

			#endif

			video::GetRender()->End();
		}

		video::teFrameBuffer * teApplicationManager::GetFrameBuffer()
		{
			return CurrentFrameBuffer;
		}

		void teApplicationManager::OnResize(const teVector2di & Size)
		{
			#ifdef TE_RENDER_GL

			#ifdef TE_PLATFORM_IPHONE
				if(Size.GetDistance() > 0.0f)
				{
					if(video::GetRender()->GetContext())
						video::GetRender()->GetContext()->SetSize(Size);

					//video::GetRender()->SetViewportOptions(video::teRenderGL::ViewportOptions(Size));
				}
				else
				{
					video::teTexture * ColorTexture = CurrentFrameBuffer->textures[video::FBL_COLOR_0];
					ColorTexture->UpdateSize();

					if(video::GetRender()->GetContext())
						video::GetRender()->GetContext()->SetSize(ColorTexture->size);

					//video::GetRender()->SetViewportOptions(video::teRenderGL::ViewportOptions(ColorTexture->GetSize()));
				}
			#else
				if(video::GetRender()->GetContext())
					video::GetRender()->GetContext()->SetSize(Size);

				//video::GetRender()->SetViewportOptions(video::teRenderGL::ViewportOptions(Size));
			#endif

			#endif

			#ifdef TE_RENDER_D3D9
				if(video::GetRender()->GetContext())
					video::GetRender()->GetContext()->SetSize(Size);
			#endif
		}

#endif

		// ------------------------------------------------------------------------------------

		teApplicationManager * GetApplicationManager()
		{
			return CurrentApplicationManager;
		}
	}
}
