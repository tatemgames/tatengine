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
			
			u1 CalculateSkin(u16 layer0, u16 frame0)
			{
				return CalculateSkin(layer0, frame0, layer0, frame0, 0.0f);

				/*
				const teSkeletonFrame * frames = GetFrames();
				
				u8 rlayer0 = 0;
				u8 rlayer1 = 0;
				
				u16 rframe0 = 0;
				u16 rframe1 = 0;
				
				u16 lastValid = 0;
				u1 findNextValid = false;

				for(u32 i = 0; i < framesCount; ++i)
				{
					if(!findNextValid)
					{
						if((frames[i].layer == layer0) && (frames[i].frame == frame0))
						{
							if(frames[i].flags)
							{
								rframe0 = lastValid;
								findNextValid = true;

							}
							else
							{
								rframe0 = i;
								rframe1 = i;
								break; 
							}
						}
					}
					
					if(!frames[i].flags)
					{
						if(findNextValid)
						{
							rframe1 = i;
							break;
						}
						else
							lastValid = i;
					}
				}
				
				f32 k;
				if (rframe1 != rframe0)
					k = (f32)(frame0 - rframe0) / (f32)(rframe1 - rframe0);
				else
					k = 0.0f;

				return CalculateSkin(rlayer0, rframe0, rlayer1, rframe1, k);
				*/
			}

			u1 CalculateSkin(u16 layer0, u16 frame0, u16 layer1, u16 frame1, f32 blend)
			{
				const teSkeletonFrame * frames = GetFrames();
				teDualQuaternion * skin = GetSkin();

				u32 frameIndex0 = u32Max, frameIndex1 = u32Max;

				for(u32 i = 0; i < framesCount; ++i) if((frames[i].layer == layer0) && (frames[i].frame == frame0)) {frameIndex0 = i; break;}
				for(u32 i = 0; i < framesCount; ++i) if((frames[i].layer == layer1) && (frames[i].frame == frame1)) {frameIndex1 = i; break;}

				if((frameIndex0 == u32Max) || (frameIndex1 == u32Max))
					return false;

				for(u32 boneIndex = 0; boneIndex < bonesCount; ++boneIndex)
				{
					teQuaternionf a, a0, a1;
					teVector3df b, b0, b1;
					
					GetBoneKey(boneIndex, frameIndex0)->Get(a0, b0);
					GetBoneKey(boneIndex, frameIndex1)->Get(a1, b1);
					
					teDualQuaternion dqc;
					a = a0.Slerp(a1, blend);
					b = b0.LinearInterpolation(b1, blend);
					
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
		};

		#pragma pack(pop)
	}
}

#endif