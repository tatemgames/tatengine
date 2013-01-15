/*
 *  teLogManager.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 11/23/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TELOGMANAGER_H
#define TE_TELOGMANAGER_H

#include "teString.h"

namespace te
{
	namespace core
	{
		enum ELogType
		{
			LT_ERROR,
			LT_WARNING,
			LT_DEFAULT,
			LT_DEBUG
		};
		
		class teLogManager
		{
		public:
			teLogManager();
			~teLogManager();

			teStringConcate & GetConcate() {return concate;}

			void FormLogMessage(ELogType type, const c8 * format, ...)
			{
				va_list args;
				va_start(args, format);
				teString msg = GetConcate().AddVA(format, args).BakeToString();
				OnLogConsole(msg, type);
				va_end(args);
			}

		protected:
			void OnLogConsole(const teString & message, ELogType type);
			teStringPool pool;
			teStringConcate concate;
		};

		teLogManager * GetLogManager();
	}
}

#define TE_LOG_ERR(...)	{te::core::GetLogManager()->FormLogMessage(te::core::LT_ERROR, __VA_ARGS__);}
#define TE_LOG_WRN(...)	{te::core::GetLogManager()->FormLogMessage(te::core::LT_WARNING, __VA_ARGS__);}

#ifdef TE_DEBUG
	#define TE_LOG_DBG(...)	{te::core::GetLogManager()->FormLogMessage(te::core::LT_DEBUG, __VA_ARGS__);}
	#define TE_LOG(...)		{te::core::GetLogManager()->FormLogMessage(te::core::LT_DEFAULT, __VA_ARGS__);}
#else
	#define TE_LOG_DBG(...) {}
	#define TE_LOG(...)		{}
#endif
 
#endif
