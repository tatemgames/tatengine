/*
 *  teAssetCamera.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 12/1/11.
 *  Copyright 2011 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEASSETCAMERA_H
#define TE_TEASSETCAMERA_H

#include "teMatrix4.h"

namespace te
{
	namespace scene
	{
		enum ECameraType
		{
			CT_PERSPECTIVE = 0,
			CT_ORTHOGRAPHIC
		};

		struct teAssetCamera
		{
			f32 nearPlane, farPlane;
			f32 fov, aspect;
			teVector2di viewportPosition, viewportSize;
			u32 transformIndex;
			u8 cameraType;

			TE_INLINE void Clear()
			{
				transformIndex = u32Max;
				//Set2D();
			}

			/*
			TE_INLINE void Set2D()
			{
				viewportPosition.SetXY(0, 0);
				viewportSize.SetXY(TE_LAUNCHER_SCREEN_WIDTH, TE_LAUNCHER_SCREEN_HEIGHT);

				cameraType = CT_ORTHOGRAPHIC;
				nearPlane = -1.0f;
				farPlane = 1.0f;
				fov = TE_LAUNCHER_SCREEN_HEIGHT;
				aspect = (f32)viewportSize.x / (f32)viewportSize.y;
			}

			TE_INLINE void Set3D()
			{
				viewportPosition.SetXY(0, 0);
				viewportSize.SetXY(TE_LAUNCHER_SCREEN_WIDTH, TE_LAUNCHER_SCREEN_HEIGHT);

				cameraType = CT_PERSPECTIVE;
				nearPlane = 2.0f;
				farPlane = 250.0f;
				fov = 45.0f;
				aspect = (f32)viewportSize.x / (f32)viewportSize.y;
			}

			TE_INLINE void SetViewportDefault()
			{
				viewportPosition.SetXY(0, 0);
				viewportSize.SetXY(TE_LAUNCHER_SCREEN_WIDTH, TE_LAUNCHER_SCREEN_HEIGHT);
				aspect = (f32)viewportSize.x / (f32)viewportSize.y;

				if(cameraType == CT_ORTHOGRAPHIC)
					fov = (f32)viewportSize.y;
			}
			*/

			TE_INLINE void BuildProjectionMatrix(teMatrix4f & matP) const
			{
				if(cameraType == CT_ORTHOGRAPHIC)
					matP.SetOrthographic(-fov * aspect / 2.0f, fov * aspect / 2.0f, -fov / 2.0f, fov / 2.0f, nearPlane, farPlane);
				else
					matP.SetPerspective(fov, aspect, nearPlane, farPlane);
			}
		};
	}
}

#endif