/*
 *  teApplicationManager.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 09/15/10.
 *  Copyright 2010 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEAPPLICATIONMANAGER_H
#define TE_TEAPPLICATIONMANAGER_H

#include "TatEngineCoreConfig.h"
#include "teReferenceCounter.h"
#include "teConstArray.h"

#ifdef TE_MODULE_RENDER
	#include "teFrameBuffer.h"
	#include "teVector3D.h"
#endif

namespace te
{
	namespace app
	{
		class IApplication : public te::teReferenceCounter
		{
		protected:
			//! Current Run
			u1 CurrentRun;
		public:
			IApplication()
				:CurrentRun(true)
			{
			}

			virtual ~IApplication()
			{
			}

			virtual void OnUpdate() = 0;

			virtual void OnRender() {}

			virtual void OnPause(u1 Enabled) {}

			void SetRun(u1 Enable)
			{
				CurrentRun = Enable;
			}

			u1 IsRun()
			{
				return CurrentRun;
			}
		};

		class teApplicationManager : public te::teReferenceCounter
		{
		protected:
			u32 CurrentTick;
			u1 firstTick;

			// ------------------------------------------------------------------------------------ Application

			IApplication * CurrentApplication;

			void InitModules();
			void DeInitModules();

			// ------------------------------------------------------------------------------------ Render

#ifdef TE_MODULE_RENDER

			video::teFrameBuffer * CurrentFrameBuffer;

			void InitRender();
			void DeInitRender();

#endif

			// ------------------------------------------------------------------------------------

		public:

			teApplicationManager(u1 InitApplication = true);

			~teApplicationManager();

			void InitApplication();

			TE_INLINE IApplication * GetApplication() {return CurrentApplication;}

			void OnTick();

			void OnPause(u1 Enabled);

			u1 IsRun();

			void OnMemoryWarning();

			u32 GetTick();

			// ------------------------------------------------------------------------------------ Render

#ifdef TE_MODULE_RENDER

			void RenderBegin(u1 clearScreen = true);

			void RenderEnd();

			video::teFrameBuffer * GetFrameBuffer();

			void OnResize(const teVector2di & Size = teVector2di(0, 0));

#endif

			// ------------------------------------------------------------------------------------
		};

		teApplicationManager * GetApplicationManager();

		//! Create Application
		IApplication * teCreateApplication();

		#define TE_DEFINE_APP(__ClassName) \
			IApplication * teCreateApplication() \
			{ \
				IApplication * NewApplication = NULL; \
				TE_NEW(NewApplication, __ClassName) \
				return NewApplication; \
			}

	}
}

#endif
