/*
 *  teSerializeStream.cpp
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 12/19/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#include "teSerializeStream.h"

namespace te
{
	namespace core
	{
		teSerializeStream::teSerializeStream()
			:mode(SSM_INVALID)
		{
		}

		teSerializeStream::teSerializeStream(void * setBuffer, size_t setSizeInBytes)
			:mode(SSM_INVALID), pool(setBuffer, setSizeInBytes)
		{
		}

		teSerializeStream::~teSerializeStream()
		{
		}

		void teSerializeStream::Serialize(teSerializeDataFunc func, void * userData)
		{
			sectionsCount = 0;
			valuesCount = 0;
			dataSize = 0;
			curSectionIndex = -1;
			curParentIndex = u32Max;

			mode = SSM_WRITE_CALCULATE;
			(*func)(this, userData);

			u32 poolSize = sectionsCount * sizeof(teSection) + valuesCount * sizeof(teValue) + dataSize;

			if(!pool.IsNested())
				pool.Reserve(poolSize);
			pool.Request(poolSize);

			curValueOffset = sectionsCount * sizeof(teSection);
			curDataOffset = curValueOffset + valuesCount * sizeof(teValue);

			mode = SSM_WRITE;
			(*func)(this, userData);
			mode = SSM_INVALID;
		}

		void teSerializeStream::Deserialize(teSerializeDataFunc func, void * userData)
		{
			sectionsCount = 0;
			valuesCount = 0;
			dataSize = 0;
			curSectionIndex = -1;
			curParentIndex = u32Max;

			teSection * firstSection = reinterpret_cast<teSection *>(pool.At(0));
			curValueOffset = (u32)pool.At(firstSection->valueOffset);
			teSection * lastSection = reinterpret_cast<teSection *>(curValueOffset - sizeof(teSection));
			curDataOffset = lastSection->valueOffset + lastSection->valueCount * sizeof(teValue);

			mode = SSM_READ;
			(*func)(this, userData);
			mode = SSM_INVALID;
		}

		u1 teSerializeStream::OpenSection(u16 id)
		{
			teSection * tempWorkSection = NULL;
			teSection * firstSection = NULL;
			teSection * lastSection = NULL;
			u32 i = 0;
			u1 result = 0;

			switch(mode)
			{
			case SSM_INVALID:
				return false;
			case SSM_READ:
					firstSection = reinterpret_cast<teSection *>(pool.At(0));
					lastSection = reinterpret_cast<teSection *>(firstSection->valueOffset - sizeof(teSection));
					i = 0;
					curSectionIndex = -1;
					while(i < firstSection->valueOffset)
					{
						curSectionIndex++;

						tempWorkSection = reinterpret_cast<teSection *>(pool.At(i));
						if ((tempWorkSection->parentIndex == curParentIndex) && (id == tempWorkSection->id))
						{
							result = 1;
							break;
						}

						i += sizeof(teSection);
					}

					if (result)
					{
						curWorkSection = tempWorkSection;
						tempWorkSection = reinterpret_cast<teSection *>(pool.At(i));
						curParentIndex = curSectionIndex;
					}
					break;
			case SSM_WRITE:
					firstSection = reinterpret_cast<teSection *>(pool.At(0));
					lastSection = reinterpret_cast<teSection *>(firstSection->valueOffset - sizeof(teSection));

					++curSectionIndex;

					curWorkSection = reinterpret_cast<teSection *>(pool.At(curSectionIndex * sizeof(teSection)));
					curWorkSection->id = id;
					curWorkSection->parentIndex = curParentIndex;
					curWorkSection->valueCount = 0;
					curWorkSection->valueOffset = curValueOffset;

					curParentIndex = curSectionIndex;

					result = 1;
					break;
			case SSM_WRITE_CALCULATE:
					++sectionsCount;
					result = 1;
					break;
			default:
				return false;
			}

			return result;
		}

		void teSerializeStream::CloseSection()
		{
			switch(mode)
			{
			case SSM_INVALID:
				break;;
			case SSM_READ:
				curParentIndex = curWorkSection->parentIndex;

				if(curWorkSection->parentIndex < u32Max)
					curWorkSection = reinterpret_cast<teSection *>(pool.At(curWorkSection->parentIndex * sizeof(teSection)));
				break;
			case SSM_WRITE:

				memcpy(pool.At(curParentIndex/*curSectionIndex*/ * sizeof(teSection)), curWorkSection, sizeof(teSection));
				curParentIndex = curWorkSection->parentIndex;

				if(curWorkSection->parentIndex < u32Max)
					curWorkSection = reinterpret_cast<teSection *>(pool.At(curWorkSection->parentIndex * sizeof(teSection)));


				break;
			case SSM_WRITE_CALCULATE:
				break;
			default:
				break;
			}
		}

		u1 teSerializeStream::OnValue(u16 id, void * value, u32 size)
		{
			teValue * curWorkValue = NULL;
			u32 tempOffset = 0;
			u32 tDataSize = 0;
			u1 result = 0;

			switch(mode)
			{
			case SSM_INVALID:
				return false;
			case SSM_READ:
				tempOffset = curWorkSection->valueOffset;
				tDataSize = 0;
				for(int i = 0; i < curWorkSection->valueCount; i++)
				{
					curWorkValue = reinterpret_cast<teValue *>(pool.At(tempOffset));

					if(curWorkValue->id == id)
					{
						memcpy(value, reinterpret_cast<void *>(pool.At(curWorkValue->dataOffset)), curWorkValue->size);
						result = 1;
						break;
					}

					tempOffset += sizeof(teValue);
				}
				break;
			case SSM_WRITE:
				curWorkSection->valueCount++;

				curWorkValue = reinterpret_cast<teValue *>(pool.At(curValueOffset));
				curWorkValue->id = id;
				curWorkValue->size = size;
				curWorkValue->dataOffset = curDataOffset;

				memcpy(pool.At(curValueOffset), curWorkValue, sizeof(teValue));

				curValueOffset += sizeof(teValue);

				memcpy(pool.At(curDataOffset), value, size);
				curDataOffset += size;
				result = 1;
				break;
			case SSM_WRITE_CALCULATE:
				++valuesCount;
				dataSize += size;
				result = 1;
				break;
			default:
				return false;
			}

			return result;
		}
	}
}
