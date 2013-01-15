//
//  teSDLDevice.cpp
//  TatEngine
//
//  Created by Dmitrii Ivanov on 07/7/10.
//  Copyright 2010 Tatem Games. All rights reserved.
//

#include "teSDLDevice.h"

#include "SDL/SDL.h"

#include "teEngineController.h"
#include "tePlatform.h"
#include "teApplicationManager.h"
#include "teRenderGL.h"
#include "teInputManager.h"
#include "teLogManager.h"
#include "teFpsCounter.h"

#include <map>

#ifdef TE_PLATFORM_MAC
	#import <Cocoa/Cocoa.h>
#endif

using namespace te;

SDL_Surface * SDLScreen = NULL;

std::map<int, input::EKeyCode> KeyMap;

teVector2di CurrentMousePosition;
u1 CurrentMouseDownInitFlag = true;
u8 CurrentMouseDownInitCount = 0;

//! Mac OS X File System
class CMacOSXFileSystem : public te::core::IFileSystem
{
public:
	//! Get Path
	teString GetPath(core::EFilePathType Type)
	{
		char * test = NULL;
		const char * sa = NULL;

		switch(Type)
		{
		#if defined(TE_PLATFORM_WIN) || defined(TE_PLATFORM_MAC)
		case core::FPT_BIN:
			sa = [[[[[NSBundle mainBundle] bundlePath] stringByDeletingLastPathComponent] stringByDeletingLastPathComponent] UTF8String];
			break;
		#endif
		case core::FPT_RESOURCES_UNPACKED:
			sa = [[[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"Unpacked"] UTF8String];
			break;
		case core::FPT_DEFAULT:
		case core::FPT_RESOURCES:
		case core::FPT_USER_RESOURCES:
		case core::FPT_CONTENT:
		default:
			sa = [[[NSBundle mainBundle] resourcePath] UTF8String];
			break;
		}
		
		test = (char*)malloc(strlen(sa) + 2);
		
		memcpy(test, sa, strlen(sa));
		test[strlen(sa)] = '/';
		test[strlen(sa) + 1] = '\0';
		
		NSAutoreleasePool * autoreleasePool = [[NSAutoreleasePool alloc] init];
		[autoreleasePool release];
		return te::teString(test); // TODO FIX THIS SHIT !!!
	}
};

//! Fill Key Map
void FillKeyMap()
{
	KeyMap[SDLK_BACKSPACE] = input::KC_BACK;
	KeyMap[SDLK_TAB] = input::KC_TAB;
	KeyMap[SDLK_CLEAR] = input::KC_CLEAR;
	KeyMap[SDLK_RETURN] = input::KC_RETURN;
	KeyMap[SDLK_PAUSE] = input::KC_PAUSE;
	KeyMap[SDLK_CAPSLOCK] = input::KC_CAPITAL;
	KeyMap[SDLK_ESCAPE] = input::KC_ESCAPE;
	KeyMap[SDLK_SPACE] = input::KC_SPACE;
	KeyMap[SDLK_PAGEUP] = input::KC_PRIOR;
	KeyMap[SDLK_PAGEDOWN] = input::KC_NEXT;
	KeyMap[SDLK_END] = input::KC_END;
	KeyMap[SDLK_HOME] = input::KC_HOME;
	KeyMap[SDLK_LEFT] = input::KC_LEFT;
	KeyMap[SDLK_UP] = input::KC_UP;
	KeyMap[SDLK_RIGHT] = input::KC_RIGHT;
	KeyMap[SDLK_DOWN] = input::KC_DOWN;
	KeyMap[SDLK_PRINT] = input::KC_PRINT;
	KeyMap[SDLK_PRINT] = input::KC_SNAPSHOT;
	KeyMap[SDLK_INSERT] = input::KC_INSERT;
	KeyMap[SDLK_DELETE] = input::KC_DELETE;
	KeyMap[SDLK_HELP] = input::KC_HELP;
	KeyMap[SDLK_0] = input::KC_0;
	KeyMap[SDLK_1] = input::KC_1;
	KeyMap[SDLK_2] = input::KC_2;
	KeyMap[SDLK_3] = input::KC_3;
	KeyMap[SDLK_4] = input::KC_4;
	KeyMap[SDLK_5] = input::KC_5;
	KeyMap[SDLK_6] = input::KC_6;
	KeyMap[SDLK_7] = input::KC_7;
	KeyMap[SDLK_8] = input::KC_8;
	KeyMap[SDLK_9] = input::KC_9;
	KeyMap[SDLK_a] = input::KC_A;
	KeyMap[SDLK_b] = input::KC_B;
	KeyMap[SDLK_c] = input::KC_C;
	KeyMap[SDLK_d] = input::KC_D;
	KeyMap[SDLK_e] = input::KC_E;
	KeyMap[SDLK_f] = input::KC_F;
	KeyMap[SDLK_g] = input::KC_G;
	KeyMap[SDLK_h] = input::KC_H;
	KeyMap[SDLK_i] = input::KC_I;
	KeyMap[SDLK_j] = input::KC_J;
	KeyMap[SDLK_k] = input::KC_K;
	KeyMap[SDLK_l] = input::KC_L;
	KeyMap[SDLK_m] = input::KC_M;
	KeyMap[SDLK_n] = input::KC_N;
	KeyMap[SDLK_o] = input::KC_O;
	KeyMap[SDLK_p] = input::KC_P;
	KeyMap[SDLK_q] = input::KC_Q;
	KeyMap[SDLK_r] = input::KC_R;
	KeyMap[SDLK_s] = input::KC_S;
	KeyMap[SDLK_t] = input::KC_T;
	KeyMap[SDLK_u] = input::KC_U;
	KeyMap[SDLK_v] = input::KC_V;
	KeyMap[SDLK_w] = input::KC_W;
	KeyMap[SDLK_x] = input::KC_X;
	KeyMap[SDLK_y] = input::KC_Y;
	KeyMap[SDLK_z] = input::KC_Z;
	KeyMap[SDLK_LSUPER] = input::KC_LWIN;
	KeyMap[SDLK_RSUPER] = input::KC_RWIN;
	KeyMap[SDLK_KP0] = input::KC_NUMPAD0;
	KeyMap[SDLK_KP1] = input::KC_NUMPAD1;
	KeyMap[SDLK_KP2] = input::KC_NUMPAD2;
	KeyMap[SDLK_KP3] = input::KC_NUMPAD3;
	KeyMap[SDLK_KP4] = input::KC_NUMPAD4;
	KeyMap[SDLK_KP5] = input::KC_NUMPAD5;
	KeyMap[SDLK_KP6] = input::KC_NUMPAD6;
	KeyMap[SDLK_KP7] = input::KC_NUMPAD7;
	KeyMap[SDLK_KP8] = input::KC_NUMPAD8;
	KeyMap[SDLK_KP9] = input::KC_NUMPAD9;
	KeyMap[SDLK_KP_MULTIPLY] = input::KC_MULTIPLY;
	KeyMap[SDLK_KP_PLUS] = input::KC_ADD;
	KeyMap[SDLK_KP_MINUS] = input::KC_SUBTRACT;
	KeyMap[SDLK_KP_PERIOD] = input::KC_DECIMAL;
	KeyMap[SDLK_KP_DIVIDE] = input::KC_DIVIDE;
	KeyMap[SDLK_F1] =  input::KC_F1;
	KeyMap[SDLK_F2] =  input::KC_F2;
	KeyMap[SDLK_F3] =  input::KC_F3;
	KeyMap[SDLK_F4] =  input::KC_F4;
	KeyMap[SDLK_F5] =  input::KC_F5;
	KeyMap[SDLK_F6] =  input::KC_F6;
	KeyMap[SDLK_F7] =  input::KC_F7;
	KeyMap[SDLK_F8] =  input::KC_F8;
	KeyMap[SDLK_F9] =  input::KC_F9;
	KeyMap[SDLK_F10] = input::KC_F10;
	KeyMap[SDLK_F11] = input::KC_F11;
	KeyMap[SDLK_F12] = input::KC_F12;
	KeyMap[SDLK_F13] = input::KC_F13;
	KeyMap[SDLK_F14] = input::KC_F14;
	KeyMap[SDLK_F15] = input::KC_F15;
	KeyMap[SDLK_NUMLOCK] = input::KC_NUMLOCK;
	KeyMap[SDLK_SCROLLOCK] = input::KC_SCROLL;
	KeyMap[SDLK_LSHIFT] = input::KC_LSHIFT;
	KeyMap[SDLK_RSHIFT] = input::KC_RSHIFT;
	KeyMap[SDLK_LCTRL] =  input::KC_LCONTROL;
	KeyMap[SDLK_RCTRL] =  input::KC_RCONTROL;
	KeyMap[SDLK_LALT] =  input::KC_LMENU;
	KeyMap[SDLK_RALT] =  input::KC_RMENU;
	KeyMap[SDLK_PLUS] =   input::KC_PLUS;
	KeyMap[SDLK_COMMA] =  input::KC_COMMA;
	KeyMap[SDLK_MINUS] =  input::KC_MINUS;
	KeyMap[SDLK_PERIOD] = input::KC_PERIOD;
}

//! Main
int main(int argc, char *argv[])
{
	FillKeyMap();

	SDL_Init(SDL_INIT_VIDEO);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	#ifdef TE_LAUNCHER_SCREEN_VSYNC
		SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
	#endif
	
	SDLScreen = SDL_SetVideoMode(TE_LAUNCHER_SCREEN_WIDTH, TE_LAUNCHER_SCREEN_HEIGHT, 0, SDL_OPENGL);
	
	core::InitEngine();

	core::IFileSystem * FileSystem = NULL;
	TE_NEW(FileSystem, CMacOSXFileSystem);
	core::GetPlatform()->SetFileSystem(FileSystem);
	FileSystem->Drop();

	TE_NEW_S(app::teApplicationManager(false))
	app::GetApplicationManager()->OnResize(teVector2di(TE_LAUNCHER_SCREEN_WIDTH, TE_LAUNCHER_SCREEN_HEIGHT));
	app::GetApplicationManager()->InitApplication();

	SDL_Event event;
		
	while(app::GetApplicationManager()->IsRun())
	{
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
			case SDL_MOUSEMOTION:
				{
					CurrentMousePosition.SetXY(event.motion.x, event.motion.y);

					if(!CurrentMouseDownInitFlag)
					{
						input::teTouchEvent * Array = input::GetInputManager()->LockTouch();

						for(u8 i = 0; i < CurrentMouseDownInitCount; ++i)
						{
							Array[i].Position = CurrentMousePosition;
							Array[i].TouchIdentifier = (void*)i;
						}

						te::input::GetInputManager()->UnlockTouch(input::TET_MOVE, CurrentMouseDownInitCount);
					}

					break;
				}
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				{
					input::teTouchEvent * Array = input::GetInputManager()->LockTouch();

					u1 ResetCounter = false;
					input::EKeyCode MouseKeyCode = input::KC_NULL;

					if(event.type == SDL_MOUSEBUTTONDOWN)
					{
						switch(event.button.button)
						{
						case 1:
							CurrentMouseDownInitCount = 1;
							MouseKeyCode = input::KC_MOUSE_LEFT;
							break;
						case 3:
							CurrentMouseDownInitCount = 2;
							MouseKeyCode = input::KC_MOUSE_RIGHT;
							break;
						case 2:
							CurrentMouseDownInitCount = 3;
							MouseKeyCode = input::KC_MOUSE_MIDDLE;
							break;
						default:
							ResetCounter = true;
							break;
						}
					}
					else
					{
						ResetCounter = true;
					}

					for(u8 i = 0; i < CurrentMouseDownInitCount; ++i)
					{
						Array[i].Position = CurrentMousePosition;
						Array[i].TouchIdentifier = (void*)i;
					}

					input::ETouchEventType Type;

					if(!ResetCounter)
					{
						if(CurrentMouseDownInitFlag)
						{
							Type = input::TET_BEGIN;
							CurrentMouseDownInitFlag = false;
						}
						else
							Type = input::TET_MOVE;
					}
					else
					{
						Type = input::TET_END;
						CurrentMouseDownInitFlag = true;
					}

					te::input::GetInputManager()->UnlockTouch(Type, CurrentMouseDownInitCount);

					input::GetInputManager()->SetKeyDown(MouseKeyCode, SDL_MOUSEBUTTONDOWN == event.type);

					break;
				}
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				{
					input::GetInputManager()->SetKeyDown(KeyMap[event.key.keysym.sym], SDL_KEYDOWN == event.type);
					break;
				}
			case SDL_QUIT:
				app::GetApplicationManager()->GetApplication()->SetRun(false);
				break;
			default:
				break;
			}
		}

		app::GetApplicationManager()->OnResize(teVector2di(SDLScreen->w, SDLScreen->h));
		app::GetApplicationManager()->OnTick();

		SDL_GL_SwapBuffers();
	}
	
	app::GetApplicationManager()->Drop();
	core::CloseEngine();
	
	SDL_Quit();
	
	return 0;
}
