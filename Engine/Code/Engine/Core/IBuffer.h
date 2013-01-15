/*
 *  IBuffer.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 11/24/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_IBUFFER_H
#define TE_IBUFFER_H

#include "teTypes.h"
#include "teReferenceCounter.h"

namespace te
{
	namespace core
	{
		enum EBufferLockType
		{
			BLT_READ = 1, // lock for read only
			BLT_WRITE = 2, // lock for write only
			BLT_READWRITE = 3 // lock for read and write
		};

		class IBuffer : public te::teReferenceCounter
		{
		public:
			IBuffer()
				:position(0), streamMode(true), lock(false)
			{
			}

			virtual ~IBuffer()
			{
			}

			virtual void Lock(EBufferLockType setLockType)
			{
				TE_ASSERT(!lock);
				lock = true;
				lockType = setLockType;
			}

			virtual void Unlock()
			{
				TE_ASSERT(lock);
				lock = false;
			}

			TE_INLINE u1 TryLock(EBufferLockType setLockType)
			{
				if(!IsLocked())
				{
					Lock(setLockType);
					return true;
				}
				else
					if(!(lockType & setLockType))
					{
						Unlock();
						Lock(setLockType);
						return true;
					}
					else
						return false;
			}

			TE_INLINE u1 TryUnlock()
			{
				if(IsLocked())
				{
					Unlock();
					return true;
				}
				else
					return false;
			}

			TE_INLINE u1 IsLocked() const {return lock;}

			//! Allocate Buffer for data, if already allocated - reallocate with new size
			virtual void Allocate(u32 size) = 0;
			virtual void DeAllocate() = 0;

			virtual void Read(void * pointer, u32 size) = 0;
			virtual void Write(const void * pointer, u32 size) = 0;

			virtual u32 GetSize() const = 0;

			virtual u8 * GetArray() {return NULL;}

			virtual void SetPosition(u32 setPosition, u1 relative = false, u1 forward = true)
			{
				if(relative)
				{
					if(forward)
						position += setPosition;
					else
						position -= setPosition;
				}
				else
					position = setPosition;
			}
			
			TE_INLINE u32 GetPosition() const {return position;}

			TE_INLINE void SetStreamMode(u1 StreamMode) {streamMode = StreamMode;}
			TE_INLINE u1 GetStreamMode() const {return streamMode;}

			TE_INLINE u1 IsCanRead() const {return position < GetSize();}

		protected:
			u32 position;
			EBufferLockType lockType;
			u1 streamMode;
			u1 lock;
		};
	}
}

#endif
