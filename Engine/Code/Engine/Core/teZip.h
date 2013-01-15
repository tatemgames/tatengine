/*
 *  teZip.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 03/20/11.
 *  Copyright 2012 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_ZIP_H
#define TE_ZIP_H

#include "teConstArray.h"
#include "IBuffer.h"
#include "teString.h"

namespace te
{
	namespace core
	{
		class teZipMaker
		{
		public:
			teZipMaker();

			void AddFile(const teString & name, IBuffer * buffer);
			void AddFile(const teString & archiveName, const teString & fileName);
			void Finalize();
			void SaveTo(const teString & fileName);

			void SetDebugMode(u1 setDebugMode, teString setDebugDirPath = "");
			TE_INLINE u1 GetDebugMode() const {return debugMode;}

		protected:
			teConstArray<u8> data;
			teString debugDirPath;
			teStringPool debugStringPool;
			teStringConcate debugStringConcate;
			u1 debugMode;
			void ReserveDataSize(u32 size);
		};
	}
}

#endif