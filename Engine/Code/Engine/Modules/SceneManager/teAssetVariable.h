/*
 *  teAssetVariable.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 12/5/11.
 *  Copyright 2011 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEASSETVARIABLE_H
#define TE_TEASSETVARIABLE_H

#include "teTypes.h"
#include "teLinkTools.h"

namespace te
{
	namespace scene
	{
		enum EAssetVariableType
		{
			AVT_F32,
			AVT_S32,
			AVT_INVALID = u8Max
		};

		struct teAssetVariable
		{
			union
			{
				f32 vf32;
				s32 vs32;
			};

			u8 type;

			TE_INLINE void Clear()
			{
				type = AVT_INVALID;
				vs32 = 0;
			}

			TE_INLINE f32 GetF32() const
			{
				switch(type)
				{
				case AVT_F32: return vf32;
				case AVT_S32: return (f32)vs32;
				default: TE_ASSERT(0); return 0;
				}
			}

			TE_INLINE s32 GetS32() const
			{
				switch(type)
				{
				case AVT_F32: return (s32)vf32;
				case AVT_S32: return vs32;
				default: TE_ASSERT(0); return 0;
				}
			}
		};
	}
}

#endif