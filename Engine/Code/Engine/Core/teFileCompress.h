/*
 *  teFileCompress.h
 *  TatEngine
 *
 *  Created by Vadim Luchko on 09/6/12.
 *  Copyright 2012 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEFILECOMPRESS_H
#define TE_TEFILECOMPRESS_H

#include "teString.h"

namespace te
{
	namespace core
	{
		void teCompressFile(const teString & from, const teString & to, u1 highCompression = false, u1 localPath = true);
		void teDecodeFile(const teString & from, const teString & to, u1 localPath = true);
	}
}

#endif
