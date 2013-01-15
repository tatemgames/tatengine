/*
 *  teBufferSecurity.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 08/19/10.
 *  Copyright 2010 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEBUFFERSECURITY_H
#define TE_TEBUFFERSECURITY_H

#include "IBuffer.h"
#include "teString.h"

namespace te
{
	namespace core
	{
		u32 GetCRC32(const u8 * buffer, size_t size);
		u32 GetCRC32(IBuffer * buffer);
		
		u32 Encrypt(u8 * buffer, size_t size, teString key);
		u1 Decrypt(u8 * buffer, size_t size, teString key, u32 crc);
	}
}

#endif
