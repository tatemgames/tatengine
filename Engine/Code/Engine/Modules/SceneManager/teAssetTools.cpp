/*
 *  teAssetTools.cpp
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 04/19/12.
 *  Copyright 2012 Tatem Games. All rights reserved.
 *
 */

#include "teAssetTools.h"

namespace te
{
	namespace scene
	{
		u32 FindCameraThatRenderAsset(const teRenderProgram & program, ERenderCommandType renderAssetCommand, u32 assetIndex)
		{
			u32 cameraIndex = u32Max;
			for(u32 i = 0; i < program.GetAlive(); ++i)
			{
				const teRenderCommand & command = program[i];

				if(command.type == RCT_SET_CAMERA)
					cameraIndex = command.from;
				else if(command.type == renderAssetCommand)
				{
					if((assetIndex >= command.from) && (assetIndex < (command.from + command.count)))
						return cameraIndex;
				}
			}

			return u32Max;
		}

		void GetRayFromPointInViewport(const teAssetPack & scene, u32 cameraIndex, const teVector2df & pointInViewportSpace, teVector3df & rayNear, teVector3df & rayFar)
		{
			if(cameraIndex == u32Max)
				return;

			const teAssetCamera & camera = scene.cameras[cameraIndex];

			teVector2df pointInScreenSpace = ((pointInViewportSpace + camera.viewportPosition) / camera.viewportSize) * 2.0f - 1.0f;
			teVector3df pointRawNear(pointInScreenSpace.x, -pointInScreenSpace.y, -1.0f); // ray in screen space cube
			teVector3df pointRawFar(pointInScreenSpace.x, -pointInScreenSpace.y, 1.0f);

			teMatrix4f matView, matProj, matViewProjInverse; // calculate MVP matrix
			scene.global[camera.transformIndex].Inverse(matView);
			camera.BuildProjectionMatrix(matProj);
			(matProj * matView).Inverse(matViewProjInverse);

			f32 rayNearTemp[4]; // get ray in world space
			f32 rayFarTemp[4];
			matViewProjInverse.MultiplyMatrixOnVector4D(pointRawNear.x, pointRawNear.y, pointRawNear.z, 1.0f, rayNearTemp);
			matViewProjInverse.MultiplyMatrixOnVector4D(pointRawFar.x, pointRawFar.y, pointRawFar.z, 1.0f, rayFarTemp);
			rayNear.SetXYZ(rayNearTemp[0] / rayNearTemp[3], rayNearTemp[1] / rayNearTemp[3], rayNearTemp[2] / rayNearTemp[3]);
			rayFar.SetXYZ(rayFarTemp[0] / rayFarTemp[3], rayFarTemp[1] / rayFarTemp[3], rayFarTemp[2] / rayFarTemp[3]);
		}

		void ProjectPointToViewport(const teAssetPack & scene, u32 cameraIndex, const teVector3df & point, const teMatrix4f & matView, const teMatrix4f & matPrj, const teVector2df & viewportPosition, const teVector2df & viewportSize, teVector2df & result)
		{
			if(cameraIndex == u32Max)
				return;

			teVector3df eye = matView.MultiplyMatrixOnVector3D(point);

			f32 prj[4];
			matPrj.MultiplyMatrixOnVector4D(eye.x, eye.y, eye.z, 1.0f, prj);

			result = (teVector2df(prj[0] / prj[3], prj[1] / prj[3]) * 0.5f + 0.5f) * viewportSize + viewportPosition;
		}

		void ProjectAABBToViewport(const teAssetPack & scene, u32 cameraIndex, const teAABB3df & aabb, const teMatrix4f & matView, const teMatrix4f & matPrj, const teVector2df & viewportPosition, const teVector2df & viewportSize, teVector2df & pointA, teVector2df & pointB)
		{
			if(cameraIndex == u32Max)
				return;

			teVector3df eyeA = matView.MultiplyMatrixOnVector3D(aabb.edgeMin);
			teVector3df eyeB = matView.MultiplyMatrixOnVector3D(aabb.edgeMax);

			f32 prjA[4];
			f32 prjB[4];
			matPrj.MultiplyMatrixOnVector4D(eyeA.x, eyeA.y, eyeA.z, 1.0f, prjA);
			matPrj.MultiplyMatrixOnVector4D(eyeB.x, eyeB.y, eyeB.z, 1.0f, prjB);

			teVector2df chkA(prjA[0] / prjA[3], prjA[1] / prjA[3]);
			teVector2df chkB(prjB[0] / prjB[3], prjB[1] / prjB[3]);

			teVector2df a(teMin(chkA.x, chkB.x), teMin(chkA.y, chkB.y));
			teVector2df b(teMax(chkA.x, chkB.x), teMax(chkA.y, chkB.y));

			pointA = (a * 0.5f + 0.5f) * viewportSize + viewportPosition;
			pointB = (b * 0.5f + 0.5f) * viewportSize + viewportPosition;
		}

		void GetSpriteAssetRenderingRect(const teContentPack & contentPack, const teAssetPack & assetPack, const teAssetSprite & sprite, teVector3df * position, teVector2df * size)
		{
			TE_ASSERT(position != nullptr);
			TE_ASSERT(size != nullptr);

			const video::teMaterial & material = contentPack.materials[sprite.renderAsset.materialIndex];
			const teMatrix4f & mat = assetPack.global[sprite.renderAsset.transformIndex];
			const video::teAtlasSprite * atlasSprite = nullptr;
			if(material.atlasSpriteIndex[0] != u32Max)
				atlasSprite = contentPack.atlasSprites.At(material.atlasSpriteIndex[0]);

			TE_ASSERT(atlasSprite != nullptr);

			teVector3df p[4];

			p[0].SetXYZ(0.0f, 0.0f, 0.0f);
			p[1].SetXYZ(1.0f, 0.0f, 0.0f);
			p[2].SetXYZ(0.0f, 1.0f, 0.0f);
			p[3].SetXYZ(1.0f, 1.0f, 0.0f);

			for(u8 i = 0; i < 4; ++i)
			{
				p[i].SetXYZ(p[i].x * (atlasSprite->size.x), p[i].y * atlasSprite->size.y, 0.0f);

				if(atlasSprite->texelToPixel)
					p[i].SetXYZ(p[i].x - teFloor(atlasSprite->origin.x), p[i].y + teFloor(atlasSprite->origin.y) - atlasSprite->size.y, 0.0f);
				else
					p[i].SetXYZ(p[i].x - atlasSprite->origin.x, p[i].y + atlasSprite->origin.y - atlasSprite->size.y, 0.0f);
			}

			for(u8 i = 0; i < 4; ++i)
			{
				p[i] = mat.MultiplyMatrixOnVector3D(p[i]);
			}

			*position = p[0];
			// width
			size->x = teAbs(p[3].x - p[0].x);
			// height
			size->y = teAbs(p[2].y - p[1].y);
		}
	}
};
