/*
 *  teInputManager.cpp
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 12/4/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#include "teInputManager.h"
#include "teLogManager.h"

#ifdef TE_INPUT_SUPPORT_DEVICE_ORIENTATION
#include "tePlatform.h"
#include "teRenderGL.h"
#endif

namespace te
{
	namespace input
	{
		teInputManager * CurrentInputManager = NULL;
		
		teInputManager::teInputManager()
			:CurrentCountOfTouches(0), CurrentResolveType(TFRT_POINTERS)
		{
			CurrentInputManager = this;
		}
		
		teInputManager::~teInputManager()
		{
			CurrentInputManager = NULL;
		}

		// ------------------------------------------------------------------------ Accelerometer
		
		void teInputManager::SetAccelerometer(const teVector3df & Rotation, u8 Index)
		{
			CurrentAccelerometer[Index] = Rotation;
		}		
		
		const teVector3df & teInputManager::GetAccelerometer(u8 Index)
		{
			return CurrentAccelerometer[Index];
		}

		// ------------------------------------------------------------------------ Touches

		u8 teInputManager::GetTouchesCount()
		{
			return CurrentCountOfTouches;
		}
		
		const teVector2df & teInputManager::GetTouch(u8 Finger, ETouchDataType Type)
		{
			return CurrentTouches[Finger].GetTouchPosition(Type);
		}

		teVector2df teInputManager::GetVirtualTouch(const teVector2df & Touch, u1 ThisIsDeltaValue)
		{
			teVector2df RealTouch = Touch;

			#ifdef TE_INPUT_SUPPORT_DEVICE_ORIENTATION

			teVector2df InputDeviceNormalSize = video::GetRender()->GetContext()->GetSize(true);

			teVector2di DeviceOrientationKoef(1, 1);

			switch(core::GetPlatform()->GetDeviceOrientation())
			{
			case core::DO_DOWN:
				DeviceOrientationKoef.SetXY(-1, -1);
				break;
			case core::DO_LEFT:
				DeviceOrientationKoef.SetXY(1, -1);
				RealTouch.SetXY(RealTouch.Y, RealTouch.X); // there we invert device coordinate system, same for right orientation
				break;
			case core::DO_RIGHT:
				DeviceOrientationKoef.SetXY(-1, 1);
				RealTouch.SetXY(RealTouch.Y, RealTouch.X);
				break;
			default:
				break;
			}

			if(ThisIsDeltaValue)
				InputDeviceNormalSize.SetXY(0, 0);

			RealTouch.SetXY((DeviceOrientationKoef.X == 1 ? RealTouch.X : InputDeviceNormalSize.X - RealTouch.X),
							(DeviceOrientationKoef.Y == 1 ? RealTouch.Y : InputDeviceNormalSize.Y - RealTouch.Y));

			#endif

			return RealTouch;
		}

		teVector2df teInputManager::GetVirtualTouch(u8 Finger, ETouchDataType Type)
		{
			return GetVirtualTouch(GetTouch(Finger, Type), Type >= TDT_DELTA_FROM_LAST);
		}

		teTouchIdentifier teInputManager::GetTouchIdentifier(u8 Finger)
		{
			return CurrentTouches[Finger].TouchIdentifier;
		}

		u1 teInputManager::GetTouchFinger(teTouchIdentifier Identifier, u8 & Finger)
		{
			for(u8 j = 0; j < CurrentCountOfTouches; ++j)
			{
				if(CurrentTouches[j].TouchIdentifier == Identifier)
				{
					Finger = j;
					return true;
				}
			}

			return false;
		}

		u1 teInputManager::IsValidIdentifier(teTouchIdentifier Identifier)
		{
			for(u8 j = 0; j < CurrentCountOfTouches; ++j)
				if(CurrentTouches[j].TouchIdentifier == Identifier)
					return true;

			return false;
		}

		teTouchEvent * teInputManager::LockTouch()
		{
			return CurrentTouchesUpdateArray;
		}
		
		void teInputManager::UnlockTouch(ETouchEventType EventType, u8 CountOfTouches)
		{
			switch(EventType)
			{
			case TET_BEGIN:
				{
					for(u8 i = 0; i < CountOfTouches; ++i)
					{
						if(CurrentResolveType == TFRT_POINTERS)
						{
							for(u8 j = 0; j < CurrentCountOfTouches; ++j)
							{
								if(CurrentTouches[j].TouchIdentifier == CurrentTouchesUpdateArray[i].TouchIdentifier)
								{
									TE_LOG_ERR("OS try to add finger (touch began event) which was not removed (events touch end and cancelled), try to use other finger resolve type")
									break;
								}
							}
						}
						
						CurrentTouches[CurrentCountOfTouches].TouchIdentifier = CurrentTouchesUpdateArray[i].TouchIdentifier;
						CurrentTouches[CurrentCountOfTouches].StartPosition = CurrentTouchesUpdateArray[i].Position;
						CurrentTouches[CurrentCountOfTouches].CurrentPosition = CurrentTouches[CurrentCountOfTouches].StartPosition;
						CurrentTouches[CurrentCountOfTouches].DeltaFromLastPosition.SetXY(0, 0);
						CurrentTouches[CurrentCountOfTouches].DeltaFromStartPosition.SetXY(0, 0);

						CurrentCountOfTouches++;
					}

					break;
				}
			case TET_MOVE:
				{
					for(u8 i = 0; i < CountOfTouches; ++i)
					{
						u8 Finger = ResolveFinger(i);

						CurrentTouches[Finger].DeltaFromLastPosition = CurrentTouchesUpdateArray[i].Position - CurrentTouches[Finger].CurrentPosition;
						CurrentTouches[Finger].DeltaFromStartPosition = CurrentTouchesUpdateArray[i].Position - CurrentTouches[Finger].StartPosition;
						CurrentTouches[Finger].CurrentPosition = CurrentTouchesUpdateArray[i].Position;
					}

					break;
				}
			case TET_END:
			case TET_CANCELLED:
				{
					for(u8 i = 0; (i < CountOfTouches) && (CurrentCountOfTouches > 0); ++i, --CurrentCountOfTouches)
						for(u8 j = ResolveFinger(i); j < CurrentCountOfTouches - 1; ++j)
							CurrentTouches[j] = CurrentTouches[j + 1];

					break;
				}
			default:
				break;
			}
		}
		
		void teInputManager::SetFingerResolveType(ETouchFingerResolveType ResolveType)
		{
			CurrentResolveType = ResolveType;
		}

		u8 teInputManager::ResolveFinger(u8 Index)
		{
			if(CurrentResolveType == TFRT_POINTERS)
			{
				for(u8 i = 0; i < CurrentCountOfTouches; ++i)
					if(CurrentTouchesUpdateArray[Index].TouchIdentifier == CurrentTouches[i].TouchIdentifier)
						return i;

				TE_LOG_ERR("Cant resolve finger, try to use other resolve type")

				return 0;
			}
			else
			{
				u8 Finger = 0;
				f32 MinDist = (CurrentTouches[0].CurrentPosition - CurrentTouchesUpdateArray[Index].Position).GetDistance();

				for(u8 i = 1; i < CurrentCountOfTouches; ++i)
				{
					f32 Dist = (CurrentTouches[i].CurrentPosition - CurrentTouchesUpdateArray[Index].Position).GetDistance();

					if(Dist <= MinDist)
					{
						MinDist = Dist;
						Finger = i;
					}
				}

				return Finger;
			}
		}
		
		void teInputManager::ForceClearTouches()
		{
			CurrentCountOfTouches = 0;
		}

		// ------------------------------------------------------------------------ Keyboard
		
		#ifdef TE_INPUT_SUPPORT_KEYBOARD

		u1 teInputManager::IsKeyHit(EKeyCode Key)
		{
			u32 Hit = CurrentKeyData[Key].KeyHits;
			
			if(Hit)
				CurrentKeyData[Key].KeyHitsProcessed++;

			return Hit ? true : false;
		}

		u1 teInputManager::IsKeyDown(EKeyCode Key, u1 TrueWhenDownMessageArrive)
		{
			if(TrueWhenDownMessageArrive)
				return CurrentKeyData[Key].KeyDownMessageArrive;
			else
				return CurrentKeyData[Key].KeyDown;
		}

		void teInputManager::FlushKeysHits()
		{
			for(u16 i = 0; i < 256; ++i)
			{
				CurrentKeyData[i].KeyHits -= teMax(CurrentKeyData[i].KeyHits, CurrentKeyData[i].KeyHitsProcessed);
				CurrentKeyData[i].KeyHitsProcessed = 0;
				CurrentKeyData[i].KeyDownMessageArrive = false;
			}
		}

		void teInputManager::SetKeyDown(EKeyCode Key, u1 Down)
		{
			if(Down && (!CurrentKeyData[Key].KeyDown))
				CurrentKeyData[Key].KeyHits++;

			if(Down)
			{
				CurrentKeyData[Key].KeyDownMessageArrive = Down;
			}

			CurrentKeyData[Key].KeyDown = Down;
		}

		#endif

		teInputManager * GetInputManager()
		{
			return CurrentInputManager;
		}
	}
}
