/*
 *  teLogManager.cpp
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 11/23/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#include "teLogManager.h"

namespace te
{
	namespace core
	{
		teLogManager * currentLogManager = NULL;
		
		teLogManager::teLogManager()
			:pool(1024)
		{
			concate.SetBuffer(pool.Allocate(pool.GetSize()));

			currentLogManager = this;
			//Log.Connect(this, &teLogManager::OnLogConsole);
		}
			
		teLogManager::~teLogManager()
		{
			currentLogManager = NULL;
		}
		
		void teLogManager::OnLogConsole(const teString & message, ELogType messageType)
		{
			switch(messageType)
			{
			case LT_DEBUG:
				printf("%s\n", message.c_str());
				break;
			case LT_DEFAULT:
				printf("%s\n", message.c_str());
				break;
			case LT_WARNING:
				printf("! Warning ! %s\n", message.c_str());
				break;
			case LT_ERROR:
				printf("! Error ! %s\n", message.c_str());
				break;
			default:
				break;
			}
		}

		teLogManager * GetLogManager()
		{
			return currentLogManager;
		}
	}
}
