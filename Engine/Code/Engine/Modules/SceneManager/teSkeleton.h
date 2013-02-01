/*
 *  teSkeleton.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 01/28/12.
 *  Copyright 2012 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TESKELETON_H
#define TE_TESKELETON_H

#include "teTypes.h"
#include "teQuaternion.h"

/*
skeleton memory map :

{
	teSkeleton skeleton;
	teSkeletonBone bones[skeleton.bonesCount];
	u32 framesCount[skeleton.layersCount];
	teSkeletonFrame frames[skeleton.framesCount];
	teDualQuaternion skin[skeleton.bonesCount];
	teSkeletonBoneKey bonesKeys[skeleton.framesCount + 1][skeleton.bonesCount]; // 0 frame = bind pose
}
*/

namespace te
{
	namespace scene
	{
		#pragma pack(push, 1)

		struct teSkeletonBone
		{
			u8 index;
			u8 parent;
			u8 childrensBegin, childrensEnd;
		};

		struct teSkeletonFrame
		{
			u8 layer;
			u16 frame;
			
			u8 flags;
		};

		struct teSkeletonBoneKey
		{
			f32 rotation[4];
			f32 translation[3];

			TE_INLINE void GetDualQuaternion(teDualQuaternion & result) const
			{
				result.SetFrom(teQuaternionf(rotation[0], rotation[1], rotation[2], rotation[3]), teVector3df(translation[0], translation[1], translation[2]));
			}
			
			TE_INLINE void Get(teQuaternionf & a, teVector3df & b) const
			{
				a.SetXYZW(rotation[0], rotation[1], rotation[2], rotation[3]);
				b.SetXYZ(translation[0], translation[1], translation[2]);
			}
		};

		struct teSkeleton
		{
			f32 frameDeltaTime;
			u32 framesCount;
			u32 bonesCount;
			u32 layersCount;
			teSkeletonBone bones[];

			TE_INLINE u32 GetFramesCount(u16 layer) const {return (reinterpret_cast<const u32*>((uintptr_t)this + sizeof(teSkeleton) + sizeof(teSkeletonBone) * bonesCount))[layer];}

			//TE_INLINE teSkeletonFrame * GetFrames() {return reinterpret_cast<const teSkeletonFrame*>((uintptr_t)this + sizeof(teSkeleton) + sizeof(teSkeletonBone) * bonesCount + sizeof(u32) * layersCount);}
			TE_INLINE const teSkeletonFrame * GetFrames() const {return reinterpret_cast<const teSkeletonFrame*>((uintptr_t)this + sizeof(teSkeleton) + sizeof(teSkeletonBone) * bonesCount + sizeof(u32) * layersCount);}

			TE_INLINE teDualQuaternion * GetSkin() {return reinterpret_cast<teDualQuaternion*>((uintptr_t)this + sizeof(teSkeleton) + sizeof(teSkeletonBone) * bonesCount + sizeof(u32) * layersCount + sizeof(teSkeletonFrame) * framesCount);}
			TE_INLINE const teDualQuaternion * GetSkin() const {return reinterpret_cast<const teDualQuaternion*>((uintptr_t)this + sizeof(teSkeleton) + sizeof(teSkeletonBone) * bonesCount + sizeof(u32) * layersCount + sizeof(teSkeletonFrame) * framesCount);}

			// if frameIndex == u32Max - return bind pose frame
			TE_INLINE const teSkeletonBoneKey * GetBoneKey(u32 boneIndex, u32 frameIndex = u32Max) const
			{
				uintptr_t keysPtr = (uintptr_t)this + sizeof(teSkeleton) + sizeof(teSkeletonBone) * bonesCount + sizeof(u32) * layersCount + sizeof(teSkeletonFrame) * framesCount + sizeof(teDualQuaternion) * bonesCount;
				keysPtr += sizeof(teSkeletonBoneKey) * ((framesCount + 1) * boneIndex + ((frameIndex == u32Max) ? 0 : (frameIndex + 1)));
				return reinterpret_cast<const teSkeletonBoneKey*>(keysPtr);
			}
			
			u1 CalculateSkin(const teVector2duh & pair0)
			{
				return CalculateSkin(pair0, pair0, 0.0f);
			}

			u1 CalculateSkin(const teVector2duh & pair0, const teVector2duh & pair1, f32 blend0)
			{
				const teSkeletonFrame * frames = GetFrames();
				teDualQuaternion * skin = GetSkin();

				u32 frameIndex[2] = {u32Max, u32Max};

				for(u32 i = 0; i < framesCount; ++i) if((frames[i].layer == pair0.x) && (frames[i].frame == pair0.y)) {frameIndex[0] = i; break;}
				for(u32 i = 0; i < framesCount; ++i) if((frames[i].layer == pair1.x) && (frames[i].frame == pair1.y)) {frameIndex[1] = i; break;}

				if((frameIndex[0] == u32Max) || (frameIndex[1] == u32Max))
					return false;

				for(u32 boneIndex = 0; boneIndex < bonesCount; ++boneIndex)
				{
					teQuaternionf a, a0, a1;
					teVector3df b, b0, b1;
					GetBoneKey(boneIndex, frameIndex[0])->Get(a0, b0);
					GetBoneKey(boneIndex, frameIndex[1])->Get(a1, b1);

					teDualQuaternion dqc;
					a = a0.Slerp(a1, blend0);
					b = b0.LinearInterpolation(b1, blend0);
					dqc.SetFrom(a, b);

					if(boneIndex)
					{
						teDualQuaternion dqp = skin[bones[boneIndex].parent];
						dqc.MultiplyBy(dqp);
						skin[boneIndex] = dqc;
					}
					else
						skin[boneIndex] = dqc;
				}

				for(u32 boneIndex = 0; boneIndex < bonesCount; ++boneIndex)
				{
					teDualQuaternion bp;
					GetBoneKey(boneIndex)->GetDualQuaternion(bp);
					bp.MultiplyBy(skin[boneIndex]);
					skin[boneIndex] = bp;
				}

				return true;
			}

			u1 CalculateSkin(const teVector2duh & pair0, const teVector2duh & pair1, const teVector2duh & pair2, f32 blend0, f32 blend1)
			{
				const teSkeletonFrame * frames = GetFrames();
				teDualQuaternion * skin = GetSkin();

				u32 frameIndex[3] = {u32Max, u32Max, u32Max};

				for(u32 i = 0; i < framesCount; ++i) if((frames[i].layer == pair0.x) && (frames[i].frame == pair0.y)) {frameIndex[0] = i; break;}
				for(u32 i = 0; i < framesCount; ++i) if((frames[i].layer == pair1.x) && (frames[i].frame == pair1.y)) {frameIndex[1] = i; break;}
				for(u32 i = 0; i < framesCount; ++i) if((frames[i].layer == pair2.x) && (frames[i].frame == pair2.y)) {frameIndex[2] = i; break;}

				if((frameIndex[0] == u32Max) || (frameIndex[1] == u32Max) || (frameIndex[2] == u32Max))
					return false;

				for(u32 boneIndex = 0; boneIndex < bonesCount; ++boneIndex)
				{
					teQuaternionf a, a0, a1, a2;
					teVector3df b, b0, b1, b2;
					GetBoneKey(boneIndex, frameIndex[0])->Get(a0, b0);
					GetBoneKey(boneIndex, frameIndex[1])->Get(a1, b1);
					GetBoneKey(boneIndex, frameIndex[2])->Get(a2, b2);

					teQuaternionf ab0 = a0.Slerp(a1, blend0);
					teVector3df bb0 = b0.LinearInterpolation(b1, blend0);

					teDualQuaternion dqc;
					a = ab0.Slerp(a2, blend1);
					b = bb0.LinearInterpolation(b2, blend1);
					dqc.SetFrom(a, b);

					if(boneIndex)
					{
						teDualQuaternion dqp = skin[bones[boneIndex].parent];
						dqc.MultiplyBy(dqp);
						skin[boneIndex] = dqc;
					}
					else
						skin[boneIndex] = dqc;
				}

				for(u32 boneIndex = 0; boneIndex < bonesCount; ++boneIndex)
				{
					teDualQuaternion bp;
					GetBoneKey(boneIndex)->GetDualQuaternion(bp);
					bp.MultiplyBy(skin[boneIndex]);
					skin[boneIndex] = bp;
				}

				return true;
			}

			u1 CalculateSkin(const teVector2duh & pair0, const teVector2duh & pair1, const teVector2duh & pair2, const teVector2duh & pair3, f32 blend0, f32 blend1, f32 blend2)
			{
				const teSkeletonFrame * frames = GetFrames();
				teDualQuaternion * skin = GetSkin();

				u32 frameIndex[4] = {u32Max, u32Max, u32Max, u32Max};

				for(u32 i = 0; i < framesCount; ++i) if((frames[i].layer == pair0.x) && (frames[i].frame == pair0.y)) {frameIndex[0] = i; break;}
				for(u32 i = 0; i < framesCount; ++i) if((frames[i].layer == pair1.x) && (frames[i].frame == pair1.y)) {frameIndex[1] = i; break;}
				for(u32 i = 0; i < framesCount; ++i) if((frames[i].layer == pair2.x) && (frames[i].frame == pair2.y)) {frameIndex[2] = i; break;}
				for(u32 i = 0; i < framesCount; ++i) if((frames[i].layer == pair3.x) && (frames[i].frame == pair3.y)) {frameIndex[3] = i; break;}

				if((frameIndex[0] == u32Max) || (frameIndex[1] == u32Max) || (frameIndex[2] == u32Max) || (frameIndex[3] == u32Max))
					return false;

				for(u32 boneIndex = 0; boneIndex < bonesCount; ++boneIndex)
				{
					teQuaternionf a, a0, a1, a2, a3;
					teVector3df b, b0, b1, b2, b3;
					GetBoneKey(boneIndex, frameIndex[0])->Get(a0, b0);
					GetBoneKey(boneIndex, frameIndex[1])->Get(a1, b1);
					GetBoneKey(boneIndex, frameIndex[2])->Get(a2, b2);
					GetBoneKey(boneIndex, frameIndex[3])->Get(a3, b3);

					teQuaternionf ab0 = a0.Slerp(a1, blend0);
					teVector3df bb0 = b0.LinearInterpolation(b1, blend0);

					teQuaternionf ab1 = a2.Slerp(a3, blend1);
					teVector3df bb1 = b2.LinearInterpolation(b3, blend1);

					teDualQuaternion dqc;
					a = ab0.Slerp(ab1, blend2);
					b = bb0.LinearInterpolation(bb1, blend2);
					dqc.SetFrom(a, b);

					if(boneIndex)
					{
						teDualQuaternion dqp = skin[bones[boneIndex].parent];
						dqc.MultiplyBy(dqp);
						skin[boneIndex] = dqc;
					}
					else
						skin[boneIndex] = dqc;
				}

				for(u32 boneIndex = 0; boneIndex < bonesCount; ++boneIndex)
				{
					teDualQuaternion bp;
					GetBoneKey(boneIndex)->GetDualQuaternion(bp);
					bp.MultiplyBy(skin[boneIndex]);
					skin[boneIndex] = bp;
				}

				return true;
			}

			u1 CalculateSkin(teVector2duh pairs[], f32 times[], u8 mode)
			{
				u8 count = ((mode & 0x1) ? 1 : 0) + ((mode & 0x2) ? 1 : 0) + ((mode & 0x4) ? 1 : 0) + ((mode & 0x8) ? 1 : 0);
				u8 ind[4] = {0, 0, 0, 0};
				u8 i = 0;

				//if(mode & 0x1) ind[i++] = 0;
				//if(mode & 0x2) ind[i++] = 1;
				//if(mode & 0x4) ind[i++] = 2;
				//if(mode & 0x8) ind[i++] = 3;
				if(mode & 0x1) ind[0] = 0;
				if(mode & 0x2) ind[1] = 1;
				if(mode & 0x4) ind[2] = 2;
				if(mode & 0x8) ind[3] = 3;

				switch(count)
				{
				case 0: return false;
				case 1: return CalculateSkin(pairs[ind[0]]);
				case 2: return CalculateSkin(pairs[ind[0]], pairs[ind[1]], times[0]);
				case 3: return CalculateSkin(pairs[ind[0]], pairs[ind[1]], pairs[ind[2]], times[0], times[1]);
				case 4: return CalculateSkin(pairs[ind[0]], pairs[ind[1]], pairs[ind[2]], pairs[ind[3]], times[0], times[1], times[2]);
				default: return false;
				}
			}

		};

		#pragma pack(pop)
	}
}

#endif