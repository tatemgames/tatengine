/*
 *  teWinDevice.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 12/13/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEWINDEVICE_H
#define TE_TEWINDEVICE_H

#include "TatEngineCoreConfig.h"

#ifdef TE_PLATFORM_WIN

#include "teVector2d.h"
#include <windows.h>

#ifdef TE_MODULE_RENDER
#include "teRenderSystem.h"
#endif

namespace te
{
	namespace game
	{
		//! Win device
		class teWinDevice
		{
		protected:
			HWND WindowHandle;
			HDC DeviceContext;
			HINSTANCE AppInstance;
			DEVMODE DefaultMode;
			HGLRC RenderContext;

			u1 CurrentRun;
			u1 CurrentActive;
			teVector2di CurrentSize;
		public:
			//! Constructor
			teWinDevice(const teVector2di & Size);

			//! Destructor
			~teWinDevice();

			//! Poll
			void Poll();

			//! Present Buffer
			void PresentBuffer();

			//! Set Run
			void SetRun(u1 Run);

			//! Get Run
			u1 GetRun();

			//! Set Active
			void SetActive(u1 Active);

			//! Get Active
			u1 GetActive();

			//! Set Size
			void SetSize(const teVector2di & Size);

			//! Get Size
			const teVector2di & GetSize();

			HWND GetWindowHandle() {return WindowHandle;}
		};

		//! Get Device
		teWinDevice * GetWinDevice();
	}
}

#endif

#endif