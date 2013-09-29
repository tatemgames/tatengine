/*
 *  tePlatform.cpp
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 12/8/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#include "tePlatform.h"

#ifdef TE_PLATFORM_IPHONE
	#include "tePlatform_iOS.h"
#endif

namespace te
{
	namespace core
	{
		tePlatform * currentPlatform = NULL;

		tePlatform::tePlatform()
			:deviceOrientation(DO_NORMAL)
		{
			currentPlatform = this;

			fileSystem = NULL;
			SetFileSystem(NULL);

			SetDeviceOrientationMask(DO_NORMAL | DO_DOWN | DO_LEFT | DO_RIGHT);

			devicePlatform = NULL;
		}

		tePlatform::~tePlatform()
		{
			TE_SAFE_DROP(fileSystem);
			TE_SAFE_DROP(devicePlatform);

			currentPlatform = NULL;
		}

		void tePlatform::SetFileSystem(IFileSystem * setFileSystem)
		{
			TE_SAFE_DROP(fileSystem);

			fileSystem = setFileSystem;

			TE_SAFE_GRAB(fileSystem);

			if(!fileSystem)
				TE_NEW(fileSystem, IFileSystem); // default file system

		}

		IFileSystem * tePlatform::GetFileSystem()
		{
			TE_ASSERT(fileSystem);
			return fileSystem;
		}

		void tePlatform::SetDeviceOrientation(EDeviceOrientation setDeviceOrientation)
		{
			if(deviceOrientationMask & ((u32)setDeviceOrientation))
				if(deviceOrientation != setDeviceOrientation)
					deviceOrientation = setDeviceOrientation;
		}

		EDeviceOrientation tePlatform::GetDeviceOrientation() const
		{
			return deviceOrientation;
		}

		void tePlatform::SetDeviceOrientationMask(u32 setDeviceOrientationMask)
		{
			deviceOrientationMask = setDeviceOrientationMask;
		}

		u32 tePlatform::GetDeviceOrientationMask() const
		{
			return deviceOrientationMask;
		}

		void tePlatform::SetCurrentDevicePlatform(teReferenceCounter * setPlatform)
		{
			TE_SAFE_DROP(devicePlatform);
			devicePlatform = setPlatform;
			TE_SAFE_GRAB(devicePlatform);
		}

		EDeviceType tePlatform::GetDeviceType()
		{
			#ifdef TE_PLATFORM_IPHONE
				return reinterpret_cast<tePlatform_iOS*>(devicePlatform)->DefinePlatform();
			#elif TE_PLATFORM_ANDROID
				return DT_ANDROID;
			#elif TE_PLATFORM_MAC
				return DT_MAC;
			#elif TE_PLATFORM_WIN
				return DT_WIN;
			#elif TE_PLATFORM_LINUX_MOBILE
				return DT_UNKNOWN;
			#elif TE_PLATFORM_LINUX
				return DT_UNKNOWN;
			#else
				return DT_UNKNOWN;
			#endif
		}

		teReferenceCounter * tePlatform::GetCurrentDevicePlatform() const
		{
			return devicePlatform;
		}

		const c8 * tePlatform::GetDeviceLocale()
		{
			#ifdef TE_PLATFORM_IPHONE
				return reinterpret_cast<tePlatform_iOS*>(devicePlatform)->GetDeviceLocale();
			#else
				return "en";
			#endif
		}

		tePlatform * GetPlatform()
		{
			return currentPlatform;
		}
	}
}
