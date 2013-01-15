/*
 *  teAssetArray.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 04/15/11.
 *  Copyright 2012 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEASSETARRAY_H
#define TE_TEASSETARRAY_H

#include "teTypes.h"

namespace te
{
	namespace scene
	{
		struct teAssetArray
		{
			u32 count;

			// data is pointer to first element in teAssetPack::arraysNested, and teAssetPack::arraysNested contains pointer to actual data
			// so if you create links to teAssetVariable, than you must cast like this : teAssetVariable ** array = reinterpret_cast<teAssetVariable**>(assetArray->data);
			teptr_t data;

			TE_INLINE void Clear()
			{
				count = 0;
				data = teptr_t_Max;
			}
		};
	}
}

#endif