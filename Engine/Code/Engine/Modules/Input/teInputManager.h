/*
 *  teInputManager.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 12/4/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEINPUTMANAGER_H
#define TE_TEINPUTMANAGER_H

#include "teReferenceCounter.h"
#include "teVector2D.h"
#include "teVector3D.h"

namespace te
{
	namespace input
	{
		enum EKeyCode
		{
			KC_NULL             = 0x00,	// Null Key (no key)
			KC_MOUSE_LEFT       = 0x01,	// Left mouse button
			KC_MOUSE_RIGHT      = 0x02,	// Right mouse button
			KC_CANCEL           = 0x03,	// Control-break processing
			KC_MOUSE_MIDDLE     = 0x04,	// Middle mouse button (three-button mouse)
			KC_XBUTTON1         = 0x05,	// X1 mouse button 
			KC_XBUTTON2         = 0x06,	// X2 mouse button 
			KC_BACK             = 0x08,	// BACKSPACE key
			KC_TAB              = 0x09,	// TAB key
			KC_CLEAR            = 0x0C,	// CLEAR key
			KC_RETURN           = 0x0D,	// ENTER key
			KC_SHIFT            = 0x10,	// SHIFT key
			KC_CONTROL          = 0x11,	// CTRL key
			KC_MENU             = 0x12,	// ALT key
			KC_PAUSE            = 0x13,	// PAUSE key
			KC_CAPITAL          = 0x14,	// CAPS LOCK key
			KC_ESCAPE           = 0x1B,	// ESC key
			KC_SPACE            = 0x20,	// SPACEBAR  
			KC_PRIOR            = 0x21,	// PAGE UP key
			KC_NEXT             = 0x22,	// PAGE DOWN key
			KC_END              = 0x23,	// END key
			KC_HOME             = 0x24,	// HOME key
			KC_LEFT             = 0x25,	// LEFT ARROW key
			KC_UP               = 0x26,	// UP ARROW key
			KC_RIGHT            = 0x27,	// RIGHT ARROW key
			KC_DOWN             = 0x28,	// DOWN ARROW key
			KC_SELECT           = 0x29,	// SELECT key
			KC_PRINT            = 0x2A,	// PRINT key
			KC_SNAPSHOT         = 0x2C,	// PRINT SCREEN key
			KC_INSERT           = 0x2D,	// INS key
			KC_DELETE           = 0x2E,	// DEL key
			KC_HELP             = 0x2F,	// HELP key
			KC_0                = 0x30,	// 0 key
			KC_1                = 0x31,	// 1 key
			KC_2                = 0x32,	// 2 key
			KC_3                = 0x33,	// 3 key
			KC_4                = 0x34,	// 4 key
			KC_5                = 0x35,	// 5 key
			KC_6                = 0x36,	// 6 key
			KC_7                = 0x37,	// 7 key
			KC_8                = 0x38,	// 8 key
			KC_9                = 0x39,	// 9 key
			KC_A                = 0x41,	// A key
			KC_B                = 0x42,	// B key
			KC_C                = 0x43,	// C key
			KC_D                = 0x44,	// D key
			KC_E                = 0x45,	// E key
			KC_F                = 0x46,	// F key
			KC_G                = 0x47,	// G key
			KC_H                = 0x48,	// H key
			KC_I                = 0x49,	// I key
			KC_J                = 0x4A,	// J key
			KC_K                = 0x4B,	// K key
			KC_L                = 0x4C,	// L key
			KC_M                = 0x4D,	// M key
			KC_N                = 0x4E,	// N key
			KC_O                = 0x4F,	// O key
			KC_P                = 0x50,	// P key
			KC_Q                = 0x51,	// Q key
			KC_R                = 0x52,	// R key
			KC_S                = 0x53,	// S key
			KC_T                = 0x54,	// T key
			KC_U                = 0x55,	// U key
			KC_V                = 0x56,	// V key
			KC_W                = 0x57,	// W key
			KC_X                = 0x58,	// X key
			KC_Y                = 0x59,	// Y key
			KC_Z                = 0x5A,	// Z key
			KC_LWIN             = 0x5B,	// Left Windows key
			KC_RWIN             = 0x5C,	// Right Windows key
			KC_NUMPAD0          = 0x60,	// Numeric keypad 0 key
			KC_NUMPAD1          = 0x61,	// Numeric keypad 1 key
			KC_NUMPAD2          = 0x62,	// Numeric keypad 2 key
			KC_NUMPAD3          = 0x63,	// Numeric keypad 3 key
			KC_NUMPAD4          = 0x64,	// Numeric keypad 4 key
			KC_NUMPAD5          = 0x65,	// Numeric keypad 5 key
			KC_NUMPAD6          = 0x66,	// Numeric keypad 6 key
			KC_NUMPAD7          = 0x67,	// Numeric keypad 7 key
			KC_NUMPAD8          = 0x68,	// Numeric keypad 8 key
			KC_NUMPAD9          = 0x69,	// Numeric keypad 9 key
			KC_MULTIPLY         = 0x6A,	// Multiply key
			KC_ADD              = 0x6B,	// Add key
			KC_SEPARATOR        = 0x6C,	// Separator key
			KC_SUBTRACT         = 0x6D,	// Subtract key
			KC_DECIMAL          = 0x6E,	// Decimal key
			KC_DIVIDE           = 0x6F,	// Divide key
			KC_F1               = 0x70,	// F1 key
			KC_F2               = 0x71,	// F2 key
			KC_F3               = 0x72,	// F3 key
			KC_F4               = 0x73,	// F4 key
			KC_F5               = 0x74,	// F5 key
			KC_F6               = 0x75,	// F6 key
			KC_F7               = 0x76,	// F7 key
			KC_F8               = 0x77,	// F8 key
			KC_F9               = 0x78,	// F9 key
			KC_F10              = 0x79,	// F10 key
			KC_F11              = 0x7A,	// F11 key
			KC_F12              = 0x7B,	// F12 key
			KC_F13              = 0x7C,	// F13 key
			KC_F14              = 0x7D,	// F14 key
			KC_F15              = 0x7E,	// F15 key
			KC_F16              = 0x7F,	// F16 key
			KC_F17              = 0x80,	// F17 key
			KC_F18              = 0x81,	// F18 key
			KC_F19              = 0x82,	// F19 key
			KC_F20              = 0x83,	// F20 key
			KC_F21              = 0x84,	// F21 key
			KC_F22              = 0x85,	// F22 key
			KC_F23              = 0x86,	// F23 key
			KC_F24              = 0x87,	// F24 key
			KC_NUMLOCK          = 0x90,	// NUM LOCK key
			KC_SCROLL           = 0x91,	// SCROLL LOCK key
			KC_LSHIFT           = 0xA0,	// Left SHIFT key 
			KC_RSHIFT           = 0xA1,	// Right SHIFT key 
			KC_LCONTROL         = 0xA2,	// Left CONTROL key 
			KC_RCONTROL         = 0xA3,	// Right CONTROL key 
			KC_LMENU            = 0xA4,	// Left MENU key 
			KC_RMENU            = 0xA5,	// Right MENU key 
			KC_PLUS             = 0xBB,	// Plus key (+)
			KC_COMMA            = 0xBC,	// Comma key(,)
			KC_MINUS            = 0xBD,	// Minus key(-)
			KC_PERIOD	        = 0xBE,	// Period Key (.)
			KC_TILDA            = 0xC0,	// Tilda Key (`)
			KC_MAX		        = 0xFF
		};

		enum ETouchEventType
		{
			TET_BEGIN,
			TET_MOVE,
			TET_END,
			TET_CANCELLED
		};

		typedef void* teTouchIdentifier;
		
		struct teTouchEvent
		{
			teVector2df Position;
			teTouchIdentifier TouchIdentifier;

			teTouchEvent()
				:TouchIdentifier(NULL)
			{
			}
		};
		
		enum ETouchDataType
		{
			TDT_START = 1,
			TDT_NORMAL = 2,
			TDT_DELTA_FROM_LAST = 3,
			TDT_DELTA_FROM_START = 4
		};
		
		enum ETouchFingerResolveType
		{
			//! By Pointers
			//! inputmanager find pointer position in touch began array
			TFRT_POINTERS,
			
			//! By closest last finger
			//! inputmanager find closest finger in last position array, it's not so accurate
			TFRT_LAST_CLOSEST
		};
		
		class teInputManager : public te::teReferenceCounter
		{
		protected:
			teVector3df CurrentAccelerometer[2];
			
			struct teTouchData
			{
				teTouchIdentifier TouchIdentifier;
				teVector2df StartPosition;
				teVector2df CurrentPosition;
				teVector2df DeltaFromLastPosition;
				teVector2df DeltaFromStartPosition;

				teTouchData()
					:TouchIdentifier(NULL)
				{
				}

				const teVector2df & GetTouchPosition(ETouchDataType Type)
				{
					switch(Type)
					{
					case TDT_START:
						return StartPosition;
					case TDT_NORMAL:
					default:
						return CurrentPosition;
					case TDT_DELTA_FROM_LAST:
						return DeltaFromLastPosition;
					case TDT_DELTA_FROM_START:
						return DeltaFromStartPosition;
					}
				}
			};

			u8 CurrentCountOfTouches;
			teTouchEvent CurrentTouchesUpdateArray[TE_INPUT_TOUCHES_MAX];
			teTouchData CurrentTouches[TE_INPUT_TOUCHES_MAX];
			ETouchFingerResolveType CurrentResolveType;
			u8 ResolveFinger(u8 Index);

			#ifdef TE_INPUT_SUPPORT_KEYBOARD

			struct teKeyData
			{
				u1 KeyDown;
				u1 KeyDownMessageArrive;
				u32 KeyHits;
				u32 KeyHitsProcessed;

				teKeyData()
					:KeyDown(false), KeyDownMessageArrive(false), KeyHits(0), KeyHitsProcessed(0)
				{
				}
			};

			teKeyData CurrentKeyData[256];

			#endif

		public:
			teInputManager();
			~teInputManager();

			// ------------------------------------------------------------------------ Accelerometer
			
			void SetAccelerometer(const teVector3df & Rotation, u8 Index = 0);
			
			const teVector3df & GetAccelerometer(u8 Index = 0);

			// ------------------------------------------------------------------------ Touches
			
			u8 GetTouchesCount();

			const teVector2df & GetTouch(u8 Finger, ETouchDataType Type = TDT_NORMAL);

			teVector2df GetVirtualTouch(const teVector2df & Touch, u1 ThisIsDeltaValue = false);

			teVector2df GetVirtualTouch(u8 Finger, ETouchDataType Type = TDT_NORMAL);

			teTouchIdentifier GetTouchIdentifier(u8 Finger);

			u1 GetTouchFinger(teTouchIdentifier Identifier, u8 & Finger);

			u1 IsValidIdentifier(teTouchIdentifier Identifier);
			
			teTouchEvent * LockTouch();

			void UnlockTouch(ETouchEventType EventType, u8 CountOfTouches);
			
			void SetFingerResolveType(ETouchFingerResolveType ResolveType);

			void ForceClearTouches();

			// ------------------------------------------------------------------------ Keyboard

			#ifdef TE_INPUT_SUPPORT_KEYBOARD

			u1 IsKeyHit(EKeyCode Key);

			u1 IsKeyDown(EKeyCode Key, u1 TrueWhenDownMessageArrive = false);

			void FlushKeysHits();

			void SetKeyDown(EKeyCode Key, u1 Down);

			#endif
		};
		
		teInputManager * GetInputManager();
	}
}

#endif
