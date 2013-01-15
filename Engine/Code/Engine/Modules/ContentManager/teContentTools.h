/*
 *  teContentTools.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 12/1/11.
 *  Copyright 2011 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TECONTENTTOOLS_H
#define TE_TECONTENTTOOLS_H

#include "teSkeleton.h"
#include "teSurfaceData.h"
#include "teString.h"
#include "teAABB3d.h"

#include "teQuaternion.h"

namespace te
{
	namespace scene
	{
		video::teSurfaceData * CreateEmptySurface(u32 dataSize);
		u1 SaveEmptySurface(const teString & fileName, u32 dataSize);

		// layers format :
		// 3 * f32 - position
		// 4 * u8 - color
		// u16 - indexes
		video::teSurfaceData * CreateGrid3D();
		u1 SaveGrid3D(const teString & fileName);

		// calculate aabb for surface
		void CalculateSurfaceAABB(const video::teSurfaceData * data, const video::teSurfaceLayers & layers, teAABB3df & aabb);
		
		// calculate aabb for skel anim
		void CalculateSurfaceAABBSkeleton(teSkeleton * skeleton, const video::teSurfaceData * data, const video::teSurfaceLayers & layers, teAABB3df & aabb);
	
		/*
		// analize skel animation for transforms..
		void AnalizeSkelAnimation4(teSkeleton * skel_data);
		
		void AnalizeSkelAnimationByBones(teSkeleton * skel_data);
		
		void GetCrit(teSkeleton * skel_data, teSkeletonBoneKey * resultAv, teSkeletonBoneKey * resultDev);
		void GetCritInSegment(teSkeleton * skel_data, teSkeletonBoneKey * resultAv, teSkeletonBoneKey * resultDev, u32 frameBegin, u32 frameEnd);
		 */
	}
}

#endif
