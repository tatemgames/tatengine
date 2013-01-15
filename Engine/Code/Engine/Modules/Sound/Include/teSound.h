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
#include "fmod_cpp.h"

namespace te
{
	namespace sound
	{
		struct teSound
		{
			TE_INLINE teSound(FMOD::System * setSystem)
				:sound(NULL), channel(NULL), soundIndex(u32Max), stream(false), loop(false)
			{
			}

			void Deinit()
			{
				if(sound)
				{
					sound->release();
					sound = NULL;
				}
			}

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

			FMOD::Sound * sound;
			FMOD::Channel * channel;
		};
	}
}

#endif
