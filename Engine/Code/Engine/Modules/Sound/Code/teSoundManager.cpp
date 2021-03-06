/*
 *  teSoundManager.cpp
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 07/5/11.
 *  Copyright 2011 Tatem Games. All rights reserved.
 *
 */

#include "teSoundManager.h"
#include "teFileManager.h"
#include "teMath.h"
#include "teLogManager.h"
#include "fmod_errors.h"

#ifdef TE_PLATFORM_IPHONE
	#include "tePlatform.h"
	#include "tePlatform_iOS.h"
	#include "fmodiphone.h"
#endif

namespace te
{
	namespace sound
	{
		void * F_CALLBACK teFMODAlloc(unsigned int size, FMOD_MEMORY_TYPE type, const char * source) {return TE_ALLOCATE(size);}
		void * F_CALLBACK teFMODRealloc(void * ptr, u32 size, FMOD_MEMORY_TYPE type, const c8 * source) {return TE_REALLOCATE(ptr, size);}
		void F_CALLBACK teFMODFree(void * ptr, FMOD_MEMORY_TYPE type, const c8 * source) {TE_FREE(ptr);}

		FMOD_RESULT F_CALLBACK teFMODOpenFile(const c8 * name, s32 unicode, u32 * filesize, void ** handle, void ** userData)
		{
			if(!name)
				return FMOD_ERR_INVALID_PARAM;

			core::IBuffer * buffer = core::GetFileManager()->OpenFile(name);

			if(!buffer)
				return FMOD_ERR_FILE_NOTFOUND;

			buffer->SetPosition(0);
			*filesize = buffer->GetSize();
			*handle = buffer;

			return FMOD_OK;
		}

		FMOD_RESULT F_CALLBACK teFMODCloseFile(void * handle, void * userData)
		{
			if(!handle)
				return FMOD_ERR_INVALID_PARAM;

			static_cast<core::IBuffer*>(handle)->Drop();
			return FMOD_OK;
		}

		FMOD_RESULT F_CALLBACK teFMODReadFile(void * handle, void * memoryBuffer, u32 sizeBytes, u32 * bytesRead, void * userData)
		{
			if(!handle || !bytesRead)
				return FMOD_ERR_INVALID_PARAM;

			core::IBuffer * buffer = static_cast<core::IBuffer*>(handle);

			*bytesRead = teMin(buffer->GetPosition() + sizeBytes, buffer->GetSize()) - buffer->GetPosition();

			if(*bytesRead > 0)
			{
				buffer->Lock(core::BLT_READ);
				buffer->Read(memoryBuffer, *bytesRead);
				buffer->Unlock();
			}

			if(*bytesRead < sizeBytes)
				return FMOD_ERR_FILE_EOF;
			else
				return FMOD_OK;
		}

		FMOD_RESULT F_CALLBACK teFMODSeekFile(void * handle, u32 pos, void * userData)
		{
			if(!handle)
				return FMOD_ERR_INVALID_PARAM;

			static_cast<core::IBuffer*>(handle)->SetPosition(pos);
			return FMOD_OK;
		}

		teSoundManager * currentSoundManager = NULL;

		teSoundManager::teSoundManager()
			:system(NULL), pool(NULL)
		{
			pool = (c8*)TE_ALLOCATE(TE_SOUND_FMOD_MEMORY_POOL_SIZE);

			FMOD_Memory_Initialize(pool, TE_SOUND_FMOD_MEMORY_POOL_SIZE, NULL, NULL, NULL, FMOD_MEMORY_ALL);
			//FMOD_Memory_Initialize(NULL, 0, teFMODAlloc, teFMODRealloc, teFMODFree, FMOD_MEMORY_ALL);

			CheckResult(FMOD_System_Create(&system));

			#ifdef TE_PLATFORM_ANDROID
				FMOD_System_SetOutput((FMOD_SYSTEM*)system, FMOD_OUTPUTTYPE_AUDIOTRACK);
			#endif

			u32 version = 0;
			CheckResult(FMOD_System_GetVersion(system, &version));

			if(version < FMOD_VERSION)
				TE_LOG_ERR("old version of fmod library");

			CheckResult(FMOD_System_SetSoftwareFormat(system, 44100, FMOD_SOUND_FORMAT_PCM16, 0, 0, FMOD_DSP_RESAMPLER_LINEAR));

			CheckResult(FMOD_System_Init(system, 64, FMOD_INIT_NORMAL, NULL));

			CheckResult(FMOD_System_SetFileSystem(system, teFMODOpenFile, teFMODCloseFile, teFMODReadFile, teFMODSeekFile, 0, 0, 2048));

			#ifdef TE_PLATFORM_IPHONE
				static_cast<core::tePlatform_iOS*>(core::GetPlatform()->GetCurrentDevicePlatform())->OnBecomeActive.Connect(this, &teSoundManager::OnRestoreAudioSession);
			#endif

			currentSoundManager = this;
		}

		teSoundManager::~teSoundManager()
		{
			#ifdef TE_PLATFORM_IPHONE
				static_cast<core::tePlatform_iOS*>(core::GetPlatform()->GetCurrentDevicePlatform())->OnBecomeActive.Remove(this, &teSoundManager::OnRestoreAudioSession);
			#endif

			if(system)
			{
				CheckResult(FMOD_System_Close(system));
				CheckResult(FMOD_System_Release(system));
				system = NULL;
			}

			TE_FREE(pool);

			currentSoundManager = NULL;
		}

		void teSoundManager::OnTick()
		{
			CheckResult(FMOD_System_Update(system));
		}

		FMOD_SYSTEM * teSoundManager::GetSystem()
		{
			return system;
		}

		void teSoundManager::InitSound(teSound & sound)
		{
			FMOD_SOUND * fmodSound = NULL;
			FMOD_MODE flags = FMOD_HARDWARE | FMOD_LOWMEM | FMOD_2D;

			if(sound.stream)
				CheckResult(FMOD_System_CreateStream(system, core::GetLogManager()->GetConcate().Add("sound_%i", sound.soundIndex).BakeToString().c_str(), flags, NULL, &fmodSound));
			else
				CheckResult(FMOD_System_CreateSound(system, core::GetLogManager()->GetConcate().Add("sound_%i", sound.soundIndex).BakeToString().c_str(), flags, NULL, &fmodSound));

			if(!fmodSound)
			{
				TE_LOG_ERR("FMOD error, sound obj is null");
				return;
			}

			sound.sound = fmodSound;
			FMOD_Sound_SetUserData(fmodSound, &sound);
		}

		void teSoundManager::CheckResult(FMOD_RESULT result)
		{
			if(result != FMOD_OK)
				TE_LOG_ERR("FMOD error '%s'", FMOD_ErrorString(result));
		}

		#ifdef TE_PLATFORM_IPHONE

		void teSoundManager::OnRestoreAudioSession()
		{
			FMOD_IPhone_RestoreAudioSession();
		}

		#endif

		teSoundManager * GetSoundManager()
		{
			return currentSoundManager;
		}
	}
}
