/*
 *  CMemoryBuffer.cpp
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 11/24/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#include "CMemoryBuffer.h"
#include "teMemoryManager.h"

namespace te
{
	namespace core
	{
		//! Constructor
		CMemoryBuffer::CMemoryBuffer()
			:Array(NULL), ArraySize(0)
		{
		}
		
		//! Destructor
		CMemoryBuffer::~CMemoryBuffer()
		{
			Lock(BLT_WRITE);
			DeAllocate();
			Unlock();
		}
		
		//! Allocate Buffer for data,size is count of objects,if already allocated - reallocate with new size
		void CMemoryBuffer::Allocate(u32 Size)
		{
			TE_ASSERT(lock)
			TE_ASSERT(lockType & BLT_WRITE)
			TE_ASSERT(Size > 0)
			
			ArraySize = Size;
			
			if(Array)
				Array = reinterpret_cast<u8*>(TE_REALLOCATE(Array, ArraySize));
			else
				Array = reinterpret_cast<u8*>(TE_ALLOCATE(ArraySize));
		}
		
		//! DeAllocate Buffer
		void CMemoryBuffer::DeAllocate()
		{
			TE_ASSERT(lock)
			TE_ASSERT(lockType & BLT_WRITE)

			if(Array)
			{
				TE_FREE(Array);
				Array = NULL;
				ArraySize = 0;
			}
		}
		
		//! Read Data
		void CMemoryBuffer::Read(void * Pointer, u32 Size)
		{
			TE_ASSERT(lock)
			TE_ASSERT(lockType & BLT_READ)
			
			memcpy(Pointer, Array + position, Size);
			
			if(streamMode)
				position += Size;
		}
		
		//! Write Data
		void CMemoryBuffer::Write(const void * Pointer, u32 Size)
		{
			TE_ASSERT(lock)
			TE_ASSERT(lockType & BLT_WRITE)
			
			memcpy(Array + position, Pointer, Size);
			
			if(streamMode)
				position += Size;
		}
		
		//! Write Data From Other Buffer
		void CMemoryBuffer::WriteFrom(IBuffer * Buffer, u32 Size)
		{
			TE_ASSERT(lock)
			TE_ASSERT(lockType & BLT_WRITE)
			
			Buffer->Read(Array + position, Size);

			if(streamMode)
				position += Size;
		}
		
		//! Get Size
		u32 CMemoryBuffer::GetSize() const
		{
			return ArraySize;
		}
		
		//! Get Array
		u8 * CMemoryBuffer::GetArray()
		{
			return Array;
		}
	}
}
