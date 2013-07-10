/*
 *  teFileCompress.cpp
 *  TatEngine
 *
 *  Created by Vadim Luchko on 09/6/12.
 *  Copyright 2012 Tatem Games. All rights reserved.
 *
 */

#include "teFileCompress.h"
#include "teFileManager.h"
#include "teLogManager.h"
#include "lz4.h"
#include "lz4hc.h"

#define CHUNKSIZE (u32)(8<<20) // 8 MB
#define ARCHIVE_MAGICNUMBER 0x184C2102
#define ARCHIVE_MAGICNUMBER_SIZE 4

namespace te
{
	namespace core
	{
		typedef int (*teLZ4CompressionFunction)(const char *, char *, int);

		void teCompressData(c8 * input, u32 inputSize, c8 * output, u32 outputSize, u32 & resultSize, u1 highCompression)
		{
			teLZ4CompressionFunction compressionFunction = highCompression ? LZ4_compressHC : LZ4_compress;

			resultSize = compressionFunction(input, output + 4, inputSize);
			*((u32*)output + 0) = resultSize;
			resultSize += 4;
		}

		void teDecodeData(c8 * input, u32 inputSize, c8 * output, u32 outputSize, u32 & resultSize)
		{
			resultSize = LZ4_uncompress_unknownOutputSize(input + 4, output, *((u32*)input + 0), outputSize);
		}

		void teCompressFile(const teString & from, const teString & to, u32 chunkSize, c8 * chunkInputBuffer, u32 chunkInputBufferSize, c8 * chunkOutputBuffer, u32 chunkOutputBufferSize, u1 highCompression, u1 localPath)
		{
			TE_ASSERT(chunkInputBufferSize >= chunkSize);
			TE_ASSERT(chunkOutputBufferSize >= LZ4_compressBound(chunkSize));

			teLZ4CompressionFunction compressionFunction = highCompression ? LZ4_compressHC : LZ4_compress;

			IBuffer * fileInput = GetFileManager()->OpenFile(from, CFileBuffer::FWM_READ, localPath);

			if(!fileInput)
				return;

			IBuffer * fileOutput = GetFileManager()->OpenFile(to, CFileBuffer::FWM_WRITE, localPath);

			if(!fileOutput)
			{
				TE_SAFE_DROP(fileInput);
				return;
			}

			fileInput->SetStreamMode(true);
			fileOutput->SetStreamMode(true);

			fileInput->Lock(BLT_READ);
			fileInput->SetPosition(0);

			fileOutput->Lock(BLT_WRITE);
			fileOutput->SetPosition(0);

			u32 magicNumber = ARCHIVE_MAGICNUMBER;
			fileOutput->Write(&magicNumber, ARCHIVE_MAGICNUMBER_SIZE);

			c8 * inBuffer = chunkInputBuffer;
			c8 * outBuffer = chunkOutputBuffer;

			u32 fileSize = 0;

			while(true)
			{
				u32 inSize = teMin(chunkSize, fileInput->GetSize() - fileInput->GetPosition());
				fileInput->Read(inBuffer, inSize);
				fileSize += inSize;

				if(!inSize)
					break;

				u32 outSize = compressionFunction(inBuffer, outBuffer + 4, inSize);
				*(u32*)outBuffer = outSize;

				fileOutput->Write(outBuffer, outSize + 4);
			}

			fileInput->Unlock();
			fileOutput->Unlock();

			TE_SAFE_DROP(fileInput);
			TE_SAFE_DROP(fileOutput);
		}

		void teDecodeFile(const teString & from, const teString & to, u32 chunkSize, c8 * chunkInputBuffer, u32 chunkInputBufferSize, c8 * chunkOutputBuffer, u32 chunkOutputBufferSize, u1 localPath)
		{
			TE_ASSERT(chunkInputBufferSize >= LZ4_compressBound(chunkSize));
			TE_ASSERT(chunkOutputBufferSize >= chunkSize);

			IBuffer * fileInput = GetFileManager()->OpenFile(from, CFileBuffer::FWM_READ, localPath);

			if(!fileInput)
				return;

			IBuffer * fileOutput = GetFileManager()->OpenFile(to, CFileBuffer::FWM_WRITE, localPath);

			if(!fileOutput)
			{
				TE_SAFE_DROP(fileInput);
				return;
			}

			fileInput->SetStreamMode(true);
			fileOutput->SetStreamMode(true);

			fileInput->Lock(BLT_READ);
			fileInput->SetPosition(0);

			u32 magicNumber = u32Max;
			fileInput->Read(&magicNumber, ARCHIVE_MAGICNUMBER_SIZE);
			TE_ASSERT(magicNumber == ARCHIVE_MAGICNUMBER);

			fileOutput->Lock(BLT_WRITE);
			fileOutput->SetPosition(0);

			c8 * inBuffer = chunkInputBuffer;
			c8 * outBuffer = chunkOutputBuffer;

			while(true)
			{
				if((fileInput->GetSize() - fileInput->GetPosition()) < ARCHIVE_MAGICNUMBER_SIZE)
					break;

				u32 chunkSizeInFile;
				fileInput->Read(&chunkSizeInFile, sizeof(u32));

				if(chunkSizeInFile == ARCHIVE_MAGICNUMBER)
					continue;

				fileInput->Read(inBuffer, chunkSizeInFile);

				s32 outputSize = LZ4_uncompress_unknownOutputSize(inBuffer, outBuffer, chunkSizeInFile, chunkSize);

				if(outputSize < 0)
				{
					TE_LOG_ERR("lz4 decoding failed, corrupted input\n");

					fileInput->Unlock();
					fileOutput->Unlock();

					TE_SAFE_DROP(fileInput);
					TE_SAFE_DROP(fileOutput);

					return;
				}

				fileOutput->Write(outBuffer, (u32)outputSize);
			}

			fileInput->Unlock();
			fileOutput->Unlock();

			TE_SAFE_DROP(fileInput);
			TE_SAFE_DROP(fileOutput);
		}

		void teCompressFile(const teString & from, const teString & to, u1 highCompression, u1 localPath)
		{
			c8 * inBuffer = (c8*)TE_ALLOCATE(CHUNKSIZE);
			c8 * outBuffer = (c8*)TE_ALLOCATE(LZ4_compressBound(CHUNKSIZE));

			teCompressFile(from, to, CHUNKSIZE, inBuffer, CHUNKSIZE, outBuffer, LZ4_compressBound(CHUNKSIZE), highCompression, localPath);

			TE_FREE(inBuffer);
			TE_FREE(outBuffer);
		}

		void teDecodeFile(const teString & from, const teString & to, u1 localPath)
		{
			c8 * inBuffer = (c8*)TE_ALLOCATE(LZ4_compressBound(CHUNKSIZE));
			c8 * outBuffer = (c8*)TE_ALLOCATE(CHUNKSIZE);

			teDecodeFile(from, to, CHUNKSIZE, inBuffer, LZ4_compressBound(CHUNKSIZE), outBuffer, CHUNKSIZE, localPath);

			TE_FREE(inBuffer);
			TE_FREE(outBuffer);
		}
	}
}
