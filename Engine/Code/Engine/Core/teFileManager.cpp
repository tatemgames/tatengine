/*
 *  teFileManager.cpp
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 01/22/11.
 *  Copyright 2011 Tatem Games. All rights reserved.
 *
 */

#include "teFileManager.h"
#include "tePlatform.h"
#include "teMath.h"
#include "zlib.h"
#include "unzip.h"

#include <sys/stat.h>
//#include "teImageTools.h"
#include "teLogManager.h"
#ifdef TE_COMPILER_MSVC
	#include "dirent_msvc.h"
#else
	#include <dirent.h>
	#include <unistd.h>
#endif

namespace te
{
	namespace core
	{
		extern "C"
		{
			voidpf ZCALLBACK unzip_fopen_file_func(voidpf opaque, const char * fileName, int mode)
			{
				if(!opaque)
					return 0;

				static_cast<IBuffer*>(opaque)->Lock(core::BLT_READ);
				return opaque;
			}

			uLong ZCALLBACK unzip_fread_file_func(voidpf opaque, voidpf stream, void * buffer, uLong size)
			{
				if(!opaque)
					return 0;

				s32 readSize = teMin((s32)size, (s32)static_cast<IBuffer*>(opaque)->GetSize() - (s32)static_cast<IBuffer*>(opaque)->GetPosition() - 1);

				if(readSize > 0)
					static_cast<IBuffer*>(opaque)->Read(buffer, (u32)readSize);

				return readSize;
			}

			long ZCALLBACK unzip_ftell_file_func(voidpf opaque, voidpf stream)
			{
				if(!opaque)
					return 0;

				return static_cast<IBuffer*>(opaque)->GetPosition();
			}

			long ZCALLBACK unzip_fseek_file_func(voidpf opaque, voidpf stream, uLong offset, int origin)
			{
				if(!opaque)
					return Z_ERRNO;

				if(origin == ZLIB_FILEFUNC_SEEK_END)
					static_cast<IBuffer*>(opaque)->SetPosition(static_cast<IBuffer*>(opaque)->GetSize() - 1);
				else
					static_cast<IBuffer*>(opaque)->SetPosition(offset, origin == ZLIB_FILEFUNC_SEEK_CUR);

				return Z_OK;
			}

			int ZCALLBACK unzip_fclose_file_func(voidpf opaque, voidpf stream)
			{
				if(!opaque)
					return Z_ERRNO;

				static_cast<IBuffer*>(opaque)->Unlock();
				return Z_OK;
			}

			int ZCALLBACK unzip_ferror_file_func(voidpf opaque, voidpf stream)
			{
				if(!opaque)
					return Z_ERRNO;

				return Z_OK;
			}

			void unzip_fill_fopen_filefunc(zlib_filefunc_def * pzlib_filefunc_def)
			{
			}
		}

		teFileManager * fileManager = NULL;

		teFileManager::teFileManager()
		{
			pool.Reserve(8 * 1024);
			pathConcate.SetBuffer(pool.Allocate(512));

			fileManager = this;
		}

		teFileManager::~teFileManager()
		{
			for(u32 i = 0; i < paks.GetAlive(); ++i)
				paks[i].files.Clear();

			fileManager = NULL;
		}

		void teFileManager::AddToLookupPaths(const teString & path, u1 insertAtEnd)
		{
			//u32 existIndex = 0;

			//if(lookupPaths.LinearSearch(path, existIndex))
			//	return;

			// TODO HERE
			//lookupPaths.InsertElement(path, insertAtEnd ? lookupPaths.GetSize() : 0);

			lookupPaths.ReserveMore(1); // TODO
			*lookupPaths.Request() = pool.Clone(path.c_str());
		}

		void teFileManager::AddPak(const teString & pakFileName, u1 localPath)
		{
			IBuffer * pakBuffer = core::GetFileManager()->OpenFile(pakFileName, CFileBuffer::FWM_READ, localPath, false);

			if(!pakBuffer)
				return;

			zlib_filefunc_def pzlib_filefunc_def;
			pzlib_filefunc_def.zopen_file = unzip_fopen_file_func;
			pzlib_filefunc_def.zread_file = unzip_fread_file_func;
			pzlib_filefunc_def.zwrite_file = NULL;
			pzlib_filefunc_def.ztell_file = unzip_ftell_file_func;
			pzlib_filefunc_def.zseek_file = unzip_fseek_file_func;
			pzlib_filefunc_def.zclose_file = unzip_fclose_file_func;
			pzlib_filefunc_def.zerror_file = unzip_ferror_file_func;
			pzlib_filefunc_def.opaque = pakBuffer;

			unzFile unzipFile = unzOpen2(NULL, &pzlib_filefunc_def);

			unz_global_info globalInfo;
			s32 errorCode = unzGetGlobalInfo(unzipFile, &globalInfo);

			unzGoToFirstFile(unzipFile);

			paks.ReserveMore(1); // TODO
			paks.Request();
			paks.GetLast().fileName = pakFileName;
			paks.GetLast().localPath = localPath;

			for(u32 i = 0; i < globalInfo.number_entry; ++i)
			{
				c8 fileName[256];
				unz_file_info fileInfo;
				errorCode = unzGetCurrentFileInfo(unzipFile, &fileInfo, fileName, sizeof(fileName), NULL, 0, NULL, 0);

				c8 * fileNameWithoutPath = fileName;

				for(s16 j = (s16)(fileInfo.size_filename - 1); j >= 0; --j)
				{
					if((fileName[j] == '/') || (fileName[j] == '\\'))
					{
						fileNameWithoutPath = fileName + j + 1;
						break;
					}
				}

				if(*fileNameWithoutPath != '\0')
				{
					unzOpenCurrentFile(unzipFile);
					paks.GetLast().files.ReserveMore(1); // TODO
					*paks.GetLast().files.Request() = tePakFile(pool.Clone(fileName), unzGetDataOffset(unzipFile), fileInfo.uncompressed_size);
					unzCloseCurrentFile(unzipFile);
				}

				if(i + 1 < globalInfo.number_entry)
					errorCode = unzGoToNextFile(unzipFile);
			}

			unzClose(unzipFile);

			pakBuffer->Drop();
		}

		IBuffer * teFileManager::OpenFile(const teString & fileName, CFileBuffer::EFileWorkMode mode, u1 localPath, u1 checkPaks, u32 offset, u32 size)
		{
			if(GetPlatform()->GetFileSystem())
			{
				if(localPath)
				{
					if(checkPaks)
						for(s32 i = paks.GetSize() - 1; i >= 0; --i)
							for(s32 j = 0; j < (s32)paks[i].files.GetSize(); ++j)
								if(paks[i].files[j].fileName.IsEqual(fileName))
									return OpenFile(paks[i].fileName, mode, paks[i].localPath, false, paks[i].files[j].offset + offset, (size != (u32)-1) ? size : paks[i].files[j].size);

					for(u32 i = 0; i < lookupPaths.GetSize(); ++i)
					{
						teString temptemp = pathConcate.Add(lookupPaths[i].c_str()).Add(fileName.c_str()).BakeToString();
						//printf("try open file %s\n", temptemp.c_str());

						IBuffer * buffer = GetPlatform()->GetFileSystem()->OpenFile(temptemp, mode, offset, size);

						if(buffer)
							return buffer;
					}

					return NULL;
				}
				else
					return GetPlatform()->GetFileSystem()->OpenFile(fileName, mode, offset, size);
			}
			else
				return NULL;
		}

		u1 teFileManager::UnZipFile(const teString & zipFileName)
		{
			// Remove previous version
			DIR * dp;
			struct dirent*  ep;
			dp = opendir(GetLogManager()->GetConcate().Add(core::GetPlatform()->GetFileSystem()->GetPath(core::FPT_RESOURCES_UNPACKED).c_str()).BakeToString().c_str());

			if(dp)
			{
				while((ep = readdir(dp)) != NULL)
				{
					unlink(GetLogManager()->GetConcate().Add(core::GetPlatform()->GetFileSystem()->GetPath(core::FPT_RESOURCES_UNPACKED).c_str()).Add(ep->d_name).BakeToString().c_str());
				}

				closedir(dp);

				#ifdef TE_COMPILER_MSVC
					RemoveDirectoryA(GetLogManager()->GetConcate().Add(core::GetPlatform()->GetFileSystem()->GetPath(core::FPT_RESOURCES_UNPACKED).c_str()).BakeToString().c_str());
				#else
					rmdir(GetLogManager()->GetConcate().Add(core::GetPlatform()->GetFileSystem()->GetPath(core::FPT_RESOURCES_UNPACKED).c_str()).BakeToString().c_str());
				#endif
			}

			IBuffer * zipBuffer = core::GetFileManager()->OpenFile(zipFileName, CFileBuffer::FWM_READ, true, false);

			if(!zipBuffer)
				return false;

			zlib_filefunc_def pzlib_filefunc_def;
			pzlib_filefunc_def.zopen_file = unzip_fopen_file_func;
			pzlib_filefunc_def.zread_file = unzip_fread_file_func;
			pzlib_filefunc_def.zwrite_file = NULL;
			pzlib_filefunc_def.ztell_file = unzip_ftell_file_func;
			pzlib_filefunc_def.zseek_file = unzip_fseek_file_func;
			pzlib_filefunc_def.zclose_file = unzip_fclose_file_func;
			pzlib_filefunc_def.zerror_file = unzip_ferror_file_func;
			pzlib_filefunc_def.opaque = zipBuffer;

			unzFile unzipFile = unzOpen2(NULL, &pzlib_filefunc_def);

			unz_global_info globalInfo;
			s32 errorCode = unzGetGlobalInfo(unzipFile, &globalInfo);

			unzGoToFirstFile(unzipFile);

			u8 * tempBuffer;
			tempBuffer = (u8*)TE_ALLOCATE(1024*1024);

			#ifdef TE_COMPILER_MSVC
				CreateDirectoryA(GetLogManager()->GetConcate().Add(core::GetPlatform()->GetFileSystem()->GetPath(core::FPT_RESOURCES_UNPACKED).c_str()).BakeToString().c_str(), NULL);
			#else
				#ifdef TE_PLATFORM_WIN
					mkdir(GetLogManager()->GetConcate().Add(core::GetPlatform()->GetFileSystem()->GetPath(core::FPT_RESOURCES_UNPACKED).c_str()).BakeToString().c_str());
				#else
					mkdir(GetLogManager()->GetConcate().Add(core::GetPlatform()->GetFileSystem()->GetPath(core::FPT_RESOURCES_UNPACKED).c_str()).BakeToString().c_str(), 0755);
				#endif
			#endif

			for(u32 i = 0; i < globalInfo.number_entry; ++i)
			{
				c8 fileName[256];
				unz_file_info fileInfo;
				errorCode = unzGetCurrentFileInfo(unzipFile, &fileInfo, fileName, sizeof(fileName), NULL, 0, NULL, 0);

				c8 * fileNameWithoutPath = fileName;

				for(s16 j = (s16)(fileInfo.size_filename - 1); j >= 0; --j)
				{
					if((fileName[j] == '/') || (fileName[j] == '\\'))
					{
						fileNameWithoutPath = fileName + j + 1;
						break;
					}
				}

				if(*fileNameWithoutPath != '\0')
				{
					c8 * fileExtension = fileNameWithoutPath;

					for(s16 j = (s16)(strlen(fileNameWithoutPath)); j >= 0; --j)
					{
						if(fileNameWithoutPath[j] == '.')
						{
							fileExtension = fileNameWithoutPath + j + 1;
							break;
						}
					}

					unzOpenCurrentFile(unzipFile);

					s32 dataSize = fileInfo.uncompressed_size;

					IBuffer * zipFileBuffer = core::GetFileManager()->OpenFile(zipFileName, CFileBuffer::FWM_READ, true, false, unzGetDataOffset(unzipFile), fileInfo.uncompressed_size);

					if(zipFileBuffer)
					{
//						if(!strcmp(fileExtension, "tga") || !strcmp(fileExtension, "png"))
//						{
//							image::teImage * image = NULL;
//
//							if(!strcmp(fileExtension, "tga"))
//								image = image::LoadTGA(zipFileBuffer);
//							else
//								image = image::LoadPNG(zipFileBuffer);
//
//							printf("ADDFILE1 : %s\n", GetLogManager()->GetConcate().Add(core::GetPlatform()->GetFileSystem()->GetPath(core::FPT_RESOURCES_UNPACKED).c_str()).Add(fileNameWithoutPath).Add(".img").BakeToString().c_str());
//
//							core::IBuffer * rawBuffer = core::GetFileManager()->OpenFile(GetLogManager()->GetConcate().Add(core::GetPlatform()->GetFileSystem()->GetPath(core::FPT_RESOURCES_UNPACKED).c_str()).Add(fileNameWithoutPath).Add(".img").BakeToString().c_str(), core::CFileBuffer::FWM_WRITE, false, false);
//							if(rawBuffer)
//							{
//								rawBuffer->Lock(core::BLT_WRITE);
//								rawBuffer->Allocate(image->GetImageDataSize());
//								rawBuffer->SetPosition(0);
//								rawBuffer->Write(image, image->GetImageDataSize());
//								rawBuffer->Unlock();
//								TE_SAFE_DROP(rawBuffer);
//							}
//
//							TE_FREE(image);
//						}
//						else
						{
							zipFileBuffer->Lock(core::BLT_READ);

							IBuffer * fileBuffer = core::GetFileManager()->OpenFile(GetLogManager()->GetConcate().Add(core::GetPlatform()->GetFileSystem()->GetPath(core::FPT_RESOURCES_UNPACKED).c_str()).Add(fileNameWithoutPath).BakeToString().c_str(), CFileBuffer::FWM_WRITE, false, false, 0, fileInfo.uncompressed_size);
							if(fileBuffer)
							{
								fileBuffer->Lock(core::BLT_WRITE);

								while(dataSize > 0)
								{
									zipFileBuffer->Read(tempBuffer, dataSize > 1024*1024 ? 1024*1024 : dataSize);
									fileBuffer->Write(tempBuffer, dataSize > 1024*1024 ? 1024*1024 : dataSize);

									dataSize -= 1024*1024;
								}

								fileBuffer->Unlock();
								TE_SAFE_DROP(fileBuffer);
							}

							zipFileBuffer->Unlock();
						}
					}

					TE_SAFE_DROP(zipFileBuffer)

					unzCloseCurrentFile(unzipFile);
				}

				if(i + 1 < globalInfo.number_entry)
					errorCode = unzGoToNextFile(unzipFile);
			}

			TE_DELETE(tempBuffer);

			unzClose(unzipFile);

			TE_SAFE_DROP(zipBuffer)

			unlink(GetLogManager()->GetConcate().Add(core::GetPlatform()->GetFileSystem()->GetPath(core::FPT_USER_RESOURCES).c_str()).Add(zipFileName.c_str()).BakeToString().c_str());

			return true;
		}

		teFileManager * GetFileManager()
		{
			return fileManager;
		}
	}
}
