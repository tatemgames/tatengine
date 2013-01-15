/*
 *  teEngineController.cpp
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 11/24/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#include "teEngineController.h"
#include "teMemoryManager.h"
#include "teLogManager.h"
#include "teTimer.h"
#include "teFpsCounter.h"
#include "tePlatform.h"
#include "teBufferSecurity.h"
#include "teFileManager.h"
#include "teJSON.h"

namespace te
{
	namespace core
	{
		void InitEngine()
		{
			MemoryManagerInit();
			TE_NEW_S(teLogManager);
			TE_NEW_S(teTimer);
			TE_NEW_S(teFpsCounter);
			//TE_NEW_S(teBufferSecurity);
			TE_NEW_S(tePlatform);
			TE_NEW_S(teFileManager);

			TE_LOG("TatEngine ver '%s', inited", TE_VERSION);
		}
		
		//! Close engine
		void CloseEngine()
		{
			TE_LOG("TatEngine stoped");

			TE_DELETE(GetFileManager());
			TE_DELETE(GetPlatform());
			//TE_DELETE(GetBufferSecurity());
			TE_DELETE(GetFpsCounter());
			TE_DELETE(GetTimer());
			TE_DELETE(GetLogManager());
			MemoryManagerClose();
		}
	}
}
