/*
 *  CMemoryBuffer.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 11/24/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_MEMORYBUFFER_H
#define TE_MEMORYBUFFER_H

#include "IBuffer.h"

namespace te
{
	namespace core
	{
		//! Memory Buffer
		class CMemoryBuffer : public te::core::IBuffer
		{
		protected:
			//! Array Size
			u32 ArraySize;

			//! Array
			u8 * Array;
			
		public:
			//! Constructor
			CMemoryBuffer();
			
			//! Destructor
			virtual ~CMemoryBuffer();
			
			//! Allocate Buffer for data, if already allocated - reallocate with new size
			void Allocate(u32 Size);
			
			//! DeAllocate Buffer
			void DeAllocate();
			
			//! Read Data
			void Read(void * Pointer, u32 Size);
			
			//! Write Data
			void Write(const void * Pointer, u32 Size);
			
			//! Write Data From Other Buffer
			void WriteFrom(IBuffer * Buffer, u32 Size);
			
			//! Get Size
			u32 GetSize() const;
			
			//! Get Array
			u8 * GetArray();
		};
	}
}

#endif
