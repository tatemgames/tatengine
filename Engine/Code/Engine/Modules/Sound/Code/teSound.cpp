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
		//! Play
		void teSound::Play()
		{
			teSoundManager::CheckResult(GetSoundManager()->GetSystem()->playSound(FMOD_CHANNEL_FREE, sound, stream, &channel));
			channel->setMode((loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF));
			channel->setPaused(false);
		}

		//! Stop
		void teSound::Stop()
		{
			if(channel)
			{
				teSoundManager::CheckResult(channel->stop());
				channel = NULL;
			}
		}

		//! Pause
		void teSound::Pause()
		{
			if(channel)
				teSoundManager::CheckResult(channel->setPaused(true));
		}

		//! Resume
		void teSound::Resume()
		{
			if(channel)
				teSoundManager::CheckResult(channel->setPaused(false));
		}

		//! Is Playing
		u1 teSound::IsPlaying()
		{
			u1 isPlaying = false;

			if(channel)
				teSoundManager::CheckResult(channel->isPlaying(&isPlaying));

			return isPlaying;
		}

		//! Set Pan
		void teSound::SetPan(f32 pan)
		{
			if(channel)
				channel->setPan(pan);
		}

		//! Get Pan
		f32 teSound::GetPan()
		{
			f32 pan = 0.0f;

			if(channel)
				teSoundManager::CheckResult(channel->getPan(&pan));

			return pan;
		}

		//! Set Volume
		void teSound::SetVolume(f32 volume)
		{
			if(channel)
				channel->setVolume(volume);
		}

		//! Get Volume
		f32 teSound::GetVolume()
		{
			f32 volume = 0.0f;

			if(channel)
				teSoundManager::CheckResult(channel->getVolume(&volume));

			return volume;
		}
	}
}
