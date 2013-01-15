/*
 *  teString.cpp
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 11/9/11.
 *  Copyright 2011 Tatem Games. All rights reserved.
 *
 */

#include "teString.h"

namespace te
{
	u32 teUTF8GetSizeWithReplacedEscapeCharacters(const c8 * input, u32 size)
	{
		if(!input)
			return 0;

		const c8 * temp = input;
		u32 result = 0;

		while(size)
		{
			if(*temp == '\\')
			{
				c8 c = *(temp + 1);
				if((c == 'r') || (c == 'n') || (c == '\\') || (c == '\"') || (c == '\''))
				{
					++temp;
					--size;
				}
			}

			++result;
			++temp;
			--size;
		}

		return result ? result + 1 : 0;
	}

	void teUTF8StrCpyReplaceEscapeCharacters(c8 * output, const c8 * input, u32 size)
	{
		if((!input) || (!output))
			return;

		const c8 * i = input;
		c8 * o = output;

		while(size)
		{
			if(*i == '\\')
			{
				c8 c = *(i + 1);

				switch(c)
				{
				case 'r':
					*o = '\r';
					++i;
					--size;
					break;
				case 'n':
					*o = '\n';
					++i;
					--size;
					break;
				case '\\':
					*o = '\\';
					++i;
					--size;
					break;
				case '"':
					*o = '"';
					++i;
					--size;
					break;
				case '\'':
					*o = '\'';
					++i;
					--size;
					break;
				default:
					*o = *i;
					++o;
					--size;
					*o = c;
					break;
				}
			}
			else
				*o = *i;

			++i;
			++o;
			--size;
		}

		*o = '\0';
	}

	void teStringPool::Reserve(u32 size)
	{
		pool.Reserve(size);
	}

	void teStringPool::Clear()
	{
		pool.Clear();
	}

	teString teStringPool::Allocate(u32 size)
	{
		return teString(pool.Request(size), this, size, 0, 0);
	}

	void teStringPool::Allocate(u32 size, u32 & offset)
	{
		pool.Request(size, &offset);
	}

	teString teStringPool::Clone(const teString & string)
	{
		teString result = Allocate(string.GetSize());
		result.SetLength(string.GetLength());
		memcpy(result.GetRaw(), string.GetRawRO(), string.GetSize());
		return result;
	}
}
