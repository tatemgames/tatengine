/*
 *  teRemoteDebugger.cpp
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 07/1/12.
 *  Copyright 2012 Tatem Games. All rights reserved.
 *
 */

#include "teRemoteDebugger.h"
#include "teFpsCounter.h"

namespace te
{
	namespace dbg
	{
		teRemoteDebugger * currentRemoteDebugger = NULL;

		teRemoteDebugger::teRemoteDebugger()
			:scene(NULL)
		{
			currentRemoteDebugger = this;

			TE_LOG_DBG("remote debugger inited");

			network.StartServer();
		}

		teRemoteDebugger::~teRemoteDebugger()
		{
			network.StopServer();

			currentRemoteDebugger = NULL;
		}

		void teRemoteDebugger::OnTick()
		{
			tePkg * pkg = network.Get();

			if(!pkg)
			{
				network.ClearGet();
				return;
			}

			switch(pkg->type)
			{
			case PT_CMD_TEST:
				{
					network.Send(PT_RET_OK);
					break;
				}
			case PT_CMD_TRANSFORM_MOVE_TO:
				{
					scene->GetAssetPack().transforms[pkg->d[0]].position.SetXYZ(pkg->f[0], pkg->f[1], pkg->f[2]);
					network.Send(PT_RET_OK);
					break;
				}
			case PT_CMD_GET_FPS:
				{
					f32 f[4] = {0.0f, 0.0f, 0.0f, 0.0f};
					f[0] = core::GetFpsCounter()->GetFps();

					network.Send(PT_RET_OK_SEE_DETAILS, NULL, f);
					break;
				}
			case PT_CMD_RETURN_MATERIALS:
				{
					network.Send(PT_RET_OK_SEE_DATA);
					network.SendData(scene->GetContentPack().materials);
					break;
				}
			case PT_CMD_RETURN_RENDER_PROGRAM:
				{
					network.Send(PT_RET_OK_SEE_DATA);
					network.SendData(scene->GetRenderProgram());
					break;
				}
			case PT_CMD_RETURN_TRANSFORMS:
				{
					network.Send(PT_RET_OK_SEE_DATA);
					network.SendData(scene->GetAssetPack().transforms);
					break;
				}
			case PT_CMD_RETURN_SPRITES:
				{
					network.Send(PT_RET_OK_SEE_DATA);
					network.SendData(scene->GetAssetPack().sprites);
					break;
				}
			case PT_CMD_RETURN_SURFACES:
				{
					network.Send(PT_RET_OK_SEE_DATA);
					network.SendData(scene->GetAssetPack().surfaces);
					break;
				}
			case PT_CMD_RETURN_TEXTS:
				{
					network.Send(PT_RET_OK_SEE_DATA);
					network.SendData(scene->GetAssetPack().texts);
					break;
				}
			case PT_CMD_RETURN_VARIABLES:
				{
					network.Send(PT_RET_OK_SEE_DATA);
					network.SendData(scene->GetAssetPack().variables);
					break;
				}
			default:
				{
					TE_LOG_WRN("network pkg : wrong type");
					break;
				}
			}

			network.ClearGet();
		}

		teRemoteDebugger * GetRemoteDebugger()
		{
			return currentRemoteDebugger;
		}
	}
}
