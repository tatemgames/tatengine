/*
 *  teReferenceCounter.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 11/24/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEREFERENCECOUNTER_H
#define TE_TEREFERENCECOUNTER_H

#include "teTypes.h"
#include "teDebug.h"
#include "teMemoryManager.h"

namespace te
{
	class teReferenceCounter
	{
	public:
		teReferenceCounter():_referenceCount(1) {}
		virtual ~teReferenceCounter() {}
		
		void Grab()
		{
			++_referenceCount;
		}
		
		u1 Drop()
		{
			TE_ASSERT(_referenceCount > 0)
			
			if (!(--_referenceCount))
			{
				TE_DELETE(this)
				return true;
			}
			else
				return false;
		}
		
		u32 GetReferenceCount() const {return _referenceCount;}

	private:
		u32 _referenceCount;
	};
}

#define TE_SAFE_GRAB(ptr) \
	{ \
		if(ptr) \
			(ptr)->Grab(); \
	}

#define TE_SAFE_DROP(ptr) \
	{ \
		if(ptr) \
		{ \
			(ptr)->Drop(); \
			(ptr) = NULL; \
		} \
	}

#endif
