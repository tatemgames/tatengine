/*
 *  teActorAbstractPlayerInput.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 12/11/11.
 *  Copyright 2011 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEACTORABSTRACTPLAYERINPUT_H
#define TE_TEACTORABSTRACTPLAYERINPUT_H

#include "teActor.h"
#include "teFpsCounter.h"
#include "teInputManager.h"

namespace te
{
	namespace scene
	{
		class teActorAbstractPlayerInput
		{
		public:
			TE_INLINE teActorAbstractPlayerInput(teFastScene * setScene){}
			TE_INLINE teActorAbstractPlayerInput(){}

			TE_INLINE void OnUpdate()
			{
				f32 speed = 1024.0f * TE_DT;
				f32 speedScale = 6.0f * TE_DT;

				#ifdef TE_INPUT_SUPPORT_KEYBOARD
					if(input::GetInputManager()->IsKeyDown(input::KC_LEFT)) transform->position.x -= speed;
					if(input::GetInputManager()->IsKeyDown(input::KC_RIGHT)) transform->position.x += speed;
					if(input::GetInputManager()->IsKeyDown(input::KC_UP)) transform->position.y += speed;
					if(input::GetInputManager()->IsKeyDown(input::KC_DOWN)) transform->position.y -= speed;
					if(input::GetInputManager()->IsKeyDown(input::KC_Q)) transform->rotation.w -= speed;
					if(input::GetInputManager()->IsKeyDown(input::KC_E)) transform->rotation.w += speed;
					if(input::GetInputManager()->IsKeyHit(input::KC_W)) transform->scale += speedScale;
					if(input::GetInputManager()->IsKeyHit(input::KC_S)) transform->scale -= speedScale;
				#endif
			}

		protected:
			teAssetTransform * transform;
		};

		TE_ACTOR_PROXY(teActorAbstractPlayerInput);

		TE_FUNC void RegisterAbstractPlayerInput(teActorsTI * ti)
		{
			ti->AddClass(sizeof(teActorAbstractPlayerInput), "abstractPlayerInput", TE_ACTOR_PROXY_NAMES(teActorAbstractPlayerInput));
			ti->AddLink("transform");
		}
	}
}

#endif