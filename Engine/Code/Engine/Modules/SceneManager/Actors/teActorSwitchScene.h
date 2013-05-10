/*
 *  teActorSwitchScene.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 02/25/12.
 *  Copyright 2012 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEACTORSWITCHSCENE_H
#define TE_TEACTORSWITCHSCENE_H

#include "teActor.h"
#include "teFastScene.h"
#include "teApplicationManager.h"

namespace te
{
	namespace scene
	{
		class teActorSwitchScene
		{
		public:
			teActorSwitchScene(teFastScene * setScene)
				:scene(setScene)
			{
			}

			~teActorSwitchScene()
			{
			}

			TE_INLINE void SwitchTo(f32 switchTo)
			{
				scene->Load((u8)switchTo, true);
			}

			TE_INLINE void CloseApplication()
			{
				app::GetApplicationManager()->GetApplication()->SetRun(false);
			}

		protected:
			teFastScene * scene;
		};

		TE_ACTOR_PROXY_NU(teActorSwitchScene);
		TE_ACTOR_SLOT_1(teActorSwitchScene, SwitchTo);
		TE_ACTOR_SLOT_0(teActorSwitchScene, CloseApplication);

		TE_FUNC void RegisterSwitchScene(teActorsTI * ti)
		{
			ti->AddClass(sizeof(teActorSwitchScene), "switchScene", TE_ACTOR_PROXY_NAMES_NU(teActorSwitchScene));
			ti->AddSlot("SwitchTo", TE_ACTOR_SLOT_PROXY(teActorSwitchScene, SwitchTo));
			ti->AddSlot("CloseApplication", TE_ACTOR_SLOT_PROXY(teActorSwitchScene, CloseApplication));
		}
	}
}

#endif
