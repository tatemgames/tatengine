/*
 *  teMemoryManager.cpp
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 11/23/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#include "teMemoryManager.h"
#include "teDebug.h"

#ifdef TE_MEMORY_MANAGEMENT
	#include <stdio.h>
	#include <map>
#endif

#include <stdlib.h>

#ifdef TE_MEMORY_USE_NEDMALLOC
	#define NO_MALLINFO 1
	#define NO_NED_NAMESPACE
	#include "nedmalloc.h"
#endif

namespace te
{
	namespace core
	{
		#ifdef TE_MEMORY_MANAGEMENT

			struct MemoryAllocRecord
			{
				const char * fileName;
				u32 lineNum;
				u8 type;
				u32 size;

				MemoryAllocRecord(const char * setFileName, u32 setLineNum, u8 setType, u32 setSize)
					:fileName(setFileName), lineNum(setLineNum), type(setType), size(setSize)
				{
				}
			};
		
			typedef std::map<void*, MemoryAllocRecord> AllocRecordsMapType;

			AllocRecordsMapType allocRecords;
			AllocRecordsMapType allocPlacementRecords;
		#endif

		u32 allocatedPointersCount;
		u32 allocatedPlacementPointersCount;
		u32 memoryAllocCallsCount;
		u32 memoryReallocCallsCount;
		u32 memoryFreeCallsCount;

		void MemoryManagerInit()
		{
			allocatedPointersCount = 0;
			allocatedPlacementPointersCount = 0;
			memoryAllocCallsCount = 0;
			memoryReallocCallsCount = 0;
			memoryFreeCallsCount = 0;
		}
		
		void MemoryManagerClose()
		{
			#if defined(TE_DEBUG) && defined(TE_MEMORY_MANAGEMENT)

				if(allocRecords.size() || allocPlacementRecords.size())
					printf("------------\nTatEngine Memory Manager Log :\n");

				if(allocRecords.size())
					for(AllocRecordsMapType::iterator Iter = allocRecords.begin(); Iter != allocRecords.end(); ++Iter)
						printf("Pointer %p '%s' : %i, %i\n",(*Iter).first, (*Iter).second.fileName, (*Iter).second.lineNum, (*Iter).second.type);

				if(allocPlacementRecords.size())
					for(AllocRecordsMapType::iterator Iter = allocPlacementRecords.begin(); Iter != allocPlacementRecords.end(); ++Iter)
						printf("Placement pointer %p '%s' : %i, %i\n",(*Iter).first, (*Iter).second.fileName, (*Iter).second.lineNum, (*Iter).second.type);
			
				if(allocatedPointersCount)
				{
					printf("------------\n");
					printf("Allocated Pointers Count : %i\n", allocatedPointersCount);
					printf("------------\n");
				}

				if(allocatedPlacementPointersCount)
				{
					printf("------------\n");
					printf("Allocated Placement Pointers Count : %i\n", allocatedPointersCount);
					printf("------------\n");
				}

			#endif

			#ifdef TE_MEMORY_MANAGEMENT
				allocRecords.clear();
				allocPlacementRecords.clear();
			#endif
		}

		void AddPointer(void * Pointer, const c8 * FileName, u32 LineNum, u8 Type, u32 Size)
		{
			#ifdef TE_MEMORY_MANAGEMENT
				if(Type == 2)
				{
					// TODO
					//AllocPlacementRecords.insert(std::pair<void*, MemoryAllocRecord>(Pointer, MemoryAllocRecord(FileName, LineNum, Type, Size)));
				}
				else
					allocRecords.insert(std::pair<void*, MemoryAllocRecord>(Pointer, MemoryAllocRecord(FileName, LineNum, Type, Size)));
			#endif

			if(Type == 2)
				++allocatedPlacementPointersCount;
			else
				++allocatedPointersCount;
		}
		
		void RemovePointer(void * Pointer, const c8 * FileName, u32 LineNum, u8 Type)
		{
			#ifdef TE_MEMORY_MANAGEMENT
				if(Type == 2)
				{
					// TODO
//					AllocRecordsMapType::iterator Iter = AllocPlacementRecords.find(Pointer);

//					if(Iter != AllocPlacementRecords.end())
//						AllocPlacementRecords.erase(Iter);
//					else
//						printf("Not found alloc entry for placement %p '%s' : %i, %i\n", Pointer, FileName, LineNum, Type);
				}
				else
				{
					AllocRecordsMapType::iterator Iter = allocRecords.find(Pointer);

					if(Iter != allocRecords.end())
						allocRecords.erase(Iter);
					else
						printf("Not found alloc entry for %p '%s' : %i, %i\n", Pointer, FileName, LineNum, Type);
				}
			#endif

			if(Type == 2)
				--allocatedPlacementPointersCount;
			else
				--allocatedPointersCount;
		}
		
		void * Allocate(size_t Size)
		{
			#ifdef TE_MEMORY_USE_NEDMALLOC
				void * Pointer = nedmalloc(Size);
			#else
				void * Pointer = malloc(Size);
			#endif
			
			#ifdef TE_MEMORY_MANAGEMENT
				if(!Pointer)
					printf("malloc error, size = %i\n", (s32)Size);
			#endif

			++memoryAllocCallsCount;

			return Pointer;
		}
		
		void * ReAllocate(void * Pointer, size_t Size)
		{
			#ifdef TE_MEMORY_USE_NEDMALLOC
				void * NewPointer = nedrealloc(Pointer, Size);
			#else
				void * NewPointer = realloc(Pointer, Size);
			#endif
			
			#ifdef TE_MEMORY_MANAGEMENT
				if(!NewPointer)
					printf("realloc error, size = %i\n", (s32)Size);
			#endif

			++memoryReallocCallsCount;
			
			return NewPointer;
		}
	
		void Free(void * Pointer)
		{
			TE_ASSERT(Pointer);

			#ifdef TE_MEMORY_USE_NEDMALLOC
				nedfree(Pointer);
			#else
				free(Pointer);
			#endif

			++memoryFreeCallsCount;
		}
		
		void * Allocate(size_t Size, const c8 * FileName, u32 LineNum)
		{
			void * Pointer = Allocate(Size);
			AddPointer(Pointer, FileName, LineNum, 0, Size);
			return Pointer;
		}
		
		void * ReAllocate(void * Pointer, size_t Size, const c8 * FileName, u32 LineNum)
		{
			RemovePointer(Pointer, FileName, LineNum, 0);
			void * NewPointer = ReAllocate(Pointer, Size);
			AddPointer(NewPointer, FileName, LineNum, 0, Size);
			return NewPointer;
		}
		
		void Free(void * Pointer, const c8 * FileName, u32 LineNum)
		{
			RemovePointer(Pointer, FileName, LineNum, 0);
			Free(Pointer);
		}

		u32 GetMemoryAllocatedBytes()
		{
			u32 Size = 0;

			#ifdef TE_MEMORY_MANAGEMENT
				for(AllocRecordsMapType::iterator Iter = allocRecords.begin(); Iter != allocRecords.end(); ++Iter)
					Size += (*Iter).second.size;
			#endif

			return Size;
		}

		u32 GetMemoryAllocatedDebugBytes()
		{
			#ifdef TE_MEMORY_MANAGEMENT
				return (allocRecords.size() + allocPlacementRecords.size()) * (sizeof(MemoryAllocRecord) + sizeof(void*));
			#else
				return 0;
			#endif
		}

		void GetMemoryCallsCount(u32 & allocCount, u32 & reallocCount, u32 & freeCount)
		{
			allocCount = memoryAllocCallsCount;
			reallocCount = memoryReallocCallsCount;
			freeCount = memoryFreeCallsCount;
		}

		//! Memory Usage Analyzer
		void MemoryUsageAnalyzer()
		{
//			#ifdef TE_MEMORY_MANAGEMENT
//				AllocRecordsMapType MemoryData = AllocRecords;
//				std::map<u32, MemoryAllocRecord> MemorySizeData;

//				for(AllocRecordsMapType::iterator Iter = MemoryData.begin(); Iter != MemoryData.end(); ++Iter)
//					MemorySizeData.insert(std::pair<u32, MemoryAllocRecord>((*Iter).second.CurrentSize, (*Iter).second));

//				for(std::map<u32, MemoryAllocRecord>::iterator Iter = MemorySizeData.begin(); Iter != MemorySizeData.end(); ++Iter)
//					printf("Alloc size %u in '%s' (%i)\n",(*Iter).first, (*Iter).second.CurrentFileName, (*Iter).second.CurrentLineNum);
//			#endif
		}
	}
}

