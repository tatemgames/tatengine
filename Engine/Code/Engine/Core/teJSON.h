/*
 *  teJSON.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 08/2/11.
 *  Copyright 2011 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEJSON_H
#define TE_TEJSON_H

#include "teConstArray.h"
#include "teString.h"
#include "IBuffer.h"
#include "teVector2D.h"
#include "teVector3D.h"
#include "teQuaternion.h"
#include "teColor.h"

namespace te
{
	namespace core
	{
		// simple struct for tree node
		struct teJSONToken
		{
		public:
			enum EValueType
			{
				VT_INVALID,
				VT_STRING,
				VT_NUMBER,
				VT_TRUE,
				VT_FALSE,
				VT_NULL,
				VT_SIMPLE,
				VT_OBJECT,
				VT_ARRAY
			};

			EValueType type;
			u32 parentIndex;
			u32 dataOffset;
			u32 childrensBegin, childrensEnd;

			TE_INLINE void Clear()
			{
				type = VT_NULL;
				parentIndex = u32Max;
				childrensBegin = u32Max;
				childrensEnd = u32Max;
			}
		};

		struct teJSONValue;

		// store json tree and data
		// data stored after teJSONPool in memory
		//
		// data storage in teJSONPool aligned in special way
		// float numbers data block aligned to 4 byte ptr
		// so dont try to use teJSONPool manually loaded from file
		struct teJSONPool
		{
			u32 dataSize;
			u32 tokensCount;
			teJSONToken tokens[];

			TE_INLINE teJSONToken & Get(u32 value) {TE_ASSERT(value < tokensCount); return tokens[value];}
			TE_INLINE const teJSONToken & Get(u32 value) const {TE_ASSERT(value < tokensCount); return tokens[value];}

			TE_INLINE c8 * GetData(u32 offset) {TE_ASSERT(offset < dataSize); return reinterpret_cast<c8*>(((uintptr_t)this) + sizeof(teJSONPool) + tokensCount * sizeof(teJSONToken) + offset);}
			TE_INLINE const c8 * GetData(u32 offset) const {TE_ASSERT(offset < dataSize); return reinterpret_cast<const c8*>(((uintptr_t)this) + sizeof(teJSONPool) + tokensCount * sizeof(teJSONToken) + offset);}

			teJSONValue GetRoot();
		};

		// tree iterator that represents tree node
		struct teJSONValue
		{
			teJSONPool * pool;
			u32 position;

			TE_INLINE teJSONValue(teJSONPool * setPool = NULL, u32 setPosition = 0):pool(setPool), position(setPosition) {}
			TE_INLINE teJSONValue(const teJSONValue & other):pool(other.pool), position(other.position) {}

			TE_INLINE u1 IsValid() const {return pool && (position != u32Max);}

			// get token
			TE_INLINE teJSONToken & Get() {TE_ASSERT(pool); return pool->Get(position);}
			TE_INLINE const teJSONToken & Get() const {TE_ASSERT(pool); return pool->Get(position);}

			// next value in tree
			TE_INLINE teJSONValue Next() const
			{
				if(IsSimple() || ((Get().childrensBegin == u32Max) && (Get().childrensEnd == u32Max)))
				{
					u32 value = position;

					while(pool->Get(value).parentIndex != u32Max)
					{
						if(value < pool->Get(pool->Get(value).parentIndex).childrensEnd)
							return teJSONValue(pool, value + 1);
						else
							value = pool->Get(value).parentIndex;
					}

					return teJSONValue(NULL, u32Max);
				}
				else
					return teJSONValue(pool, Get().childrensBegin);
			}

			// get parent
			TE_INLINE teJSONValue Back() const {if(Get().parentIndex != u32Max) return teJSONValue(pool, Get().parentIndex); else return teJSONValue(NULL, u32Max);}

			// check type
			TE_INLINE u1 IsSimple() const	{return (position != u32Max) && (Get().type < teJSONToken::VT_SIMPLE);}
			TE_INLINE u1 IsString() const	{return (position != u32Max) && (Get().type == teJSONToken::VT_STRING);}
			TE_INLINE u1 IsNumber() const	{return (position != u32Max) && (Get().type == teJSONToken::VT_NUMBER);}
			TE_INLINE u1 IsBool() const		{return (position != u32Max) &&  ((Get().type == teJSONToken::VT_TRUE) || (Get().type == teJSONToken::VT_FALSE));}
			TE_INLINE u1 IsNull() const		{return (position == u32Max) || (Get().type == teJSONToken::VT_NULL);}
			TE_INLINE u1 IsObject() const	{return (position != u32Max) && (Get().type == teJSONToken::VT_OBJECT);}
			TE_INLINE u1 IsArray() const	{return (position != u32Max) && (Get().type == teJSONToken::VT_ARRAY);}

			// get node data
			TE_INLINE teString		GetString(teString def = "") const {return IsString() ? teString(pool->GetData(Get().dataOffset)) : def;}
			TE_INLINE f32			GetNumber(f32 def = 0.0f) const {return IsNumber() ? (*(f32*)pool->GetData(Get().dataOffset)) : def;}
			TE_INLINE u1			GetBool(u1 def = false) const {return IsBool() ? (Get().type == teJSONToken::VT_TRUE) : def;}
			TE_INLINE f32			GetF32(f32 def = 0.0f)	const {return IsNumber() ? (f32)GetNumber() : def;}
			TE_INLINE s32			GetS32(s32 def = 0)		const {return IsNumber() ? (s32)GetNumber() : def;}
			TE_INLINE u32			GetU32(u32 def = 0)		const {return IsNumber() ? (u32)GetNumber() : def;}
			TE_INLINE s64			GetS64(s32 def = 0)		const {return IsNumber() ? (s64)GetNumber() : def;}
			TE_INLINE u64			GetU64(u32 def = 0)		const {return IsNumber() ? (u64)GetNumber() : def;}
			TE_INLINE u1			GetU1 (u1  def = false)	const {return GetBool(def);}

			// object and array functions
			TE_INLINE u32 GetArrayValuesCount() const {return (Get().childrensBegin == u32Max) ? 0 : ((Get().childrensEnd - Get().childrensBegin + 1) / (IsObject() ? 2 : 1));}
			TE_INLINE teJSONValue GetArrayValue(u32 index) const {return teJSONValue(pool, Get().childrensBegin + (IsObject() ? index * 2 + 1 : index));}
			TE_INLINE teJSONValue GetKeyValue(const teString & key) const
			{
				TE_ASSERT(IsObject());

				for(u32 i = Get().childrensBegin; i <= Get().childrensEnd; i += 2)
				{
					TE_ASSERT(pool->Get(i).type == teJSONToken::VT_STRING);

					if(teString(pool->GetData(pool->Get(i).dataOffset)).IsEqual(key))
						return teJSONValue(pool, i + 1);
				}

				return teJSONValue(NULL, u32Max);
			}

			// get access to keys like this : "name":[1,2,3,4]
			TE_INLINE const f32 * GetNumbersArray() const {TE_ASSERT(IsArray()); return (f32*)pool->GetData(pool->Get(Get().childrensBegin).dataOffset);}
			teVector2df GetVector2df() const {const f32 * v = GetNumbersArray(); return teVector2df(v[0], v[1]);}
			TE_INLINE teVector3df GetVector3df() const {const f32 * v = GetNumbersArray(); return teVector3df(v[0], v[1], v[2]);}
			TE_INLINE teQuaternionf GetQuaternionf() const {const f32 * v = GetNumbersArray(); return teQuaternionf(v[0], v[1], v[2], v[3]);}
			TE_INLINE teColor4f GetColor4f() const {const f32 * v = GetNumbersArray(); return teColor4f(v[0], v[1], v[2], v[3]);}
			TE_INLINE teColor4u GetColor4u() const {const f32 * v = GetNumbersArray(); return teColor4f((f32)v[0], (f32)v[1], (f32)v[2], (f32)v[3]);}
		};

		teJSONPool * ParseJSON(const teString & text, c8 * nestedBuffer = NULL, u32 nestedBufferSize = 0);
		teJSONPool * ParseJSON(core::IBuffer * buffer, c8 * nestedBuffer = NULL, u32 nestedBufferSize = 0);
	}
}

#endif
