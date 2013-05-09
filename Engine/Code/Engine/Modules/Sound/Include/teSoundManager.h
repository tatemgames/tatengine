/*
 *  teSoundManager.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 07/5/11.
 *  Copyright 2011 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TESOUNDMANAGER_H
#define TE_TESOUNDMANAGER_H

#include "teReferenceCounter.h"
#include "teSound.h"
#include "teString.h"

namespace te
{
	namespace sound
	{
		class teSoundManager
		{
		public:
			teSoundManager();
			~teSoundManager();

			void OnTick();

			FMOD_SYSTEM * GetSystem();

			void InitSound(teSound & sound);

			static void CheckResult(FMOD_RESULT result);

			#ifdef TE_PLATFORM_IPHONE
				void OnRestoreAudioSession();
			#endif

		protected:
			FMOD_SYSTEM * system;
			c8 * pool;
		};

		teSoundManager * GetSoundManager();
	}
}

#endif
