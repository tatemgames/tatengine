/*
 *  CFileBuffer.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 11/24/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_CFILEBUFFER_H
#define TE_CFILEBUFFER_H

#include "IBuffer.h"
#include "teString.h"

namespace te
{
	namespace core
	{
		//! File Buffer
		class CFileBuffer : public te::core::IBuffer
		{
		public:
			//! File Work Mode
			enum EFileWorkMode
			{
				//! Read
				FWM_READ = 1,
				
				//! Write
				FWM_WRITE = 2,
				
				//! Read and write
				FWM_READWRITE = 3
			};
		protected:
			//! Current File
			FILE * CurrentFile;
			
			//! File Offset
			u32 FileOffset;

			//! File Size
			u32 FileSize;
			
			//! File Mode
			EFileWorkMode CurrentFileWorkMode;
		public:
			//! Constructor
			CFileBuffer(const teString & FileName, EFileWorkMode FileWorkMode = FWM_READ, u32 FileBlockOffset = 0, u32 FileBlockLength = u32Max);

			//! Destructor
			virtual ~CFileBuffer();
			
			//! Allocate Buffer for data, if already allocated - reallocate with new size
			void Allocate(u32 Size);
			
			//! DeAllocate Buffer
			void DeAllocate();
			
			//! Read Data
			void Read(void * Pointer, u32 Size);
			
			//! Write Data
			void Write(const void * Pointer, u32 Size);
			
			//! Get Size
			u32 GetSize() const;
			
			//! Set Position
			void SetPosition(u32 Position, u1 Relative = false, u1 Forward = true);
			
			//! Is Valid
			u1 IsValid();

			//! Get File
			FILE * GetFile()
			{
				return CurrentFile;
			}
		};
	}
}

#endif
