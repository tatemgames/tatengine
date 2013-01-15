/*
 *  CFileBuffer.cpp
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 11/24/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#include "CFileBuffer.h"
#include "teLogManager.h"

namespace te
{
	namespace core
	{
		//! Constructor
		CFileBuffer::CFileBuffer(const teString & FileName, EFileWorkMode FileWorkMode, u32 FileBlockOffset, u32 FileBlockLength)
			:CurrentFileWorkMode(FileWorkMode)
		{
			switch(CurrentFileWorkMode)
			{
			case FWM_READ:
				CurrentFile = fopen(FileName.c_str(), "rb");
				break;
			case FWM_WRITE:
				CurrentFile = fopen(FileName.c_str(), "wb");
				break;
			case FWM_READWRITE:
				CurrentFile = fopen(FileName.c_str(), "wb+");
				break;
			default:
				break;
			}

			if(CurrentFile)
			{
				FileOffset = FileBlockOffset;

				if(FileBlockLength != u32Max)
				{
					FileSize = FileBlockLength;
				}
				else
				{
					fseek(CurrentFile, 0, SEEK_END);
					FileSize = ftell(CurrentFile) - FileOffset;
					fseek(CurrentFile, 0, SEEK_SET);
				}

				SetPosition(0);
			}
		}

		//! Destructor
		CFileBuffer::~CFileBuffer()
		{
			if(CurrentFile)
				fclose(CurrentFile);
		}

		//! Allocate Buffer for data, if already allocated - reallocate with new size
		void CFileBuffer::Allocate(u32 Size)
		{
			FileSize = Size;
		}

		//! DeAllocate Buffer
		void CFileBuffer::DeAllocate()
		{
			fseek(CurrentFile, FileOffset, SEEK_SET);
		}

		//! Read Data
		void CFileBuffer::Read(void * Pointer, u32 Size)
		{
			TE_ASSERT(lock)
			TE_ASSERT(lockType & BLT_READ)

			fread(Pointer, Size, 1, CurrentFile);

			if(streamMode)
				position += Size;
			else
				fseek(CurrentFile, position + FileOffset, SEEK_SET);
		}

		//! Write Data
		void CFileBuffer::Write(const void * Pointer, u32 Size)
		{
			TE_ASSERT(lock)
			TE_ASSERT(lockType & BLT_WRITE)

			fwrite(Pointer, Size, 1, CurrentFile);

			if(streamMode)
				position += Size;
			else
				fseek(CurrentFile, position + FileOffset, SEEK_SET);
		}

		//! Get Size
		u32 CFileBuffer::GetSize() const
		{
			return FileSize;
		}

		//! Set Position
		void CFileBuffer::SetPosition(u32 Position, u1 Relative, u1 Forward)
		{
			IBuffer::SetPosition(Position, Relative, Forward);
			fseek(CurrentFile, position + FileOffset, SEEK_SET);
		}

		//! Is Valid
		u1 CFileBuffer::IsValid()
		{
			return CurrentFile ? true : false;
		}
	}
}
