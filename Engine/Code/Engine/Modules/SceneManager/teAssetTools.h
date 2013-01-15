/*
 *  teAssetTools.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 04/19/12.
 *  Copyright 2012 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEASSETTOOLS_H
#define TE_TEASSETTOOLS_H

#include "teAssetPack.h"

namespace te
{
	namespace scene
	{
		u32 FindCameraThatRenderAsset(const teRenderProgram & program, ERenderCommandType renderAssetCommand, u32 assetIndex);
		void GetRayFromPointInViewport(const teAssetPack & scene, u32 cameraIndex, const teVector2df & pointInViewportSpace, teVector3df & rayNear, teVector3df & rayFar);

		void ProjectPointToViewport(const teAssetPack & scene, u32 cameraIndex, const teVector3df & point, const teMatrix4f & matView, const teMatrix4f & matPrj, const teVector2df & viewportPosition, const teVector2df & viewportSize, teVector2df & result);
		void ProjectAABBToViewport(const teAssetPack & scene, u32 cameraIndex, const teAABB3df & aabb, const teMatrix4f & matView, const teMatrix4f & matPrj, const teVector2df & viewportPosition, const teVector2df & viewportSize, teVector2df & pointA, teVector2df & pointB);
	}
};

#endif