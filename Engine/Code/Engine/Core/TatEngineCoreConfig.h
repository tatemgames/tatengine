/*
 *  TatEngineCoreConfig.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 11/23/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TATENGINECORECONFIG_H
#define TE_TATENGINECORECONFIG_H

// -------------------------------------------------------------- Core
#define TE_VERSION "2.0"

// -------------------------------------------------------------- Platform

//#define TE_PLATFORM_IPHONE
//#define TE_PLATFORM_LINUX_MOBILE
//#define TE_PLATFORM_ANDROID

//#define TE_PLATFORM_MAC
//#define TE_PLATFORM_WIN
//#define TE_PLATFORM_LINUX

#ifdef TE_PLATFORM_WIN
	#ifdef __CYGWIN32__
		#define TE_COMPILER_MINGW
	#else
		#define TE_COMPILER_MSVC
	#endif
#elif TE_PLATFORM_IPHONE
	#define TE_COMPILER_CLANG
#elif TE_PLATFORM_ANDROID
	#define TE_COMPILER_GCC
#else
	#define TE_COMPILER_GENERIC
#endif

// -------------------------------------------------------------- Compiler specific

//#define TE_NATIVE_HALF_FLOAT

#ifdef TE_COMPILER_MSVC
	#define TE_INLINE inline
	#define TE_FORCE_INLINE __forceinline
	#pragma warning(disable : 4200) // disable warning for zero-size array declaration
	#pragma warning(disable : 4996) // disable warning for secure version of crt functions
#else
	#define TE_INLINE inline
	#define TE_FORCE_INLINE inline
#endif

#define TE_FUNC static TE_INLINE
#define TE_FUNC_FI static TE_FORCE_INLINE

// -------------------------------------------------------------- Debug
#define TE_DEBUG

// -------------------------------------------------------------- Memory
#if defined(TE_DEBUG) && !defined(TE_PLATFORM_ANDROID)
	#define TE_MEMORY_MANAGEMENT
#endif

//#define TE_MEMORY_USE_NEDMALLOC

// -------------------------------------------------------------- Fps Counter
#define TE_FPS_COUNTER_FILTER_SIZE 32

// -------------------------------------------------------------- Render

#if defined(TE_PLATFORM_IPHONE) || defined(TE_PLATFORM_LINUX_MOBILE) || defined(TE_PLATFORM_ANDROID)
	#define TE_OPENGL_ES_11
	#define TE_OPENGL_ES_20
	#define TE_RENDER_GL
	#define TE_RENDER_GL_SHADERS
#else
	#define TE_OPENGL_15
	#define TE_OPENGL_21
	#define TE_RENDER_GL
	#define TE_RENDER_GL_SHADERS
#endif

#ifdef TE_PLATFORM_IPHONE
	#define TE_RENDER_PVRTC 1
#endif

//#define TE_RENDER_FORCE_TEX_FILTERED

// -------------------------------------------------------------- Input
#define TE_INPUT_TOUCHES_MAX 12

#ifndef TE_PLATFORM_IPHONE
	#define TE_INPUT_SUPPORT_KEYBOARD
#endif

// -------------------------------------------------------------- Fonts

//#define TE_CONTENT_FIX_FONTS

// -------------------------------------------------------------- Sound

#define TE_SOUND_FMOD_MEMORY_POOL_SIZE 4 * 1024 * 1024

// -------------------------------------------------------------- Bakery

#define TE_CTTI

//#define TE_CONTENT_STRINGS_ENCR
//#define TE_CONTENT_STRINGS_ENCR_KEY "TatEngine" // please redefine it in your application

// -------------------------------------------------------------- Launcher

#if defined(TE_PLATFORM_WIN)
	#define TE_LAUNCHER_SCREEN_WIDTH 1024
	#define TE_LAUNCHER_SCREEN_HEIGHT 768
#elif defined(TE_PLATFORM_MAC)
	#define TE_LAUNCHER_SCREEN_WIDTH 1024
	#define TE_LAUNCHER_SCREEN_HEIGHT 768
#endif

#define TE_LAUNCHER_SCREEN_VSYNC

#define TE_LAUNCHER_CONSOLE // enable windows console in win launcher

#define TE_LAUNCHER_APPLICATION_TITLE "TatEngine"

//#define TE_LAUNCHER_CUSTOM_MAIN // define custom main for launchers

// enable scale factor 2 when resolution equal 640
#define TE_LAUNCHER_IOS_SUPPORT_IPHONE_RETINA

// enable scale factor 2 when resolution equal 2048
#define TE_LAUNCHER_IOS_SUPPORT_IPAD_RETINA

#define TE_LAUNCHER_IOS_START_ORIENTATION UIInterfaceOrientationMaskLandscape

// -------------------------------------------------------------- Application Config

#define TE_APP_CONFIG

#ifdef TE_APP_CONFIG
	#include "TatAppConfig.h"
#endif

// -------------------------------------------------------------- Network Bugfix

#if defined(TE_MODULE_NETWORK) && defined(TE_PLATFORM_WIN) // there some problem with sockets on windows, stupid platform sdk require include winsock2.h before windows.h
#include <winsock2.h>
#include <windows.h>
#endif

#endif
