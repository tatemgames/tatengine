/*
 *  teContentPack.cpp
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 12/1/11.
 *  Copyright 2011 Tatem Games. All rights reserved.
 *
 */

#include "teContentPack.h"
#include "teFileManager.h"
#include "teContentTools.h"
#include "teImageTools.h"
#include "teRender.h"
#include "teLogManager.h"
#include "teFontTools.h"
#include "teBufferSecurity.h"

//#define TE_TEXTURE_MMAP
//#define TE_TEXTURE_MMAP_SAVE_IMAGE

#if defined(TE_TEXTURE_MMAP)

#if defined(TE_PLATFORM_WIN)

#include "tePlatform.h"

#include <windows.h>

typedef struct
{
	HANDLE f;
	HANDLE m;
	void * p;
} SIMPLE_UNMMAP;

void * simple_mmap(const char * filename, int * length, SIMPLE_UNMMAP * un)
{
	HANDLE f = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ,  NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	HANDLE m;
	void *p;
	if(!f) return NULL;
	m = CreateFileMapping(f, NULL, PAGE_READONLY, 0,0, NULL);
	if(!m) { CloseHandle(f); return NULL; }
	p = MapViewOfFile(m, FILE_MAP_READ, 0,0,0);
	if(!p) { CloseHandle(m); CloseHandle(f); return NULL; }
	if(length) *length = GetFileSize(f, NULL);
	if(un)
	{
		un->f = f;
		un->m = m;
		un->p = p;
	}
	return p;
}

void simple_unmmap(SIMPLE_UNMMAP * un)
{
	UnmapViewOfFile(un->p);
	CloseHandle(un->m);
	CloseHandle(un->f);
}

#else

#include "tePlatform.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

typedef struct
{
	int fd;
	int size;
	void * p;
} SIMPLE_UNMMAP;

void * simple_mmap(const char * filename, int * length, SIMPLE_UNMMAP * un)
{
	int fd = open(filename, O_RDONLY);

	if(fd < 0)
		return NULL;

	fcntl(fd, F_NOCACHE, 1);
	fcntl(fd, F_RDAHEAD, 1);

	struct stat statbuf;

	if(fstat(fd, &statbuf) < 0)
		return NULL;

	void * p = mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, fd, 0);

	if(length)
		*length = statbuf.st_size;

	if(un)
	{
		un->fd = fd;
		un->size = statbuf.st_size;
		un->p = p;
	}

	return p;
}

void simple_unmmap(SIMPLE_UNMMAP * un)
{
	munmap(un->p, un->size);
	close(un->fd);
}

#endif

#endif

namespace te
{
	namespace scene
	{
		teContentPack::teContentPack()
			:finalized(false)
		{
		}

		teContentPack::~teContentPack()
		{
			Clear();
		}

		void teContentPack::Save(core::IBuffer * buffer)
		{
			TE_ASSERT(!finalized);

			u32 size = 4;

			#ifdef TE_CONTENT_STRINGS_ENCR
				size += sizeof(u32);
			#endif

			size += texturesData.GetSystemSize();
			size += materials.GetSystemSize();
			size += frameBuffers.GetSystemSize();
			size += surfaceLayers.GetSystemSize();
			size += surfaceData.GetSystemSize();
			size += surfaceAABB.GetSystemSize();
			size += skeletonData.GetSystemSize();
			size += atlasSprites.GetSystemSize();
			size += fontData.GetSystemSize();
			size += stringsData.GetSystemSize();
			size += soundsData.GetSystemSize();

			buffer->Lock(core::BLT_WRITE);
			buffer->Allocate(size);
			buffer->SetPosition(0);

			#ifdef TE_CONTENT_STRINGS_ENCR
				buffer->Write("TECE", 4);
			#else
				buffer->Write("TECP", 4);
			#endif

			#ifdef TE_CONTENT_STRINGS_ENCR
				stringsDataCRC = core::Encrypt((u8*)stringsData.GetPool(), stringsData.GetAlive(), TE_CONTENT_STRINGS_ENCR_KEY);
				buffer->Write(&stringsDataCRC, sizeof(u32));
			#endif

			texturesData.Save(buffer);
			materials.Save(buffer);
			frameBuffers.Save(buffer);
			surfaceLayers.Save(buffer);
			surfaceData.Save(buffer);
			surfaceAABB.Save(buffer);
			skeletonData.Save(buffer);
			atlasSprites.Save(buffer);
			fontData.Save(buffer);
			stringsData.Save(buffer);
			soundsData.Save(buffer);

			buffer->Unlock();

			#ifdef TE_CONTENT_STRINGS_ENCR
				core::Decrypt((u8*)stringsData.GetPool(), stringsData.GetAlive(), TE_CONTENT_STRINGS_ENCR_KEY, stringsDataCRC);
			#endif
		}

		void teContentPack::Load(core::IBuffer * buffer)
		{
			buffer->Lock(core::BLT_READ);
			buffer->SetPosition(0);

			c8 temp[4];
			buffer->Read(temp, 4);

			#ifdef TE_CONTENT_STRINGS_ENCR
				u1 encrypted = false;

				if(memcmp(temp, "TECE", 4) == 0)
				{
					encrypted = true;
					buffer->Read(&stringsDataCRC, sizeof(u32));
				}
				else if(memcmp(temp, "TECP", 4) == 0)
				{
					stringsDataCRC = 0;
				}
				else
				{
					TE_LOG_ERR("failed to load content pack - unknown format");
				}
			#else
				if(memcmp(temp, "TECP", 4) == 0)
				{
					stringsDataCRC = 0;
				}
				else
				{
					TE_LOG_ERR("failed to load content pack - unknown format");
				}
			#endif

			texturesData.Load(buffer);
			materials.Load(buffer);
			frameBuffers.Load(buffer);
			surfaceLayers.Load(buffer);
			surfaceData.Load(buffer);
			surfaceAABB.Load(buffer);
			skeletonData.Load(buffer);
			atlasSprites.Load(buffer);
			fontData.Load(buffer);
			stringsData.Load(buffer);
			soundsData.Load(buffer);

			#ifdef TE_CONTENT_STRINGS_ENCR
				if(encrypted)
					core::Decrypt((u8*)stringsData.GetPool(), stringsData.GetAlive(), TE_CONTENT_STRINGS_ENCR_KEY, stringsDataCRC);
			#endif

			buffer->Unlock();

			finalized = false;
		}

		void teContentPack::Clear()
		{
			for(u32 i = 0; i < frameBuffers.GetAlive(); ++i)
				frameBuffers[i].Deinit();

			for(u32 i = 0; i < textures.GetAlive(); ++i)
				textures[i].Deinit();

			for(u32 i = 0; i < soundsData.GetAlive(); ++i)
				soundsData[i].Deinit();

			textures.Clear();

			texturesData.Clear();
			materials.Clear();
			frameBuffers.Clear();
			surfaceLayers.Clear();
			surfaceData.Clear();
			surfaceAABB.Clear();
			skeletonData.Clear();
			atlasSprites.Clear();
			fontData.Clear();
			stringsData.Clear();
			soundsData.Clear();

			surfaceIndexes.Clear();

			finalized = false;
		}

		void teContentPack::Finalize()
		{
			video::GetRender()->RestartCache();

			if(texturesData.GetAlive())
			{
				u32 texturesCount = 0, poolPosition = 0;

				while(poolPosition < texturesData.GetAlive())
				{
					const teTextureInfo * info = reinterpret_cast<const teTextureInfo*>(texturesData.At(poolPosition));
					poolPosition += info->GetDataSize();
					++texturesCount;
				}

				textures.Reserve(texturesCount);
				poolPosition = 0;

				while(poolPosition < texturesData.GetAlive())
				{
					teTextureInfo * info = reinterpret_cast<teTextureInfo*>(texturesData.At(poolPosition));
					poolPosition += info->GetDataSize();

					u32 textureIndex = u32Max;
					video::teTexture & texture = *textures.Request(&textureIndex);

					if(info->renderBufferType != video::RBT_NONE)
					{
						u1 needResetup = false;
						teVector2di contextSize = video::GetRender()->GetContext()->GetSize();

						if(info->renderBufferSize.x == u16Max)
						{
							#ifdef TE_PLATFORM_IPHONE
								info->renderBufferSize.x = contextSize.y;
							#else
								info->renderBufferSize.x = contextSize.x;
							#endif
							needResetup = true;
						}

						if(info->renderBufferSize.y == u16Max)
						{
							#ifdef TE_PLATFORM_IPHONE
								info->renderBufferSize.y = contextSize.x;
							#else
								info->renderBufferSize.y = contextSize.y;
							#endif
							needResetup = true;
						}

						if(needResetup)
						{
							for(u32 i = 0; i < atlasSprites.GetAlive(); ++i)
							{
								if(atlasSprites[i].textureIndex == textureIndex)
								{
									atlasSprites[i].size = info->renderBufferSize;
									atlasSprites[i].origin = atlasSprites[i].size / 2;
								}
							}
						}

						//texture.Init(video::TT_RENDERBUFFER, info->flags, NULL, (video::ERenderBufferType)info->renderBufferType, info->renderBufferSize);
						texture.Init(video::TT_2D, info->flags, NULL, (video::ERenderBufferType)info->renderBufferType, info->renderBufferSize);
					}
					else
					{
						#if defined(TE_TEXTURE_MMAP)

						SIMPLE_UNMMAP mmap;
						s32 mmapLength;

						#ifdef TE_PLATFORM_IPHONE
							teString fn = core::GetLogManager()->GetConcate().Add(core::GetPlatform()->GetFileSystem()->GetPath(core::FPT_RESOURCES).c_str()).Add(info->name.ToString().c_str()).Add(".img").BakeToString();
						#else
							teString fn = core::GetLogManager()->GetConcate().Add(core::GetPlatform()->GetFileSystem()->GetPath(core::FPT_USER_RESOURCES).c_str()).Add(info->name.ToString().c_str()).Add(".img").BakeToString();
						#endif

						simple_mmap(fn.c_str(), &mmapLength, &mmap);

						TE_LOG("ok to mmap open %s", info->name.ToString().c_str());

						texture.Init(video::TT_2D, info->flags, (image::teImage*)mmap.p);

						simple_unmmap(&mmap);

						#else

						core::IBuffer * buffer = core::GetFileManager()->OpenFile(info->name.ToString());

						if(!buffer)
						{
							// ------------------------- TODO test code
							// if we cannot load tga file, try to check extension to png
							memcpy((c8*)info->name.data + info->name.size - 4, "png", 3);
							buffer = core::GetFileManager()->OpenFile(info->name.ToString());
							// ------------------------- TODO test code
						}

						if(!buffer)
						{
							// ------------------------- TODO test code
							// if we cannot load tga file, try to check extension to pvr
							memcpy((c8*)info->name.data + info->name.size - 4, "pvr", 3);
							buffer = core::GetFileManager()->OpenFile(info->name.ToString());
							// ------------------------- TODO test code
						}

						if(!buffer)
							continue;

						image::teImage * image = NULL;

						if(strstr(info->name.ToString().c_str(), ".png"))
							image = image::LoadPNG(buffer);
						else if(strstr(info->name.ToString().c_str(), ".tga"))
							image = image::LoadTGA(buffer);
						else if(strstr(info->name.ToString().c_str(), ".pvr"))
							image = image::LoadPVR(buffer);

						//TE_LOG("ok to open %s", info->name.ToString().c_str());

						TE_ASSERT(image);

						TE_SAFE_DROP(buffer);

						#ifdef TE_TEXTURE_MMAP_SAVE_IMAGE
						buffer = core::GetFileManager()->OpenFile(core::GetLogManager()->GetConcate().Add(info->name.ToString().c_str()).Add(".img").BakeToString(), core::CFileBuffer::FWM_WRITE);
						TE_ASSERT(buffer);
						buffer->Lock(core::BLT_WRITE);
						buffer->Allocate(image->GetImageDataSize());
						buffer->Write(image, image->GetImageDataSize());
						buffer->Unlock();
						TE_SAFE_DROP(buffer);
						#endif

						texture.Init(video::TT_2D, info->flags, image);
						TE_FREE(image);

						#endif
					}
				}

				//texturesData.Clear();
			}

			if(frameBuffers.GetAlive())
			{
				for(u32 i = 0; i < frameBuffers.GetAlive(); ++i)
				{
					u32 temp[video::FBL_MAX];

					for(u8 j = 0; j < video::FBL_MAX; ++j)
						temp[j] = (u32)(teptr_t)frameBuffers[i].textures[j];

					frameBuffers[i].Init();
					frameBuffers[i].Bind();

					for(u8 j = 0; j < video::FBL_MAX; ++j)
						if(temp[j] != u32Max)
							frameBuffers[i].SetTexture((video::EFrameBufferLayer)j, textures.At(temp[j]));

					if(!frameBuffers[i].IsValid())
						TE_LOG_ERR("frameBuffer %i is invalid", i);

					if((i + 1) == frameBuffers.GetAlive()) // unbind last framebuffer
						frameBuffers[i].UnBind();
				}
			}

			#ifdef TE_CONTENT_FIX_FONTS
			if(fontData.GetAlive())
			{
				u32 poolPosition = 0;

				while(poolPosition < fontData.GetAlive())
				{
					font::teFont * font = reinterpret_cast<font::teFont*>(fontData.At(poolPosition));
					poolPosition += font->GetFontDataSize();
					SortFontData(font);
				}
			}
			#endif

			if(soundsData.GetAlive())
			{
				for(u32 i = 0; i < soundsData.GetAlive(); ++i)
				{
					sound::teSound & sound = soundsData[i];
					sound::GetSoundManager()->InitSound(sound);
				}
			}

			u32 surfaceCount = 0;
			for(u32 surfaceOffset = 0; surfaceOffset < surfaceData.GetAlive();)
			{
				video::teSurfaceData * data = reinterpret_cast<video::teSurfaceData*>(surfaceData.At(surfaceOffset));
				data->flags |= video::SDF_WAS_CHANGED;
				surfaceOffset += sizeof(video::teSurfaceData) + data->dataSize;
				++surfaceCount;
			}

			if(surfaceCount)
			{
				surfaceIndexes.Reserve(surfaceCount);

				for(u32 surfaceOffset = 0; surfaceOffset < surfaceData.GetAlive();)
				{
					*surfaceIndexes.Request() = surfaceOffset;
					surfaceOffset += sizeof(video::teSurfaceData) + reinterpret_cast<video::teSurfaceData*>(surfaceData.At(surfaceOffset))->dataSize;
				}
			}

			finalized = true;
		}

		void teContentPack::Invalidate()
		{
			for(u32 i = 0; i < frameBuffers.GetAlive(); ++i)
			{
				for(u8 j = 0; j < video::FBL_MAX; ++j)
					if(frameBuffers[i].textures[j])
						frameBuffers[i].textures[j] = (video::teTexture*)(teptr_t)textures.GetIndexInArray(frameBuffers[i].textures[j]);

				frameBuffers[i].Deinit();
			}

			for(u32 i = 0; i < textures.GetAlive(); ++i)
				textures[i].Deinit();

			for(u32 i = 0; i < soundsData.GetAlive(); ++i)
					soundsData[i].Deinit();

			textures.Clear();

			surfaceIndexes.Clear();

			finalized = false;
		}

		void teContentPack::UpdateSurfaceAABB(u32 surfaceIndex, u32 surfaceOffset, u1 updateSkeleton)
		{
			if((surfaceIndex != u32Max) && (surfaceOffset != u32Max))
			{
				video::teSurfaceData * data = reinterpret_cast<video::teSurfaceData*>(surfaceData.At(surfaceOffset));

				if(updateSkeleton && (data->skeletonIndex != u32Max))
				{
					teSkeleton * skelData = reinterpret_cast<teSkeleton*>(skeletonData.At(data->skeletonIndex));
					CalculateSurfaceAABBSkeleton(skelData, data, surfaceLayers[data->layersIndex], surfaceAABB[surfaceIndex]);
				}
				else
					CalculateSurfaceAABB(data, surfaceLayers[data->layersIndex], surfaceAABB[surfaceIndex]);
			}
			else
			{
				surfaceIndex = 0;
				surfaceOffset = 0;

				while(surfaceOffset < surfaceData.GetAlive())
				{
					video::teSurfaceData * data = reinterpret_cast<video::teSurfaceData*>(surfaceData.At(surfaceOffset));

					if(updateSkeleton && (data->skeletonIndex != u32Max))
					{
						teSkeleton * skelData = reinterpret_cast<teSkeleton*>(skeletonData.At(data->skeletonIndex));
						CalculateSurfaceAABBSkeleton(skelData, data, surfaceLayers[data->layersIndex], surfaceAABB[surfaceIndex]);
					}
					else
						CalculateSurfaceAABB(data, surfaceLayers[data->layersIndex], surfaceAABB[surfaceIndex]);

					++surfaceIndex;
					surfaceOffset += sizeof(video::teSurfaceData) + data->dataSize;
				}
			}
		}
	};
}
