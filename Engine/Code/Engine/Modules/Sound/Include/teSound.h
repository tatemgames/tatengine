/*
 *  teSound.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 07/5/11.
 *  Copyright 2011 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TESOUND_H
#define TE_TESOUND_H

#include "teReferenceCounter.h"
#include "fmod.h"

namespace te
{
	namespace sound
	{
		struct teSound
		{
			teSound(FMOD_SYSTEM * setSystem);
			void Deinit();

			void Play();
			void Stop();
			void Pause();
			void Resume();

			u1 IsPlaying();

			void SetPan(f32 pan);
			f32 GetPan();

			void SetVolume(f32 volume);
			f32 GetVolume();

			u32 soundIndex;
			u1 stream;
			u1 loop;

			FMOD_SOUND * sound;
			FMOD_CHANNEL * channel;
		};
	}
}

#endif
