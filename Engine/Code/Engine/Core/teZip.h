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
			void AddFile(const teString & name, IBuffer * buffer);
			void Finalize();
			void SaveTo(const teString & fileName);

		protected:
			teConstArray<u8> data;
			void ReserveDataSize(u32 size);
		};
	}
}

#endif