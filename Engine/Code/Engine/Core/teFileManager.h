/*
 *  teFileManager.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 01/22/11.
 *  Copyright 2011 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEFILEMANAGER_H
#define TE_TEFILEMANAGER_H

#include "teConstArray.h"
#include "teString.h"
#include "IBuffer.h"
#include "CFileBuffer.h"

namespace te
{
	namespace core
	{
		class teFileManager
		{
		public:
			typedef teConstArray<teString> LookupPathsArrayType;

			struct tePakFile
			{
				teString fileName;
				u32 offset;
				u32 size;

				tePakFile()
					:offset(0), size(0)
				{
				}

				tePakFile(const teString & setFileName, u32 setOffset, u32 setSize)
					:fileName(setFileName), offset(setOffset), size(setSize)
				{
				}
			};

			struct tePak
			{
				teString fileName;
				u1 localPath;
				teConstArray<tePakFile> files;
			};

			typedef teConstArray<tePak> PaksArrayType;

			teFileManager();
			~teFileManager();

			LookupPathsArrayType & GetLookupPaths() {return lookupPaths;}
			const LookupPathsArrayType & GetLookupPaths() const {return lookupPaths;}

			void AddToLookupPaths(const teString & path, u1 insertAtEnd = true);

			PaksArrayType & GetPaks() {return paks;}
			const PaksArrayType & GetPaks() const {return paks;}
			
			teStringConcate & GetPathConcate() {return pathConcate;}

			void AddPak(const teString & pakFileName, u1 localPath = true);

			IBuffer * OpenFile(const teString & fileName, CFileBuffer::EFileWorkMode mode = CFileBuffer::FWM_READ, u1 localPath = true, u1 checkPaks = true, u32 offset = 0, u32 size = u32Max);
			
			u1 UnZipFile(const teString & zipFileName);

		protected:
			LookupPathsArrayType lookupPaths;
			PaksArrayType paks;
			teStringPool pool;
			teStringConcate pathConcate;
		};
		
		//! Get File Manager
		teFileManager * GetFileManager();
	}
}

#endif
