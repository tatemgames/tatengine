/*
 *  teAssetDrawActor.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 06/14/12.
 *  Copyright 2012 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEASSETDRAWACTOR_H
#define TE_TEASSETDRAWACTOR_H

#include "teRenderAsset.h"
#include "teActor.h"

namespace te
{
	namespace scene
	{
		struct teAssetDrawActor
		{
			teRenderAsset renderAsset;
			teptr_t actorPtr; // TODO find better approach
			__actor::teSlotType4 actorSlot; // TODO find better approach

			TE_INLINE void Clear()
			{
				renderAsset.Clear();
				actorPtr = 0;
				actorSlot = NULL;
			}
		};
	}
}

#endif
