/*
 *  teWinDevice.cpp
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 12/13/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#include "teWinDevice.h"

#ifdef TE_PLATFORM_WIN

#include "teLogManager.h"

#ifdef TE_MODULE_RENDER
#ifdef TE_RENDER_GL
#include "wglext.h"
#endif
#endif

#ifdef TE_MODULE_INPUT
#include "teInputManager.h"
#endif

namespace te
{
	namespace game
	{
		//! Current Mouse Position
		teVector2di CurrentMousePosition;

		//! Current Size
		teVector2di CurrentSize;

		//! Current Mouse Down Init Flag
		u1 CurrentMouseDownInitFlag;

		//! Current Mouse Down Count
		u8 CurrentMouseDownInitCount;

		//! Win32 Callback function
		LRESULT CALLBACK WndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
		{
			#ifndef WM_MOUSEWHEEL
				#define WM_MOUSEWHEEL 0x020A
			#endif
			#ifndef WHEEL_DELTA
				#define WHEEL_DELTA 120
			#endif

			#ifndef WM_XBUTTONDOWN
				#define WM_XBUTTONDOWN 0x020B
			#endif
			#ifndef WM_XBUTTONUP
				#define WM_XBUTTONUP 0x020C
			#endif
			#ifndef WM_XBUTTONDBLCLK
				#define WM_XBUTTONDBLCLK 0x020D
			#endif

			#define MOUSE_XBUTTON1 0x0001
			#define MOUSE_XBUTTON2 0x0002

			switch (uMsg)
			{
			case WM_PAINT:
				{
					PAINTSTRUCT ps;
					BeginPaint(hWnd, &ps);
					EndPaint(hWnd, &ps);
				}
				return 0;

			case WM_ERASEBKGND:
				{
					return 0;
				}

			case WM_ACTIVATE:
				{
					if(wParam)
						GetWinDevice()->SetActive(true);
					else
						GetWinDevice()->SetActive(false);

					return 0;
				}
			case WM_SYSCOMMAND:
				{
					switch (wParam)
					{
						case SC_SCREENSAVE:
						case SC_MONITORPOWER:
						return 0;
					}
					break;
				}
			case WM_CLOSE:
				{
					GetWinDevice()->SetRun(false);
					PostQuitMessage(0);
					return 0;
				}
			case WM_SETFOCUS:
				{
					GetWinDevice()->SetActive(true);
					return 0;
				}
			case WM_DESTROY:
			case WM_KILLFOCUS:
				{
					GetWinDevice()->SetActive(false);
					return 0;
				}
			case WM_KEYDOWN:
			case WM_KEYUP:
				{
					#ifdef TE_MODULE_INPUT
					#ifdef TE_INPUT_SUPPORT_KEYBOARD
					input::GetInputManager()->SetKeyDown((input::EKeyCode)wParam, (WM_KEYDOWN == uMsg));
					#endif
					#endif
					return 0;
				}
			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
			case WM_RBUTTONDOWN:
			case WM_RBUTTONUP:
			case WM_MBUTTONDOWN:
			case WM_MBUTTONUP:
				{
					#ifdef TE_MODULE_INPUT

					input::teTouchEvent * Array = input::GetInputManager()->LockTouch();

					u1 ResetCounter = false;

					switch(uMsg)
					{
					case WM_LBUTTONDOWN:
						CurrentMouseDownInitCount = 1;
						break;
					case WM_RBUTTONDOWN:
						CurrentMouseDownInitCount = 2;
						break;
					case WM_MBUTTONDOWN:
						CurrentMouseDownInitCount = 3;
						break;
					default:
						ResetCounter = true;
						break;
					}

					for(u8 i = 0; i < CurrentMouseDownInitCount; ++i)
					{
						Array[i].Position = CurrentMousePosition;
						Array[i].TouchIdentifier = (void*)(i + 1);
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

					if(uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP)
						input::GetInputManager()->SetKeyDown(input::KC_MOUSE_LEFT, (WM_LBUTTONDOWN == uMsg));
					else if(uMsg == WM_RBUTTONDOWN || uMsg == WM_RBUTTONUP)
						input::GetInputManager()->SetKeyDown(input::KC_MOUSE_RIGHT, (WM_RBUTTONDOWN == uMsg));
					else if(uMsg == WM_MBUTTONDOWN || uMsg == WM_MBUTTONUP)
						input::GetInputManager()->SetKeyDown(input::KC_MOUSE_MIDDLE, (WM_MBUTTONDOWN == uMsg));

					#endif

					return 0;
				}
			case WM_XBUTTONDOWN:
			case WM_XBUTTONUP:
				{
					//u32 ButtonNum = HIWORD(wParam);
					//input::KeyCode Key;
					//switch(ButtonNum)
					//{
					//case MOUSE_XBUTTON1:
					//	{
					//		Key = input::KC_XBUTTON1;
					//		break;
					//	}
					//case MOUSE_XBUTTON2:
					//	{
					//		Key = input::KC_XBUTTON2;
					//		break;
					//	}
					//default:
					//	{
					//		JELOG_E("Unknown button")
					//		Key = input::KC_XBUTTON1;
					//		break;
					//	}
					//}
					//(static_cast<input::CKeyDriver*>(CurrentWindow->GetKeyDriver()))->SetDown(Key,(WM_XBUTTONDOWN == uMsg));
					return true; // check msdn for this
				}
			case WM_MOUSEMOVE:
				{
					#ifdef TE_MODULE_INPUT

					CurrentMousePosition.SetXY(LOWORD(lParam), HIWORD(lParam));

					if(!CurrentMouseDownInitFlag)
					{
						input::teTouchEvent * Array = input::GetInputManager()->LockTouch();

						for(u8 i = 0; i < CurrentMouseDownInitCount; ++i)
						{
							Array[i].Position = CurrentMousePosition;
							Array[i].TouchIdentifier = (void*)(i + 1);
						}

						te::input::GetInputManager()->UnlockTouch(input::TET_MOVE, CurrentMouseDownInitCount);
					}

					#endif

					return 0;
				}
			case WM_MOUSEWHEEL:
				{
					//(static_cast<input::CMouseDriver*>(CurrentWindow->GetMouseDriver()))->SetMouseWheelPosition( CurrentWindow->GetMouseDriver()->GetMouseWheelPosition() + ((short)HIWORD(wParam)) / (s32)WHEEL_DELTA );
					return 0;
				}
			case WM_SIZE:
				{
					CurrentSize.SetXY(LOWORD(lParam), HIWORD(lParam));
					GetWinDevice()->SetSize(CurrentSize);
					return 0;
				}
			};

			return DefWindowProc(hWnd,uMsg,wParam,lParam);
		}

		//! Current Win Device
		teWinDevice * CurrentWinDevice = NULL;

		//! Constructor
		teWinDevice::teWinDevice(const teVector2di & Size)
			:CurrentRun(true), CurrentActive(true)
		{
			CurrentWinDevice = this;

			CurrentMouseDownInitFlag = true;
			CurrentMouseDownInitCount = 0;

			RECT WindowRect;
			WindowRect.left = 0;
			WindowRect.right = (long)Size.x;
			WindowRect.top = (long)0;
			WindowRect.bottom = (long)Size.y;

			CurrentSize = Size;

			AppInstance = GetModuleHandle(NULL);

			ZeroMemory(&DefaultMode, sizeof(DEVMODE));
			DefaultMode.dmSize = sizeof(DEVMODE);
			EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &DefaultMode);

			WNDCLASS wc;
			wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
			wc.lpfnWndProc = (WNDPROC)WndProc;
			wc.cbClsExtra = 0;
			wc.cbWndExtra = 0;
			wc.hInstance = AppInstance;
			wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
			wc.hCursor = LoadCursor(NULL, IDC_ARROW);
			wc.hbrBackground = NULL;
			wc.lpszMenuName = NULL;
			wc.lpszClassName = L"TatEngine_Win";

			if (!RegisterClass(&wc))
			{
				TE_LOG_ERR("Failed To Register Window Class")
				return;
			}

			DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
			DWORD dwStyle = WS_OVERLAPPEDWINDOW;

			AdjustWindowRectEx(&WindowRect,dwStyle,false,dwExStyle);

			u1 Result = false;

			const char * Title = TE_LAUNCHER_APPLICATION_TITLE;

			if(!(WindowHandle = CreateWindowExA(dwExStyle, "TatEngine_Win", Title, dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
				0, 0, WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top, NULL, NULL, AppInstance, NULL)))
				TE_LOG_ERR("Window Creation Error")

			if (!(DeviceContext = GetDC(WindowHandle)))
				TE_LOG_ERR("Device Context Creation Error")

			ShowWindow(WindowHandle,SW_SHOW);
			SetForegroundWindow(WindowHandle);
			SetFocus(WindowHandle);

			PIXELFORMATDESCRIPTOR pfd =
				{
					sizeof(PIXELFORMATDESCRIPTOR),
					1,
					PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_SWAP_EXCHANGE,
					PFD_TYPE_RGBA,
					32,
					0, 0, 0, 0, 0, 0,
					0,
					0,
					0,
					0, 0, 0, 0,
					24,
					8,
					0,
					PFD_MAIN_PLANE,
					0,
					0, 0, 0
				};

			#ifdef TE_MODULE_RENDER
			#ifdef TE_RENDER_GL

			GLuint PixelFormat;
			
			if (!(PixelFormat = ChoosePixelFormat(DeviceContext, &pfd)))
				TE_LOG_ERR("Cant Choose OpenGL Render Context Pixel Format")

			if(!SetPixelFormat(DeviceContext, PixelFormat, &pfd))
				TE_LOG_ERR("Cant Set OpenGL Render Context Pixel Format")

			if (!(RenderContext = wglCreateContext(DeviceContext)))
				TE_LOG_ERR("Cant Create OpenGL Render Context")

			if(!wglMakeCurrent(DeviceContext, RenderContext))
				TE_LOG_ERR("Cant Activate OpenGL Render Context")

			//PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
			//wglSwapIntervalEXT(false);

			#endif
			#endif
		}

		//! Destructor
		teWinDevice::~teWinDevice()
		{
			#ifdef TE_MODULE_RENDER
			#ifdef TE_RENDER_GL

			if(!wglMakeCurrent(NULL,NULL))
				TE_LOG_ERR("Cant Deactivate OpenGL Render Context")

			if(!wglDeleteContext(RenderContext))
				TE_LOG_ERR("Deleting OpenGL Render Context Failed")

			#endif
			#endif

			if (DeviceContext && !ReleaseDC(WindowHandle, DeviceContext))
				TE_LOG_ERR("Error at release device context")

			DeviceContext = NULL;

			if (WindowHandle)
			{
				SetWindowLong(WindowHandle, DWLP_USER, (LONG)NULL);
				if(!DestroyWindow(WindowHandle))
					TE_LOG_ERR("Error at release window handle")
			}

			WindowHandle = NULL;

			if (!UnregisterClass(L"TatEngine_Win", AppInstance))
				TE_LOG_ERR("Error at unregister class")

			AppInstance = NULL;

			CurrentWinDevice = NULL;
		}

		//! Poll
		void teWinDevice::Poll()
		{
			MSG msg;
			while(PeekMessage( &msg,0,0,0,PM_REMOVE))
			{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
		}

		//! Present Buffer
		void teWinDevice::PresentBuffer()
		{
			SwapBuffers(DeviceContext);
		}

		//! Set Run
		void teWinDevice::SetRun(u1 Run)
		{
			CurrentRun = Run;
		}

		//! Get Run
		u1 teWinDevice::GetRun()
		{
			return CurrentRun;
		}

		//! Set Active
		void teWinDevice::SetActive(u1 Active)
		{
			CurrentActive = Active;
		}

		//! Get Active
		u1 teWinDevice::GetActive()
		{
			return CurrentActive;
		}

		//! Set Size
		void teWinDevice::SetSize(const teVector2di & Size)
		{
			CurrentSize = Size;
		}

		//! Get Size
		const teVector2di & teWinDevice::GetSize()
		{
			return CurrentSize;
		}

		//! Get Device
		teWinDevice * GetWinDevice()
		{
			return CurrentWinDevice;
		}
	}
}

#endif
