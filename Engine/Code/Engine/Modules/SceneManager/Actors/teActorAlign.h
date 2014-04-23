/*
 *  teActorAlign.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 02/26/12.
 *  Copyright 2012 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEACTORALIGN_H
#define TE_TEACTORALIGN_H

#include "teActor.h"
#include "teMath.h"

namespace te
{
	namespace scene
	{
		class teActorAlign
		{
		public:
			TE_INLINE void OnViewportChanged(f32 width, f32 height)
			{
				f32 px, py;
				GetRectangleProportion((ERectangleOriginCorner)alignType->GetS32(), px, py);

				px = px - 0.5f;
				py = 0.5f - py;

				transform->position.x = width * px + (offsetX ? offsetX->GetF32() : 0);
				transform->position.y = height * py + (offsetY ? offsetY->GetF32() : 0);
			}

		protected:
			teAssetTransform * transform;
			teAssetVariable * alignType;
			teAssetVariable * offsetX;
			teAssetVariable * offsetY;

		};

		TE_ACTOR_SLOT_2(teActorAlign, OnViewportChanged);

		TE_FUNC void RegisterAlign(teActorsTI * ti)
		{
			ti->AddClass(sizeof(teActorAlign), "align", TE_ACTOR_PROXY_NAMES_NONE, "align 2d sprite to corner of 2d camera viewport");
			ti->AddLink("transform", false, "transform", "transform of sprite");
			ti->AddLink("alignType");
			ti->AddLink("offsetX");
			ti->AddLink("offsetY");
			ti->AddSlot("OnViewportChanged", TE_ACTOR_SLOT_PROXY(teActorAlign, OnViewportChanged));
		}
	}
}

#endif