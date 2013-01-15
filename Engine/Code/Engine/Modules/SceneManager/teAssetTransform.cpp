/*
 *  teAssetTransform.cpp
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 06/27/12.
 *  Copyright 2012 Tatem Games. All rights reserved.
 *
 */

#include "teAssetTransform.h"
#include "teAssetPack.h"
#include "teFastScene.h"

namespace te
{
	namespace scene
	{
		void CalculateTransformGlobalMatrix(teAssetPack & assetPack, u32 index)
		{
			teAssetTransform & transform = assetPack.transforms[index];

			if(transform.parent != u32Max)
			{
				transform.inFrame = transform.visible && assetPack.transforms[transform.parent].inFrame;

				teMatrix4f mr;
				transform.rotation.GetMatrix(mr);

				teMatrix4f mo;
				mo.SetTransforms3D(transform.position, transform.scale, mr);

				assetPack.global[transform.parent].MultiplicationAffine(mo, assetPack.global[index]);
			}
			else
			{
				transform.inFrame = transform.visible;

				teMatrix4f mr;
				transform.rotation.GetMatrix(mr);

				assetPack.global[index].SetTransforms3D(transform.position, transform.scale, mr);
			}
		}
	}
}
