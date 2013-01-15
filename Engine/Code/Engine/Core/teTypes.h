/*
 *  teTypes.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 11/23/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TYPES_H
#define TE_TYPES_H

#include "TatEngineCoreConfig.h"
#include <stdlib.h>
#include <stdint.h>

namespace te
{
	typedef bool u1;

	typedef unsigned char u8;
	const u8 u8Min = 0;
	const u8 u8Max = 0xff;
	
	typedef signed char s8;
	const s8 s8Min = -128;
	const s8 s8Max = 127;
	
	typedef char c8;

	typedef unsigned short u16;
	const u16 u16Min = 0;
	const u16 u16Max = 0xffff;
	
	typedef signed short s16;
	const s16 s16Min = -32768;
	const s16 s16Max = 32767;
	
	#ifdef TE_NATIVE_HALF_FLOAT
	typedef __fp16 f16;
	#else
	typedef u16 f16;
	#endif
	
	typedef unsigned int u32;
	const u32 u32Min = 0;
	const u32 u32Max = 0xffffffff;

	typedef signed int s32;
	const s32 s32Min = -2147483647 - 1;
	const s32 s32Max = 2147483647;

	typedef float f32;
	
	#ifdef TE_COMPILER_MSVC
		typedef unsigned __int64 u64;
	#else
		typedef uint64_t u64;
	#endif
	const u64 u64Min = 0;
	const u64 u64Max = 0xffffffffffffffffU;

	#ifdef TE_COMPILER_MSVC
		typedef __int64 s64;
	#else
		typedef int64_t s64;
	#endif
	const s64 s64Min = -0x7fffffffffffffff - 1;
	const s64 s64Max = 0x7fffffffffffffff;

	typedef double f64;
	
	//#define TE_PTR_T_64_BIT

	#ifdef TE_PTR_T_64_BIT
		typedef u64 teptr_t; // 64 bit pointer for data serialization
		const teptr_t teptr_t_Min = u64Min;
		const teptr_t teptr_t_Max = u64Max;
	#else
		typedef u32 teptr_t; // 32 bit pointer for data serialization
		const teptr_t teptr_t_Min = u32Min;
		const teptr_t teptr_t_Max = u32Max;
	#endif

	#ifndef NULL
		#ifdef TE_COMPILER_MSVC
			#define NULL nullptr
		#else
			#define NULL 0
		#endif
	#endif
	
	//! pod types
	template<typename T> struct teIsPod {enum{value = false};};
	#define TE_DEFINE_POD_TYPE(__type) template<> struct teIsPod <__type> {enum{value = true};};
	TE_DEFINE_POD_TYPE(te::u1);
	TE_DEFINE_POD_TYPE(te::u8);
	TE_DEFINE_POD_TYPE(te::s8);
	TE_DEFINE_POD_TYPE(te::c8);
	TE_DEFINE_POD_TYPE(te::u16);
	TE_DEFINE_POD_TYPE(te::s16);
	TE_DEFINE_POD_TYPE(te::u32);
	TE_DEFINE_POD_TYPE(te::s32);
	TE_DEFINE_POD_TYPE(te::u64);
	TE_DEFINE_POD_TYPE(te::s64);
	TE_DEFINE_POD_TYPE(te::f32);
	TE_DEFINE_POD_TYPE(te::f64);
	#define TE_IS_POD(T) (te::teIsPod<T>::value)

}

#endif
