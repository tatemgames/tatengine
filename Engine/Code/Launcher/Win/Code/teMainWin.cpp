//
//  teMainGame.cpp
//  TatEngine
//
//  Created by Dmitrii Ivanov on 12/13/09.
//  Copyright 2009 Tatem Games. All rights reserved.
//

#include "TatEngineCoreConfig.h"

#if defined(TE_PLATFORM_WIN)

#include "teEngineController.h"
#include "tePlatform.h"
#include "teWinDevice.h"
#include "teString.h"
#include "teApplicationManager.h"
#include <iostream>
#include <conio.h>

#include "teLogManager.h"
#include "teFpsCounter.h"

#ifdef TE_MODULE_RENDER
#ifdef TE_RENDER_GL
#include "teRenderGL.h"
#include "wglext.h"
#endif
#endif

using namespace te;

class CWinFileSystem : public te::core::IFileSystem
{
public:

	CWinFileSystem()
	{
		pool.Reserve(1024);
		concate.SetBuffer(pool.Allocate(pool.GetSize()));

		wchar_t binPathUTF16[1024];
		u32 size = GetModuleFileName(NULL, binPathUTF16, 1024);

		for(s32 i = (s32)size; i >= 0; --i)
			if(binPathUTF16[i] == '\\')
			{
				binPathUTF16[i + 1] = '\0';
				break;
			}

		WideCharToMultiByte(CP_UTF8, 0, binPathUTF16, -1, binPathUTF8, 1024, NULL, NULL);
	}

	//! Get Path
	teString GetPath(core::EFilePathType Type)
	{

		switch(Type)
		{
		case core::FPT_DEFAULT:
		case core::FPT_BIN:
		default:
			return concate.Add("%s", binPathUTF8).BakeToString();
		case core::FPT_RESOURCES:
			return concate.Add("%s../Media/", binPathUTF8).BakeToString();
		case core::FPT_RESOURCES_UNPACKED:
			return concate.Add("%s../User/Unpacked/", binPathUTF8).BakeToString();
		case core::FPT_USER_RESOURCES:
			return concate.Add("%s../User/", binPathUTF8).BakeToString();
		case core::FPT_CONTENT:
			return concate.Add("%s../Content/", binPathUTF8).BakeToString();
		}

		return "/";
	}

	teStringPool pool;
	teStringConcate concate;
	c8 binPathUTF8[1024];
};

#define TE_LAUNCHER_CONSOLE

#ifdef TE_LAUNCHER_CONSOLE
int CrtReportHook(int nRptType, char *szMsg, int *retVal)
{
	printf("%s",szMsg);
	return 0;
}

int main(int argc, char * argv[])
{
#ifdef TE_COMPILER_MSVC
	_CrtSetReportHook (CrtReportHook);
	_CrtMemState s1, s2, s3;
	_CrtMemCheckpoint(&s1);
#endif

#else

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

#endif

	core::InitEngine();

	core::IFileSystem * FileSystem = NULL;
	TE_NEW(FileSystem, CWinFileSystem);
	core::GetPlatform()->SetFileSystem(FileSystem);
	FileSystem->Drop();

	TE_NEW_S(game::teWinDevice(teVector2di(TE_LAUNCHER_SCREEN_WIDTH, TE_LAUNCHER_SCREEN_HEIGHT)))

#ifdef TE_MODULE_RENDER
#ifdef TE_RENDER_GL
		//#ifndef TE_LAUNCHER_SCREEN_VSYNC
	PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
	wglSwapIntervalEXT(true);
//#endif
#endif
#endif

	TE_NEW_S(app::teApplicationManager(false))

#ifdef TE_MODULE_RENDER
	app::GetApplicationManager()->OnResize(teVector2di(TE_LAUNCHER_SCREEN_WIDTH, TE_LAUNCHER_SCREEN_HEIGHT));
#endif

	app::GetApplicationManager()->InitApplication();

	f32 time = TE_TIME_32;

	while(game::GetWinDevice()->GetRun() && app::GetApplicationManager()->IsRun())
	//while(app::GetApplicationManager()->IsRun())
	{
		game::GetWinDevice()->Poll();

		//if(game::GetWinDevice()->GetActive())
		{
			#ifdef TE_MODULE_RENDER
			app::GetApplicationManager()->OnResize(game::GetWinDevice()->GetSize());
			#endif
			app::GetApplicationManager()->OnTick();
			game::GetWinDevice()->PresentBuffer();
		}

		//f32 time2 = TE_TIME_32;
		//f32 dt = time2 - time;
		//time = time2;

		//if(dt < 1000.0f / 60.0f)
		//	Sleep(1000.0f / 60.0f - dt);
	}

	app::GetApplicationManager()->Drop();

	TE_DELETE(game::GetWinDevice())

	core::CloseEngine();

#ifdef TE_LAUNCHER_CONSOLE
#ifdef TE_COMPILER_MSVC

	_CrtMemCheckpoint(&s2);
	if(_CrtMemDifference(&s3, &s1, &s2))
	{
		_CrtMemDumpStatistics(&s3);
		_CrtDumpMemoryLeaks();
	}
#endif

	printf("Press any key to continue\n");
	_getch();
#endif

	return 0;
}

#endif
