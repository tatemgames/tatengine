/*
 *  teRemoteDebugger.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 07/1/12.
 *  Copyright 2012 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEREMOTEDEBUGGER_H
#define TE_TEREMOTEDEBUGGER_H

#include "teTypes.h"
#include "teRDNetwork.h"
#include "teFastScene.h"

namespace te
{
	namespace dbg
	{
		enum EPkgType
		{
			PT_CMD_TEST = 0,
			
			PT_CMD_TRANSFORM_MOVE_TO,

			PT_CMD_GET_FPS,

			PT_CMD_RETURN_MATERIALS,
			PT_CMD_RETURN_RENDER_PROGRAM,
			PT_CMD_RETURN_TRANSFORMS,
			PT_CMD_RETURN_SPRITES,
			PT_CMD_RETURN_SURFACES,
			PT_CMD_RETURN_TEXTS,
			PT_CMD_RETURN_VARIABLES,

			PT_RET_OK = 0xD0,
			PT_RET_OK_SEE_DETAILS,
			PT_RET_OK_SEE_DATA
		};

		class teRemoteDebugger
		{
		public:
			teRemoteDebugger();
			~teRemoteDebugger();

			void OnTick();

			TE_INLINE void SetScene(scene::teFastScene * setScene)
			{
				scene = setScene;
			}

			TE_INLINE scene::teFastScene * GetScene()
			{
				return scene;
			}

		protected:
			teRDNetwork network;
			scene::teFastScene * scene;
		};

		teRemoteDebugger * GetRemoteDebugger();
	}
}

#endif

