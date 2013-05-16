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
		#define TE_LZ4_COMPRESS_BOUND(size) (size + (size / 255) + 16)
		#define TE_LZ4_COMPRESS_DATA_BOUND(size) (size + (size / 255) + 16 + 4)

		void teCompressData(c8 * input, u32 inputSize, c8 * output, u32 outputSize, u32 & resultSize, u1 highCompression = false);
		void teDecodeData(c8 * input, u32 inputSize, c8 * output, u32 outputSize, u32 & resultSize);

		// size requests
		// chunkInputBufferSize >= chunkSize
		// chunkOutputBufferSize >= TE_LZ4_COMPRESS_BOUND(chunkSize)
		void teCompressFile(const teString & from, const teString & to, u32 chunkSize, c8 * chunkInputBuffer, u32 chunkInputBufferSize, c8 * chunkOutputBuffer, u32 chunkOutputBufferSize, u1 highCompression = false, u1 localPath = true);

		// size requests
		// chunkInputBufferSize >= TE_LZ4_COMPRESS_BOUND(chunkSize)
		// chunkOutputBufferSize >= chunkSize)
		void teDecodeFile(const teString & from, const teString & to, u32 chunkSize, c8 * chunkInputBuffer, u32 chunkInputBufferSize, c8 * chunkOutputBuffer, u32 chunkOutputBufferSize, u1 localPath = true);

		void teCompressFile(const teString & from, const teString & to, u1 highCompression = false, u1 localPath = true);
		void teDecodeFile(const teString & from, const teString & to, u1 localPath = true);
	}
}

#endif
