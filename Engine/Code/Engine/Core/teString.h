/*
 *  teString.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 11/9/11.
 *  Copyright 2011 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEFASTSTRING_H
#define TE_TEFASTSTRING_H

#include "teConstArray.h"
#include <stdarg.h>
#include <stdio.h>

namespace te
{
	/*!\fn u8 teUTF8toUTF32(c8 * utf8, u32 & result)
	*  \brief Convert UTF-8 character to UTF-32 character.
	*  \param[in] input input character string.
	*  \param[out] result UTF-32 character.
	*  \return count of readed chars.
	*/
	TE_FUNC_FI u8 teUTF8toUTF32(const c8 * input, u32 & result)
	{
		if((input[0] & 0xFC) == 0xFC)
		{
			result = ((input[0] & 0x01) << 30) | ((input[1] & 0x3F) << 24) | ((input[2] & 0x3F) << 18) | ((input[3] & 0x3F) << 12) | ((input[4] & 0x3F) << 6) | (input[5] & 0x3F);
			return 6;
		}
		else if((input[0] & 0xF8) == 0xF8)
		{
			result = ((input[0] & 0x03) << 24) | ((input[1] & 0x3F) << 18) | ((input[2] & 0x3F) << 12) | ((input[3] & 0x3F) << 6) | (input[4] & 0x3F);
			return 5;
		}
		else if((input[0] & 0xF0) == 0xF0)
		{
			result = ((input[0] & 0x07) << 18) | ((input[1] & 0x3F) << 12) | ((input[2] & 0x3F) << 6) | (input[3] & 0x3F);
			return 4;
		}
		else if((input[0] & 0xE0) == 0xE0)
		{
			result = ((input[0] & 0x0F) << 12) | ((input[1] & 0x3F) << 6) | (input[2] & 0x3F);
			return 3;
		}
		else if((input[0] & 0xC0) == 0xC0)
		{
			result = ((input[0] & 0x1F) << 6) | (input[1] & 0x3F);
			return 2;
		}
		else
		{
			result = input[0];
			return 1;
		}
	}

	/*!\fn u8 teUTF8GetCharacterSize(c8 * utf8)
	*  \brief Return UTF-8 character size in bytes.
	*  \param[in] input input character string.
	*  \return size in bytes.
	*/
	TE_FUNC_FI u8 teUTF8GetCharacterSize(const c8 * input)
	{
		if((input[0] & 0xFC) == 0xFC)
			return 6;
		else if((input[0] & 0xF8) == 0xF8)
			return 5;
		else if((input[0] & 0xF0) == 0xF0)
			return 4;
		else if((input[0] & 0xE0) == 0xE0)
			return 3;
		else if((input[0] & 0xC0) == 0xC0)
			return 2;
		else
			return 1;
	}

	TE_FUNC u32 teUTF8GetLength(const c8 * input)
	{
		if(!input)
			return 0;

		u32 result = 0;
		while(*input != '\0')
		{
			input += teUTF8GetCharacterSize(input);
			++result;
		}
		return result;
	}

	TE_FUNC u32 teUTF8GetSize(const c8 * input)
	{
		if(!input)
			return 0;

		u32 result = strlen(input);
		return result ? result + 1 : 0;
	}

	u32 teUTF8GetSizeWithReplacedEscapeCharacters(const c8 * input, u32 size);
	void teUTF8StrCpyReplaceEscapeCharacters(c8 * output, const c8 * input, u32 size);

	class teString;

	class teStringPool
	{
	public:
		TE_INLINE teStringPool() {}
		TE_INLINE teStringPool(u32 size) {Reserve(size);}

		void Reserve(u32 size);
		void Clear();
		teString Allocate(u32 size);
		void Allocate(u32 size, u32 & offset);
		teString Clone(const teString & string);

		TE_INLINE teConstArray<c8> & GetPool() {return pool;}
		TE_INLINE const teConstArray<c8> & GetPool() const {return pool;}

		TE_INLINE u32 GetSize() const {return (u32)pool.GetSize();}
		TE_INLINE u32 GetAlive() const {return (u32)pool.GetAlive();}

	protected:
		teConstArray<c8> pool;
	};

	class teString
	{
	public:
		TE_INLINE teString(const c8 * stringLiteral = NULL)
			:rawReadOnly(stringLiteral), raw(NULL), owner(NULL), size(0), length(0), version(0)
		{
			size = teUTF8GetSize(rawReadOnly);
			length = teUTF8GetLength(rawReadOnly);
		}

		TE_INLINE teString(c8 * stringData, teStringPool * setOwner, u32 setSize, u32 setLength, u32 setVersion)
			:raw(stringData), rawReadOnly(stringData), owner(setOwner), size(setSize), length(setLength), version(setVersion)
		{
		}

		TE_INLINE teString(const teString & other)
			:rawReadOnly(other.rawReadOnly), raw(other.raw), owner(other.owner), size(other.size), version(other.version)
		{
		}

		TE_INLINE const c8 * c_str() const {return rawReadOnly;}

		TE_INLINE c8 * GetRaw() const {TE_ASSERT(raw); return raw;}
		TE_INLINE const c8 * GetRawRO() const {return rawReadOnly;}

		TE_INLINE void SetSize(u32 setSize) {size = setSize;}
		TE_INLINE u32 GetSize() const {return size;}

		TE_INLINE void SetLength(u32 setLength) {length = setLength;}
		TE_INLINE u32 GetLength() const {return length;}

		TE_INLINE void PushVersion() {++version;}
		TE_INLINE u32 GetVersion() const {return version;}

		u1 IsEqual(const teString & other) const
		{
			if(rawReadOnly == other.rawReadOnly)
			{
				TE_ASSERT(owner == other.owner);
				return version == other.version;
			}
			else if(rawReadOnly && other.rawReadOnly)
				return !strcmp(rawReadOnly, other.rawReadOnly);
			else if(!(rawReadOnly || other.rawReadOnly))
				return true;
			else
				return false;
		}

		u1 operator == (const teString & other) const {return IsEqual(other);}
		u1 operator != (const teString & other) const {return !IsEqual(other);}

	protected:
		const c8 * rawReadOnly; // pointer for read only
		c8 * raw;

		teStringPool * owner;
		u32 size;
		u32 length;
		u32 version;
	};

	class teStringConcate
	{
	public:
		TE_INLINE teStringConcate()
			:freePosition(0)
		{
		}

		TE_INLINE teStringConcate(const teString & buffer)
			:string(buffer), freePosition(0)
		{
		}

		TE_INLINE void SetBuffer(const teString & buffer)
		{
			string = buffer;
			freePosition = 0;
		}

		TE_INLINE teStringConcate & Add(const c8 * formatStr, ...)
		{
			TE_ASSERT_NODEBUG((freePosition + 1) < string.GetSize());
			va_list args;
			va_start(args, formatStr);
			freePosition += vsnprintf(string.GetRaw() + freePosition, string.GetSize() - freePosition, formatStr, args);
			va_end(args);
			TE_ASSERT_NODEBUG((freePosition + 1) <= string.GetSize());
			return *this;
		}

		TE_INLINE teStringConcate & AddVA(const c8 * formatStr, const va_list & args)
		{
			TE_ASSERT_NODEBUG((freePosition + 1) < string.GetSize());
			freePosition += vsnprintf(string.GetRaw() + freePosition, string.GetSize() - freePosition, formatStr, args);
			TE_ASSERT_NODEBUG((freePosition + 1) <= string.GetSize());
			return *this;
		}

		TE_INLINE teString BakeToString()
		{
			string.GetRaw()[freePosition++] = '\0';
			string.PushVersion();

			teString result(string);
			result.SetSize(freePosition);
			result.SetLength(teUTF8GetLength(result.GetRawRO()));
			freePosition = 0;

			return result;
		}

	private:
		teStringConcate(const teStringConcate & other) {}

	protected:
		teString string;
		u32 freePosition;
	};
}

#endif
