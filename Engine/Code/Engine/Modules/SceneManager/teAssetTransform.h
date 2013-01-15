/*
 *  teAssetTransform.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 12/1/11.
 *  Copyright 2011 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEASSETTRANSFORM_H
#define TE_TEASSETTRANSFORM_H

#include "teVector3D.h"
#include "teQuaternion.h"

namespace te
{
	namespace scene
	{
		struct teAssetTransform
		{
			teQuaternionf rotation;
			teVector3df position;
			teVector3df scale;
			u32 parent;
			u1 visible;
			u1 inFrame;

			TE_INLINE void Clear()
			{
				position.SetXYZ(0.0f, 0.0f, 0.0f);
				rotation.SetXYZW(0.0f, 0.0f, 0.0f, 0.0f);
				scale.SetXYZ(1.0f, 1.0f, 1.0f);

				parent = u32Max;
				visible = true;
			}
		};

		class teAssetPack;
		void CalculateTransformGlobalMatrix(teAssetPack & assetPack, u32 index);
	}
}

#endif