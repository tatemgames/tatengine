/*
 *  teZip.cpp
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 03/20/11.
 *  Copyright 2012 Tatem Games. All rights reserved.
 *
 */

#include "teZip.h"
#include "teBufferSecurity.h"
#include "teFileManager.h"

#ifdef TE_PLATFORM_WIN
#include <windows.h>
#endif

namespace te
{
	namespace core
	{
		#pragma pack(push, 1)

		struct teZipFileHeader
		{
			u32 signature;
			u16 versionToExtract;
			u16 generalFlag;
			u16 compressionMethod;
			u16 modTime;
			u16 modDate;
			u32 crc;
			u32 compressedSize;
			u32 uncompressedSize;
			u16 fileNameSize;
			u16 extraFieldSize;
			c8 fileName[];

			TE_INLINE void Clear()
			{
				signature = 0x04034b50;
				versionToExtract = 10;
				generalFlag = 0;
				compressionMethod = 0;
				modTime = 0;
				modDate = 0;
				crc = 0;
				compressedSize = 0;
				uncompressedSize = 0;
				fileNameSize = 0;
				extraFieldSize = 0;
			}

			TE_INLINE void SetSize(u32 size)
			{
				compressedSize = size;
				uncompressedSize = size;
			}
		};

		struct teZipCentralFileHeader
		{
			u32 signature;
			u16 versionMade;
			u16 versionToExtract;
			u16 generalFlag;
			u16 compressionMethod;
			u16 modTime;
			u16 modDate;
			u32 crc;
			u32 compressedSize;
			u32 uncompressedSize;
			u16 fileNameSize;
			u16 extraFieldSize;
			u16 fileCommentLength;
			u16 diskNumberStart;
			u16 internalFileAttributes;
			u32 externalFileAttributes;
			u32 relativeOffsetOfHeader;
			c8 fileName[];

			TE_INLINE void Clear()
			{
				signature = 0x02014b50;
				versionMade = 0;
				versionToExtract = 10;
				generalFlag = 0;
				compressionMethod = 0;
				modTime = 0;
				modDate = 0;
				crc = 0;
				compressedSize = 0;
				uncompressedSize = 0;
				fileNameSize = 0;
				extraFieldSize = 0;
				fileCommentLength = 0;
				diskNumberStart = 0;
				internalFileAttributes = 0;
				externalFileAttributes = 0;
				relativeOffsetOfHeader = 0;
			}

			TE_INLINE void SetFrom(const teZipFileHeader & header)
			{
				versionToExtract	= header.versionToExtract;
				generalFlag			= header.generalFlag;
				compressionMethod	= header.compressionMethod;
				modTime				= header.modTime;
				modDate				= header.modDate;
				crc					= header.crc;
				compressedSize		= header.compressedSize;
				uncompressedSize	= header.uncompressedSize;
				fileNameSize		= header.fileNameSize;
				extraFieldSize		= header.extraFieldSize;
			}
		};

		struct teZipCentralEnd
		{
			u32 signature;
			u16 numberDisk;
			u16 numberDisk2;
			u16 entriesCount;
			u16 entriesCount2;
			u32 sizeOfCentralDirectory;
			u32 centralDirectoryOffset;
			u16 fileCommentSize;

			TE_INLINE void Clear()
			{
				signature = 0x06054b50;
				numberDisk = 0;
				numberDisk2 = 0;
				entriesCount = 0;
				entriesCount2 = 0;
				sizeOfCentralDirectory = 0;
				centralDirectoryOffset = 0;
				fileCommentSize = 0;
			}
		};

		#pragma pack(pop)

		teZipMaker::teZipMaker()
			:debugMode(false)
		{
		}

		void teZipMaker::AddFile(const teString & name, IBuffer * buffer)
		{
			if(debugMode)
			{
				teString fileNameDebug = debugStringConcate.Add("%s%s", debugDirPath.c_str(), name.c_str()).BakeToString();

				IBuffer * fileBuffer = GetFileManager()->OpenFile(fileNameDebug, CFileBuffer::FWM_WRITE, false, false);

				if(fileBuffer)
				{
					c8 * temp = (c8*)TE_ALLOCATE(buffer->GetSize());
					buffer->Lock(BLT_READ);
					buffer->SetPosition(0);
					buffer->Read(temp, buffer->GetSize());
					buffer->Unlock();

					fileBuffer->Lock(BLT_WRITE);
					fileBuffer->SetPosition(0);
					fileBuffer->Allocate(buffer->GetSize());
					fileBuffer->Write(temp, buffer->GetSize());
					fileBuffer->Unlock();

					TE_SAFE_DROP(fileBuffer);
					TE_FREE(temp);
				}
			}
			else
			{
				u32 nameSize = name.GetSize();

				teZipFileHeader header;
				header.Clear();
				header.SetSize(buffer->GetSize());
				header.fileNameSize = nameSize;
				header.crc = GetCRC32(buffer); // TODO

				ReserveDataSize(sizeof(teZipFileHeader) + nameSize + buffer->GetSize());
				memcpy(data.Request(sizeof(teZipFileHeader)), &header, sizeof(teZipFileHeader));
				memcpy(data.Request(nameSize), name.c_str(), nameSize);

				buffer->Lock(core::BLT_READ);
				buffer->SetPosition(0);
				buffer->Read(data.Request(buffer->GetSize()), buffer->GetSize());
				buffer->Unlock();
			}
		}

		void teZipMaker::AddFile(const teString & archiveName, const teString & fileName)
		{
			if(debugMode)
			{
				#ifdef TE_PLATFORM_WIN
					teString fileNameDebug = debugStringConcate.Add("%s%s", debugDirPath.c_str(), archiveName.c_str()).BakeToString();
					CopyFileA(fileName.c_str(), fileNameDebug.c_str(), false);
				#else
					core::IBuffer * fileBuffer = core::GetFileManager()->OpenFile(fileName, core::CFileBuffer::FWM_READ, false, false);
					AddFile(archiveName, fileBuffer);
					TE_SAFE_DROP(fileBuffer);
				#endif
			}
			else
			{
				core::IBuffer * fileBuffer = core::GetFileManager()->OpenFile(fileName, core::CFileBuffer::FWM_READ);
				AddFile(archiveName, fileBuffer);
				TE_SAFE_DROP(fileBuffer);
			}
		}

		void teZipMaker::Finalize()
		{
			if(debugMode)
				return;

			u32 localFilesSize = data.GetAlive();
			u32 poolPosition = 0, filesCount = 0;

			while(poolPosition < localFilesSize)
			{
				const teZipFileHeader & header = *reinterpret_cast<const teZipFileHeader*>(data.At(poolPosition));

				teZipCentralFileHeader central;
				central.Clear();
				central.SetFrom(header);
				central.relativeOffsetOfHeader = poolPosition;

				ReserveDataSize(sizeof(teZipCentralFileHeader) + central.fileNameSize);
				memcpy(data.Request(sizeof(teZipCentralFileHeader)), &central, sizeof(teZipCentralFileHeader));
				memcpy(data.Request(central.fileNameSize), (*reinterpret_cast<const teZipFileHeader*>(data.At(poolPosition))).fileName, central.fileNameSize);

				poolPosition += sizeof(teZipFileHeader) + central.fileNameSize + central.compressedSize;
				++filesCount;
			}

			teZipCentralEnd end;
			end.Clear();
			end.entriesCount = filesCount;
			end.entriesCount2 = filesCount;
			end.sizeOfCentralDirectory = data.GetAlive() - localFilesSize;
			end.centralDirectoryOffset = localFilesSize;

			ReserveDataSize(sizeof(teZipCentralEnd));
			memcpy(data.Request(sizeof(teZipCentralEnd)), &end, sizeof(teZipCentralEnd));
		}

		void teZipMaker::SaveTo(const teString & fileName)
		{
			if(debugMode)
				return;

			core::IBuffer * outputBuffer = core::GetFileManager()->OpenFile(fileName, core::CFileBuffer::FWM_WRITE, false);
			outputBuffer->Lock(core::BLT_WRITE);
			outputBuffer->SetPosition(0);
			outputBuffer->Allocate(data.GetAlive());
			outputBuffer->Write(data.GetPool(), data.GetAlive());
			outputBuffer->Unlock();
			TE_SAFE_DROP(outputBuffer);
		}

		void teZipMaker::SetDebugMode(u1 setDebugMode, teString setDebugDirPath)
		{
			debugMode = setDebugMode;
			debugDirPath = setDebugDirPath;

			if(debugMode)
			{
				debugStringPool.Reserve(1024);
				debugStringConcate.SetBuffer(debugStringPool.Allocate(1024));
			}
			else
				debugStringPool.Clear();
		}


		void teZipMaker::ReserveDataSize(u32 size)
		{
			if(data.GetAlive() + size >= data.GetSize())
			{
				u32 blocksCount = (size / 1024) + 1;
				TE_ASSERT(blocksCount * 1024 >= size);
				data.ReserveMore(blocksCount * 1024);
			}
		}
	}
}
