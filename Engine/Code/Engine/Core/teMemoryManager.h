/*
 *  teMemoryManager.h
 *  tatengine
 *
 *  Created by Dmitrii Ivanov on 11/23/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEMEMORYMANAGER_H
#define TE_TEMEMORYMANAGER_H

#include "TatEngineCoreConfig.h"
#include "teTypes.h"

#include <memory.h>
#include <memory>
#include <new>

namespace te
{
	namespace core
	{
		void MemoryManagerInit();
		void MemoryManagerClose();
		
		void AddPointer(void * Pointer, const c8 * FileName, u32 LineNum, u8 Type, u32 Size);
		void RemovePointer(void * Pointer, const c8 * FileName, u32 LineNum, u8 Type);

		void * Allocate(size_t Size);
		void * ReAllocate(void * Pointer, size_t Size);
		void Free(void * Pointer);

		void * Allocate(size_t Size, const c8 * FileName, u32 LineNum);
		void * ReAllocate(void * Pointer, size_t Size, const c8 * FileName, u32 LineNum);
		void Free(void * Pointer, const c8 * FileName, u32 LineNum);

		u32 GetMemoryAllocatedBytes();
		u32 GetMemoryAllocatedDebugBytes();
		void GetMemoryCallsCount(u32 & allocCount, u32 & reallocCount, u32 & freeCount);
		void MemoryUsageAnalyzer();
	}
}

#if defined(TE_DEBUG) && defined(TE_MEMORY_MANAGEMENT)

	#define TE_NEW(__var, __obj) {__var = new __obj; te::core::AddPointer(__var, __FILE__, __LINE__, 0, sizeof(__obj));}
	#define TE_NEW_S(__obj) {void * __ptr = new __obj; te::core::AddPointer(__ptr, __FILE__, __LINE__, 0, sizeof(__obj));}
	#define TE_DELETE(__var) {te::core::RemovePointer(__var, __FILE__, __LINE__, 0); delete(__var);}
	#define TE_NEW_A(__var, __obj, __count) {__var = new __obj [__count]; te::core::AddPointer(__var, __FILE__, __LINE__, 1, sizeof(__obj) * __count);}
	#define TE_NEW_S_A(__obj, __count) {void * __ptr = new __obj [__count]; te::core::AddPointer(__ptr, __FILE__, __LINE__, 1, sizeof(__obj) * __count);}
	#define TE_DELETE_A(__var) {te::core::RemovePointer(__var, __FILE__, __LINE__, 1); delete [] (__var);}
	#define TE_NEW_P(__var, __ptr, __obj) {__var = new (__ptr) __obj; te::core::AddPointer(__ptr, __FILE__, __LINE__, 2, sizeof(__obj));}
	#define TE_NEW_S_P(__ptr, __obj) {new (__ptr) __obj; te::core::AddPointer(__ptr, __FILE__, __LINE__, 2, sizeof(__obj));}
	#define TE_DELETE_P(__var, __destructor_name) {(__var)->__destructor_name; te::core::RemovePointer(__var, __FILE__, __LINE__, 2);}

	#define TE_ALLOCATE(__size) te::core::Allocate(( __size ), __FILE__, __LINE__)
	#define TE_REALLOCATE(__ptr, __size) te::core::ReAllocate(( __ptr ), ( __size ), __FILE__, __LINE__)
	#define TE_FREE(__ptr) te::core::Free(( __ptr ), __FILE__, __LINE__)

#else

	#define TE_NEW(__var, __obj) { __var = new __obj; }
	#define TE_NEW_S(__obj) { new __obj; }
	#define TE_DELETE(__var) { delete(__var); }
	#define TE_NEW_A(__var, __obj, __count) { __var = new __obj [__count]; }
	#define TE_NEW_S_A(__obj, __count) { new __obj [__count]; }
	#define TE_DELETE_A(__var) { delete [] (__var); }
	#define TE_NEW_P(__var, __ptr, __obj) { __var = new (__ptr) __obj; }
	#define TE_NEW_S_P(__ptr, __obj) { new (__ptr) __obj; }
	#define TE_DELETE_P(__var, __destructor_name) { (__var)->__destructor_name; }
	#define TE_ALLOCATE(__size) te::core::Allocate(__size)
	#define TE_REALLOCATE(__ptr, __size) te::core::ReAllocate(__ptr, __size)
	#define TE_FREE(__ptr) te::core::Free(__ptr)

#endif

// get size for alignment allocation
// align must be power of two
#define TE_ALIGN_SIZE(__size, __align) ((__size) + ((__align) - 1))
// get aligned ptr from allocated alignment block
// align must be power of two
#define TE_ALIGN_PTR(__ptr, __align) (((__ptr) + ((__align) - 1)) & ~((__align) - 1))
// get align padding from ptr
#define TE_ALIGN_PAD(__ptr, __align) ((-(__ptr)) & ((__align) - 1))

// allocate alignment block
// example :
// uintptr_t rawmemory;
// f32 * alignedmemory;
// TE_ALLOCATE_ALIGN(alignedmemory, f32, rawmemory, 8 * 4, 4);
#define TE_ALLOCATE_ALIGN(__var, __var_obj, __var_store_raw, __size, __align) \
{ \
	__var_store_raw = (uintptr_t)TE_ALLOCATE(TE_ALIGN_SIZE(__size, __align)); \
	__var = (__var_obj*)TE_ALIGN_PTR(__var_store_raw, __align); \
}

#endif
