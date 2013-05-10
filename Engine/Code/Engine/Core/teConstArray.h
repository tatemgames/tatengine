/*
 *  teConstArray.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 12/1/11.
 *  Copyright 2011 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TECONSTARRAY_H
#define TE_TECONSTARRAY_H

#include "teMemoryManager.h"
#include "IBuffer.h"
#include "teMath.h"

namespace te
{
	//! @class teConstArray
	//! @brief Const size array for structs
	//! @details Simple, fast, const size array for structs, doesnt support constructors, copy constructors, destructors, placement new, etc.
	//! @details Basic ideas :
	//! @details 1) We reserve some elements count at first with Reserve method. GetSize return total reserved elements count. Reserve actually allocate memory.
	//! @details 2) We allocate 1 or more elements with Require, after allocation elements become alive. GetAlive return total alive elements count.
	//! @details 3) You cannot free element after allocation.
	//! @details So, GetAlive is size of all allocated elements, and GetSize is size of all reserved elements.
	//! @details Beware : all methods use TE_ASSERT for checks, so it becomes naive, check-less code in release.
	template<class T>
	class teConstArray
	{
	public:
		//! @brief Default constructor
		teConstArray()
			:size(0), alignment(0), freePosition(0), rawMemory(0), data(NULL)
		{
		}

		//! @brief Default constructor
		//! @param[in] setSize reserve array with elements count
		//! @param[in] setAlignment memory allocation alignment
		teConstArray(size_t setSize, size_t setAlignment = 4)
			:size(0), alignment(0), freePosition(0), rawMemory(0), data(NULL)
		{
			Reserve(setSize, setAlignment);
		}

		//! @brief Default constructor with nested buffer
		//! @param[in] setBuffer setup const array on buffer
		//! @param[in] setSizeInBytes buffer size in bytes
		teConstArray(void * setBuffer, size_t setSizeInBytes)
			:size(0), alignment(0), freePosition(0), rawMemory(0), data(NULL)
		{
			size = setSizeInBytes / sizeof(T);

			if(size)
				data = (T*)setBuffer;
		}

		//! @brief Default destructor
		//! @details Clear all allocated data.
		~teConstArray()
		{
			Clear();
		}

		//! @brief Reserve array with size
		//! @param[in] setSize array elements count
		//! @param[in] setAlignment memory allocation alignment
		void Reserve(size_t setSize, size_t setAlignment = 4)
		{
			TE_ASSERT(!data);
			TE_ASSERT(setSize);
			TE_ASSERT(IsNested() == false);
			size = setSize;
			alignment = setAlignment;
			freePosition = 0;

			TE_ALLOCATE_ALIGN(data, T, rawMemory, size * sizeof(T), alignment);
			memset(data, 0, size * sizeof(T));
		}

		//! @brief Reserve more size, shouldn't be used in production game code !
		//! @param[in] addSize reserve additional elements count
		//! @param[in] setAlignment memory allocation alignment
		//! @details Force realloc array, memory fragmenation occurs. Use it for editors and utils.
		void ReserveMore(size_t addSize, size_t setAlignment = 4)
		{
			TE_ASSERT(addSize);
			TE_ASSERT(IsNested() == false);

			if(rawMemory)
			{
				alignment = setAlignment;

				uintptr_t oldRawMemory = rawMemory;
				T * oldData = data;

				TE_ALLOCATE_ALIGN(data, T, rawMemory, (size + addSize) * sizeof(T), alignment);
				memcpy(data, oldData, size * sizeof(T));
				TE_FREE((void*)oldRawMemory);

				memset(data + size, 0, addSize * sizeof(T));
				size += addSize;
			}
			else
				Reserve(addSize, setAlignment);
		}

		//! @brief Remove all reserved data.
		//! @details After clear you can reserve array again.
		void Clear()
		{
			if(rawMemory)
				TE_FREE((void*)rawMemory);

			size = 0;
			alignment = 0;
			freePosition = 0;
			rawMemory = 0;
			data = NULL;
		}

		//! @brief Remove all requested data.
		//! @details After clear you can request elements again.
		void ClearRequested()
		{
			freePosition = 0;
		}

		//! @brief Return required size in bytes for save or load
		TE_INLINE size_t GetSystemSize() const
		{
			return sizeof(u64) + freePosition * sizeof(T);
		}

		//! @brief Save array to buffer
		//! @param[in] buffer save array to buffer
		//! @details You need to allocate memory in buffer first. Function only saves alive data.
		void Save(core::IBuffer * buffer) const
		{
			TE_ASSERT(buffer);

			u64 freePosition64 = freePosition;
			buffer->Write(&freePosition64, sizeof(u64));
			if(freePosition)
				buffer->Write(data, freePosition * sizeof(T));
		}

		//! @brief Load array from buffer
		//! @param[in] buffer load array from buffer
		//! @details You need clear array (with no data). Loading automaticly reserve data.
		void Load(core::IBuffer * buffer)
		{
			TE_ASSERT(buffer);

			Clear();

			u64 needSize = 0;
			buffer->Read(&needSize, sizeof(u64));
			if(needSize)
			{
				Reserve((size_t)needSize);
				buffer->Read(data, size * sizeof(T));
				freePosition = size;
			}
		}

		//! @brief Return true if nested
		TE_FORCE_INLINE u1 IsNested() const {return (data != NULL) && (rawMemory == NULL);}

		//! @brief Get array reserved elements count
		TE_FORCE_INLINE size_t GetSize() const {return size;}

		//! @brief Get array alive elements count
		TE_FORCE_INLINE size_t GetAlive() const {return freePosition;}

		//! @brief Get array reserved size in bytes
		TE_FORCE_INLINE size_t GetSizeBytes() const {return size * sizeof(T);}

		//! @brief Get array alive size in bytes
		TE_FORCE_INLINE size_t GetAliveBytes() const {return freePosition * sizeof(T);}

		//! @brief Return array memory pool
		TE_FORCE_INLINE T * GetPool(){TE_ASSERT(GetAlive()); return data;}

		//! @brief Return array memory pool
		TE_FORCE_INLINE const T * GetPool() const {TE_ASSERT(GetAlive()); return data;}

		//! @brief Return element with index
		TE_FORCE_INLINE T & operator [] (size_t i) {TE_ASSERT(i < freePosition); return data[i];}

		//! @brief Return element with index
		TE_FORCE_INLINE const T & operator [] (size_t i) const {TE_ASSERT(i < freePosition); return data[i];}

		//! @brief Return memory pointer to element at index
		TE_FORCE_INLINE T * At(size_t i) {TE_ASSERT(i < freePosition); return data + i;}

		//! @brief Return memory pointer to element at index
		TE_FORCE_INLINE const T * At(size_t i) const {TE_ASSERT(i < freePosition); return data + i;}

		//! @brief Return first element
		TE_FORCE_INLINE T & GetFirst() {TE_ASSERT(GetAlive()); return *At(0);}

		//! @brief Return first element
		TE_FORCE_INLINE const T & GetFirst() const {TE_ASSERT(GetAlive()); return *At(0);}

		//! @brief Return last element
		TE_FORCE_INLINE T & GetLast() {TE_ASSERT(GetAlive()); return *At(GetAlive() - 1);}

		//! @brief Return last element
		TE_FORCE_INLINE const T & GetLast() const {TE_ASSERT(GetAlive()); return *At(GetAlive() - 1);}

		//! @brief Return element index in array by pointer
		//! param[in] pointer element pointer
		//! @details Not failsafe, so additional result checks required if you cant guarantee valid pointer.
		TE_FORCE_INLINE u64 GetIndexInArray(const T * pointer) const
		{
			return (reinterpret_cast<uintptr_t>(pointer) - reinterpret_cast<uintptr_t>(data)) / sizeof(T);
		}

		//! @brief Return true if pointer from this array
		//! param[in] pointer element pointer
		TE_FORCE_INLINE u1 IsFromThisArray(const T * pointer) const
		{
			if(!GetAlive())
				return false;

			uintptr_t first = reinterpret_cast<uintptr_t>(At(0));
			uintptr_t last = reinterpret_cast<uintptr_t>(At(GetAlive() - 1));
			uintptr_t ptr = reinterpret_cast<uintptr_t>(pointer);
			return (ptr >= first) && (ptr <= last);
		}

		//! @brief Request free elements from reserved array
		//! @param[in] count request elements count
		//! @param[out] getIndex write index of first element
		TE_INLINE T * Request(size_t count, u64 * getIndex = NULL)
		{
			TE_ASSERT(freePosition + count <= size);

			if(getIndex)
				*getIndex = freePosition;

			T * result = data + freePosition;
			freePosition += count;
			return result;
		}

		//! @brief Request free elements from reserved array
		//! @param[in] count request elements count
		//! @param[out] getIndex write index of first element
		TE_INLINE T * Request(size_t count, u32 * getIndex)
		{
			TE_ASSERT(freePosition + count <= size);

			if(getIndex)
				*getIndex = freePosition;

			T * result = data + freePosition;
			freePosition += count;
			return result;
		}

		//! @brief Request free element from reserved array
		//! @param[out] getIndex write index of first element
		TE_INLINE T * Request(u64 * getIndex = NULL) {return Request(1, getIndex);}

		//! @brief Request free element from reserved array
		//! @param[out] getIndex write index of first element
		TE_INLINE T * Request(u32 * getIndex) {return Request(1, getIndex);}

		//! @brief Element compare function prototype
		//! @details result > 0 if a > b, < 0 if a < b, == 0, if a == b
		typedef void (*teCompareFunction)(s8 & result, const T & a, const T & b);

		//! @brief Sort array with shell sort
		//! @param[in] f compare function
		void ShellSort(teCompareFunction f)
		{
			size_t h = 1;
			while(h <= GetAlive() / 9)
				h = 3 * h + 1;

			u8 temp[sizeof(T)];
			s8 tempResult = 0;

			while(h > 0)
			{
				for(size_t i = h; i < GetAlive(); ++i)
				{
					size_t j = i;

					memcpy(temp, At(i), sizeof(T));

					f(tempResult, *At(j - h), *reinterpret_cast<const T*>(temp));

					while((j >= h) && (tempResult > 0))
					{
						memcpy(At(j), At(j - h), sizeof(T));

						if(j >= h)
							j -= h;
						else
							j = 0;

						if(j >= h)
							f(tempResult, *At(j - h), *reinterpret_cast<const T*>(temp));
					}

					memcpy(At(j), temp, sizeof(T));
				}

				h /= 3;
			}
		}

		//! @brief Linear element search
		//! @param[in] object search element that equal to this
		//! @param[out] result finded index
		//! @return u1 true if finded, false if failed
		TE_INLINE u1 LinearSearch(const T & object, size_t & result) const
		{
			for(size_t i = 0; i < GetAlive(); ++i)
				if(*At(i) == object)
				{
					result = i;
					return true;
				}

			return false;
		}

		//! @brief Binary element search
		//! @param[in] object search element that equal to this
		//! @param[out] result finded index
		//! @return u1 true if finded, false if failed
		//! @details Array must be ascent sorted.
		TE_INLINE u1 BinarySearch(const T & object, size_t & result) const // TODO check this
		{
			size_t min = 0, max = GetAlive() - 1, mid = (min + max) / 2;

			while((*At(mid) != object) && (min < max))
			{
				mid = (min + max) / 2;

				if(object < (*At(mid)))
					max = mid - 1;
				else
					min = mid + 1;
			}

			if(*At(mid) == object)
			{
				result = mid;
				return true;
			}

			return false;
		}

	protected:
		//! @brief Total reserved elements count
		size_t size;

		//! @brief Allocation alignment
		size_t alignment;

		//! @brief Current element free position
		size_t freePosition;

		//! @brief Unaligned memory block
		uintptr_t rawMemory;

		//! @brief Aligned memory block
		T * data;

	private:
		//! @brief Private copy constructor
		teConstArray(const teConstArray & other){}
	};
}

#endif
