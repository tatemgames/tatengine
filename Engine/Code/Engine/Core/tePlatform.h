/*
 *  tePlatform.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 12/8/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEPLATFORM_H
#define TE_TEPLATFORM_H

#include "teTypes.h"
#include "teString.h"
#include "IFileSystem.h"

namespace te
{
	namespace core
	{
		enum EDeviceOrientation
		{
			DO_NORMAL = 1,
			DO_DOWN = 2,
			DO_LEFT = 4,
			DO_RIGHT = 8
		};
		
		enum EDeviceType
		{
			DT_UNKNOWN = 0,

			DT_IOS_UNKNOWN,
			DT_IOS_IPHONE_2G,
			DT_IOS_IPHONE_3G,
			DT_IOS_IPHONE_3GS,
			DT_IOS_IPHONE_4,
			DT_IOS_IPHONE_4S,
			DT_IOS_IPHONE_5,
			DT_IOS_IPOD_1,
			DT_IOS_IPOD_2,
			DT_IOS_IPOD_3,
			DT_IOS_IPOD_4,
			DT_IOS_IPOD_5,
			DT_IOS_IPAD_1,
			DT_IOS_IPAD_2,
			DT_IOS_IPAD_3,
			DT_IOS_IPAD_4,
			DT_IOS_IPAD_MINI_1,

			DT_ANDROID,
			DT_PS_VITA,
			DT_MAC,
			DT_WIN
		};

		class tePlatform
		{
		public:
			tePlatform();
			~tePlatform();

			void SetFileSystem(IFileSystem * setFileSystem);
			IFileSystem * GetFileSystem();

			void SetDeviceOrientation(EDeviceOrientation setDeviceOrientationMask);
			EDeviceOrientation GetDeviceOrientation() const;

			void SetDeviceOrientationMask(u32 setDeviceOrientationMask);
			u32 GetDeviceOrientationMask() const;

			void SetCurrentDevicePlatform(teReferenceCounter * setPlatform);
			teReferenceCounter * GetCurrentDevicePlatform() const;

			EDeviceType GetDeviceType();

			TE_INLINE f32 GetDevicePPI()
			{
				switch(GetDeviceType())
				{
				case DT_IOS_IPHONE_2G:
				case DT_IOS_IPHONE_3G:
				case DT_IOS_IPHONE_3GS:
				case DT_IOS_IPOD_1:
				case DT_IOS_IPOD_2:
				case DT_IOS_IPOD_3:
					return 163.0f;

				case DT_IOS_IPHONE_4:
				case DT_IOS_IPHONE_4S:
				case DT_IOS_IPOD_4:
				case DT_IOS_IPHONE_5:
				case DT_IOS_IPOD_5:
					return 326.0f;

				case DT_IOS_IPAD_1:
				case DT_IOS_IPAD_2:
					return 132.0f;

				case DT_IOS_IPAD_3:
				case DT_IOS_IPAD_4:
					return 264.0f;

				case DT_IOS_IPAD_MINI_1:
					return 163.0f;

				case DT_IOS_UNKNOWN:
				case DT_UNKNOWN:
				case DT_ANDROID:
				case DT_PS_VITA:
				case DT_MAC:
				case DT_WIN:
				default:
					return -1.0f;
				}
			}

		protected:
			IFileSystem * fileSystem;
			EDeviceOrientation deviceOrientation;
			u32 deviceOrientationMask;
			teReferenceCounter * devicePlatform;
		};
		
		tePlatform * GetPlatform();
	}
}

#endif
