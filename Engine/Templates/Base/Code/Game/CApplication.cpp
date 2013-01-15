/*
 *  CApplication.cpp
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 12/2/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#include "CApplication.h"
#include "teApplicationManager.h"
#include "teInputManager.h"

namespace te
{
	namespace scene
	{
		void RegisterActors(teActorsTI * ti)
		{
		}
	}

	namespace app
	{
		CApplication * currentApplication = NULL;

		CApplication::CApplication()
			:fastScene(NULL)
		{
			currentApplication = this;

			//core::GetFpsCounter()->SetDeltaTime(false, 1.0f);

			TE_NEW(fastScene, scene::teFastScene(&scene::RegisterActors));
			fastScene->Load(0, true);
		}

		CApplication::~CApplication()
		{
			TE_SAFE_DROP(fastScene);

			currentApplication = NULL;
		}

		void CApplication::OnUpdate()
		{
			#ifdef TE_INPUT_SUPPORT_KEYBOARD

			if(input::GetInputManager()->IsKeyHit(input::KC_ESCAPE))
				SetRun(false);

			#endif

			if(fastScene)
				fastScene->OnUpdate();
		}

		void CApplication::OnRender()
		{
			GetApplicationManager()->RenderBegin();

			//video::GetRender()->ClearScreen(254.0f / 255.0f, 254.0f / 255.0f, 254.0f / 255.0f, 0.0f, true);

			if(fastScene)
				fastScene->OnRender();

			GetApplicationManager()->RenderEnd();
		}

		CApplication * GetApplication()
		{
			return currentApplication;
		}

		TE_DEFINE_APP(CApplication)
	}
}
