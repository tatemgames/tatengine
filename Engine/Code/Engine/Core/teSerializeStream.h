/*
 *  teSerializeStream.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 12/19/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TESERIALIZESTREAM_H
#define TE_TESERIALIZESTREAM_H

#include "teConstArray.h"

namespace te
{
	namespace core
	{
		class teSerializeStream
		{
		public:
			teSerializeStream();
			teSerializeStream(void * setBuffer, size_t setSizeInBytes);
			~teSerializeStream();

			TE_INLINE teConstArray<u8> & GetPool() {return pool;}
			TE_INLINE const teConstArray<u8> & GetPool() const {return pool;}

			typedef void (*teSerializeDataFunc)(teSerializeStream *, void *);

			void Serialize(teSerializeDataFunc func, void * userData);
			void Deserialize(teSerializeDataFunc func, void * userData);

			u1 OpenSection(u16 id);
			void CloseSection();
			u1 OnValue(u16 id, void * value, u32 size);

			template <typename T>
			TE_INLINE u1 OnValue(u16 id, T * value)
			{
				return OnValue(id, value, sizeof(T));
			}

			enum ESerializeStreamMode
			{
				SSM_INVALID = 0,
				SSM_READ,
				SSM_WRITE,
				SSM_WRITE_CALCULATE,

				SSM_MAX
			};

			ESerializeStreamMode GetMode() const {return (ESerializeStreamMode)mode;}

		protected:
			teConstArray<u8> pool;

			u8 mode;

			u32 sectionsCount;
			u32 valuesCount;
			u32 dataSize;

			#pragma pack(push, 1)

			/*
			memory map :
			teSection sections[];
			teValue values[];
			binary data;
			*/

			struct teSection
			{
				u16 id;
				u16 valueCount;
				u32 valueOffset;
				u32 parentIndex;
				u16 childrenCount;
				u16 _reserved; // for memory align

				TE_INLINE void Clear()
				{
					id = u16Max;
					valueCount = 0;
					valueOffset = 0;
					parentIndex = 0;
					childrenCount = 0;
				}
			};

			struct teValue
			{
				u16 id;
				u16 _reserved; // for memory align only
				u32 size;
				u32 dataOffset;

				TE_INLINE void Clear()
				{
					id = u16Max;
					size = 0;
				}

				TE_INLINE u32 GetTotalSize() const {return sizeof(teValue) + size;}
			};

			#pragma pack(pop)

			teSection * curWorkSection;
			s32 curSectionIndex;
			u32 curParentIndex;

			u32 curValueOffset;

			u32 curDataOffset;
		};
	}
}

#endif
