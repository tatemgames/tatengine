/*
 *  teSound.cpp
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 07/5/11.
 *  Copyright 2011 Tatem Games. All rights reserved.
 *
 */

#include "teSound.h"
#include "teSoundManager.h"

namespace te
{
	namespace sound
	{
		teSound::teSound(FMOD_SYSTEM * setSystem)
			:sound(NULL), channel(NULL), soundIndex(u32Max), stream(false), loop(false)
		{
		}

		void teSound::Deinit()
		{
			if(sound)
			{
				FMOD_Sound_Release(sound);
				sound = NULL;
			}
		}

		//! Play
		void teSound::Play()
		{
			teSoundManager::CheckResult(FMOD_System_PlaySound(GetSoundManager()->GetSystem(), FMOD_CHANNEL_REUSE, sound, stream, &channel));
			teSoundManager::CheckResult(FMOD_Channel_SetMode(channel, (loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF)));
			teSoundManager::CheckResult(FMOD_Channel_SetPaused(channel, false));
		}

		//! Stop
		void teSound::Stop()
		{
			if(channel)
			{
				teSoundManager::CheckResult(FMOD_Channel_Stop(channel));
				channel = NULL;
			}
		}

		//! Pause
		void teSound::Pause()
		{
			if(channel)
				teSoundManager::CheckResult(FMOD_Channel_SetPaused(channel, true));
		}

		//! Resume
		void teSound::Resume()
		{
			if(channel)
				teSoundManager::CheckResult(FMOD_Channel_SetPaused(channel, false));
		}

		//! Is Playing
		u1 teSound::IsPlaying()
		{
			FMOD_BOOL isPlaying = false;

			if(channel)
				teSoundManager::CheckResult(FMOD_Channel_IsPlaying(channel, &isPlaying));

			return isPlaying ? true : false;
		}

		//! Set Pan
		void teSound::SetPan(f32 pan)
		{
			if(channel)
				teSoundManager::CheckResult(FMOD_Channel_SetPan(channel, pan));
		}

		//! Get Pan
		f32 teSound::GetPan()
		{
			f32 pan = 0.0f;

			if(channel)
				teSoundManager::CheckResult(FMOD_Channel_GetPan(channel, &pan));

			return pan;
		}

		//! Set Volume
		void teSound::SetVolume(f32 volume)
		{
			if(channel)
				teSoundManager::CheckResult(FMOD_Channel_SetVolume(channel, volume));
		}

		//! Get Volume
		f32 teSound::GetVolume()
		{
			f32 volume = 0.0f;

			if(channel)
				teSoundManager::CheckResult(FMOD_Channel_GetVolume(channel, &volume));

			return volume;
		}
	}
}
