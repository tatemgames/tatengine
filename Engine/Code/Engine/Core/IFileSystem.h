/*
 *  IFileSystem.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 06/15/11.
 *  Copyright 2011 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_IFILESYSTEM_H
#define TE_IFILESYSTEM_H

#include "teReferenceCounter.h"
#include "teString.h"
#include "IBuffer.h"
#include "CFileBuffer.h"

namespace te
{
	namespace core
	{
		//! File Path Type
		enum EFilePathType
		{
			//! Default Path
			FPT_DEFAULT,

			//! Resources Path
			FPT_RESOURCES,
			
			//! Resources Unpacked
			FPT_RESOURCES_UNPACKED,

			//! User Resources Path
			FPT_USER_RESOURCES,

			//! Content Path
			FPT_CONTENT,

			//! Bin Path
			FPT_BIN,

			//! Max
			FPT_MAX
		};

		//! File System
		class IFileSystem : public te::teReferenceCounter
		{
		public:
			//! Constructor
			IFileSystem()
			{
			}

			//! Destructor
			virtual ~IFileSystem()
			{
			}

			//! Get Path
			virtual teString GetPath(EFilePathType Type)
			{
				return "";
			}

			//! Open File
			virtual IBuffer * OpenFile(const teString & FileName, CFileBuffer::EFileWorkMode Mode = CFileBuffer::FWM_READ, u32 Offset = 0, u32 Size = u32Max)
			{
				CFileBuffer * FileBuffer = NULL;
				TE_NEW(FileBuffer, CFileBuffer(FileName, Mode, Offset, Size))

				if(FileBuffer->IsValid())
					return FileBuffer;
				else
				{
					FileBuffer->Drop();
					return NULL;
				}
			}
		};
	}
}

#endif
