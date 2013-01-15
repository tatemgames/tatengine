/*
 *  teContentTools.cpp
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 12/1/11.
 *  Copyright 2011 Tatem Games. All rights reserved.
 *
 */

#include "teContentTools.h"
#include "teFileManager.h"
#include "teMemoryManager.h"
#include "teVector3D.h"
#include "teColor.h"

#include "teFastScene.h"
//#include "teRegisterActors.h"
//#include "teActorDebug.h"

namespace te
{
	namespace scene
	{
		video::teSurfaceData * CreateEmptySurface(u32 dataSize)
		{
			video::teSurfaceData * surface = (video::teSurfaceData*)TE_ALLOCATE(sizeof(video::teSurfaceData) + dataSize);
			surface->Clear();
			surface->vertexCount = 0;
			surface->indexCount = 0;
			surface->dataSize = dataSize;
			surface->indexesOffset = 0;
			surface->materialIndex = u16Max;
			surface->operationType = video::ROT_LINES;
			surface->layersIndex = u8Max;
			return surface;
		}

		u1 SaveEmptySurface(const teString & fileName, u32 dataSize)
		{
			core::IBuffer * file = core::GetFileManager()->OpenFile(fileName, core::CFileBuffer::FWM_WRITE);

			if(!file)
				return false;

			video::teSurfaceData * data = CreateEmptySurface(dataSize);

			file->Lock(core::BLT_WRITE);
			file->Allocate(sizeof(video::teSurfaceData) + data->dataSize);
			file->SetPosition(0);
			file->Write((u8*)data, sizeof(video::teSurfaceData) + data->dataSize);
			file->Unlock();
			TE_SAFE_DROP(file);

			TE_FREE(data);

			return true;
		}

		struct teGridVertex
		{
			teVector3df position;
			teColor4u color;
		};

		video::teSurfaceData * CreateGrid3D()
		{
			u8 linesCount = 31;

			u32 surfaceIndex = 0;
			u32 dataSize = sizeof(teGridVertex) * (linesCount * 4 + 6) + sizeof(u16) * (linesCount * 4 + 6);

			video::teSurfaceData * surface = (video::teSurfaceData*)TE_ALLOCATE(sizeof(video::teSurfaceData) + dataSize);
			surface->Clear();
			surface->vertexCount = 0;
			surface->indexCount = 0;
			surface->dataSize = dataSize;
			surface->indexesOffset = sizeof(teGridVertex) * (linesCount * 4 + 6);
			surface->materialIndex = u16Max;
			surface->operationType = video::ROT_LINES;
			surface->layersIndex = u8Max;

			teGridVertex * vertexes = reinterpret_cast<teGridVertex*>(surface->data);
			u16 * indexes = reinterpret_cast<u16*>(surface->data + surface->indexesOffset);

			f32 dp = 1.0f;
			f32 dpmax = dp * ((f32)linesCount) / 2.0f - dp/2.0f;
			f32 dpmin = -dpmax;

			for(u32 i = 0; i < linesCount; ++i)
			{
				f32 p = (((f32)linesCount) / 2.0f - (f32)i) * dp - dp/2.0f;

				vertexes[i * 4 + 0].position.SetXYZ(p, 0.0f, dpmin);
				vertexes[i * 4 + 1].position.SetXYZ(p, 0.0f, dpmax);
				vertexes[i * 4 + 2].position.SetXYZ(dpmin, 0.0f, p);
				vertexes[i * 4 + 3].position.SetXYZ(dpmax, 0.0f, p);

				for(u32 j = 0; j < 4; ++j)
					vertexes[i * 4 + j].color.SetRGBA(255, 255, 255, 255);

				for(u32 j = 0; j < 4; ++j)
					indexes[i * 4 + j] = i * 4 + j;

				surface->vertexCount += 4;
				surface->indexCount += 4;
			}

			vertexes = reinterpret_cast<teGridVertex*>(surface->data) + surface->vertexCount;
			indexes = reinterpret_cast<u16*>(surface->data + surface->indexesOffset) + surface->indexCount;

			vertexes[0].position.SetXYZ(0.0f, 0.01f, 0.0f);
			vertexes[1].position.SetXYZ(1.0f, 0.01f, 0.0f);
			vertexes[2].position.SetXYZ(0.0f, 0.01f, 0.0f);
			vertexes[3].position.SetXYZ(0.0f, 1.01f, 0.0f);
			vertexes[4].position.SetXYZ(0.0f, 0.01f, 0.0f);
			vertexes[5].position.SetXYZ(0.0f, 0.01f, 1.0f);

			vertexes[0].color.SetRGBA(255, 0, 0, 255);
			vertexes[1].color.SetRGBA(255, 0, 0, 255);
			vertexes[2].color.SetRGBA(0, 255, 0, 255);
			vertexes[3].color.SetRGBA(0, 255, 0, 255);
			vertexes[4].color.SetRGBA(0, 0, 255, 255);
			vertexes[5].color.SetRGBA(0, 0, 255, 255);

			indexes[0] = 0 + surface->vertexCount;
			indexes[1] = 1 + surface->vertexCount;
			indexes[2] = 2 + surface->vertexCount;
			indexes[3] = 3 + surface->vertexCount;
			indexes[4] = 4 + surface->vertexCount;
			indexes[5] = 5 + surface->vertexCount;

			surface->vertexCount += 6;
			surface->indexCount += 6;

			return surface;
		}

		u1 SaveGrid3D(const teString & fileName)
		{
			core::IBuffer * file = core::GetFileManager()->OpenFile(fileName, core::CFileBuffer::FWM_WRITE);

			if(!file)
				return false;

			video::teSurfaceData * data = CreateGrid3D();

			file->Lock(core::BLT_WRITE);
			file->Allocate(sizeof(video::teSurfaceData) + data->dataSize);
			file->SetPosition(0);
			file->Write((u8*)data, sizeof(video::teSurfaceData) + data->dataSize);
			file->Unlock();
			TE_SAFE_DROP(file);

			TE_FREE(data);

			return true;
		}

		void CalculateSurfaceAABB(const video::teSurfaceData * data, const video::teSurfaceLayers & layers, teAABB3df & aabb)
		{
			aabb.Flush();

			u32 stride = layers.stride[video::SLT_POSITION];
			u8 varCount = layers.variablesPerObject[video::SLT_POSITION];

			TE_ASSERT(stride);
			TE_ASSERT(varCount);

			#define TE_PROCESS_POS(__type) \
				{ \
					teVector3df pos; \
					for(u32 i = 0; i < data->vertexCount; ++i) \
					{ \
						const __type * ptr = reinterpret_cast<const __type*>(data->data + layers.offset[video::SLT_POSITION] + stride * i); \
						pos.SetXYZ((f32)ptr[0], (varCount >= 2 ? (f32)ptr[1] : 0.0f), (varCount >= 3 ? (f32)ptr[2] : 0.0f)); \
						aabb.Unite(teAABB3df(pos, pos)); \
					} \
				}

			switch(layers.variableType[video::SLT_POSITION])
			{
			case video::SVT_BYTE: TE_PROCESS_POS(s8); break;
			case video::SVT_UNSIGNED_BYTE: TE_PROCESS_POS(u8); break;
			case video::SVT_SHORT: TE_PROCESS_POS(s16); break;
			case video::SVT_UNSIGNED_SHORT: TE_PROCESS_POS(u16); break;
			case video::SVT_INT: TE_PROCESS_POS(s32); break;
			case video::SVT_UNSIGNED_INT: TE_PROCESS_POS(u32); break;
			case video::SVT_HALF_FLOAT: TE_ASSERT(0); break; // not implemented
			case video::SVT_FLOAT: TE_PROCESS_POS(f32); break;
			case video::SVT_DOUBLE: TE_PROCESS_POS(f64); break;
			default: TE_ASSERT(0); break;
			}

			#undef TE_PROCESS_POS

			if(TE_GET_BIT(layers.normalized, video::SLT_POSITION))
			{
				switch(layers.variableType[video::SLT_POSITION])
				{
				case video::SVT_BYTE: aabb.SetEdges((aabb.edgeMin * 2.0f + 1.0f) / ((f32)u8Max - 1.0f), (aabb.edgeMax * 2.0f + 1.0f) / ((f32)u8Max - 1.0f)); break;
				case video::SVT_UNSIGNED_BYTE: aabb.SetEdges(aabb.edgeMin / ((f32)u8Max - 1.0f), aabb.edgeMax / ((f32)u8Max - 1.0f)); break;
				case video::SVT_SHORT: aabb.SetEdges((aabb.edgeMin * 2.0f + 1.0f) / ((f32)u16Max - 1.0f), (aabb.edgeMax * 2.0f + 1.0f) / ((f32)u16Max - 1.0f)); break;
				case video::SVT_UNSIGNED_SHORT: aabb.SetEdges(aabb.edgeMin / ((f32)u16Max - 1.0f), aabb.edgeMax / ((f32)u16Max - 1.0f)); break;
				case video::SVT_INT: aabb.SetEdges((aabb.edgeMin * 2.0f + 1.0f) / ((f32)u32Max - 1.0f), (aabb.edgeMax * 2.0f + 1.0f) / ((f32)u32Max - 1.0f)); break;
				case video::SVT_UNSIGNED_INT: aabb.SetEdges(aabb.edgeMin / ((f32)u32Max - 1.0f), aabb.edgeMax / ((f32)u32Max - 1.0f)); break;
				case video::SVT_HALF_FLOAT: TE_ASSERT(0); break;
				case video::SVT_FLOAT: TE_ASSERT(0); break;
				case video::SVT_DOUBLE: TE_ASSERT(0); break;
				default: TE_ASSERT(0); break;
				}
			}
		}
		
		
		void CalculateSurfaceAABBSkeleton(teSkeleton * skeleton, const video::teSurfaceData * data, const video::teSurfaceLayers & layers, teAABB3df & aabb)
		{
			aabb.Flush();

			u32 stride = layers.stride[video::SLT_POSITION];
			u8 varCount = layers.variablesPerObject[video::SLT_POSITION];

			TE_ASSERT(stride);
			TE_ASSERT(varCount);

			#define TE_PROCESS_POS_SKEL(__type) \
			{ \
				const __type * ptrPos = reinterpret_cast<const __type*>(data->Get(layers, video::SLT_POSITION, k)); \
				pos.SetXYZ((f32)ptrPos[0], (varCount >= 2 ? (f32)ptrPos[1] : 0.0f), (varCount >= 3 ? (f32)ptrPos[2] : 0.0f)); \
			}

			TE_ASSERT(!TE_GET_BIT(layers.normalized, video::SLT_POSITION));

			TE_ASSERT(layers.variableType[video::SLT_BONES_IDS] == video::SVT_UNSIGNED_BYTE);
			TE_ASSERT(layers.variableType[video::SLT_BONES_W] == video::SVT_FLOAT);

			for(u32 i = 0; i < skeleton->framesCount; i++)
			{
				skeleton->CalculateSkin(0, i);
				teDualQuaternion * skin = skeleton->GetSkin();

				teVector3df pos;
				teDualQuaternion resQuat, tempQuat;

				for(u32 k = 0; k< data->vertexCount; k++)
				{
					switch(layers.variableType[video::SLT_POSITION])
					{
						case video::SVT_BYTE: TE_PROCESS_POS_SKEL(s8); break;
						case video::SVT_UNSIGNED_BYTE: TE_PROCESS_POS_SKEL(u8); break;
						case video::SVT_SHORT: TE_PROCESS_POS_SKEL(s16); break;
						case video::SVT_UNSIGNED_SHORT: TE_PROCESS_POS_SKEL(u16); break;
						case video::SVT_INT: TE_PROCESS_POS_SKEL(s32); break;
						case video::SVT_UNSIGNED_INT: TE_PROCESS_POS_SKEL(u32); break;
						case video::SVT_HALF_FLOAT: TE_ASSERT(0); break; // not implemented
						case video::SVT_FLOAT: TE_PROCESS_POS_SKEL(f32); break;
						case video::SVT_DOUBLE: TE_PROCESS_POS_SKEL(f64); break;
						default: TE_ASSERT(0); break;
					}

					const u8 * ptrBoneInds = reinterpret_cast<const u8*>(data->Get(layers, video::SLT_BONES_IDS, k));
					const f32 * ptrWeights = reinterpret_cast<const f32*>(data->Get(layers, video::SLT_BONES_W, k));

					for(u8 j = 0; j < 4; j++)
					{
						if (ptrBoneInds[j] < u8Max)
						{
							tempQuat = skin[ptrBoneInds[j]];
							tempQuat.MultiplyBy(ptrWeights[j]);
							resQuat.AddBy(tempQuat);
						}
					}

					f32 qlength = resQuat.a.GetLength();
					resQuat.a /= qlength;
					resQuat.b /= qlength;

					teMatrix4f bone = teMatrix4f(
									1.0f - 2.0f * (resQuat.a.y*resQuat.a.y+resQuat.a.z*resQuat.a.z),
									2.0f * (resQuat.a.x*resQuat.a.y+resQuat.a.z*resQuat.a.w),
									2.0f * (resQuat.a.x*resQuat.a.z-resQuat.a.y*resQuat.a.w),
									0.0f,
									2.0f * (resQuat.a.x*resQuat.a.y-resQuat.a.z*resQuat.a.w),
									1.0f - 2.0f * (resQuat.a.x*resQuat.a.x+resQuat.a.z*resQuat.a.z),
									2.0f * (resQuat.a.z*resQuat.a.y+resQuat.a.x*resQuat.a.w),
									0.0f,
									2.0f * (resQuat.a.x*resQuat.a.z+resQuat.a.y*resQuat.a.w),
									2.0f * (resQuat.a.z*resQuat.a.y-resQuat.a.x*resQuat.a.w),
									1.0f - 2.0f * (resQuat.a.x*resQuat.a.x+resQuat.a.y*resQuat.a.y),
									0.0f,
									2.0f * (-resQuat.b.w*resQuat.a.x + resQuat.b.x*resQuat.a.w - resQuat.b.y*resQuat.a.z + resQuat.b.z*resQuat.a.y),
									2.0f * (-resQuat.b.w*resQuat.a.y + resQuat.b.x*resQuat.a.z + resQuat.b.y*resQuat.a.w - resQuat.b.z*resQuat.a.x),
									2.0f * (-resQuat.b.w*resQuat.a.z - resQuat.b.x*resQuat.a.y + resQuat.b.y*resQuat.a.x + resQuat.b.z*resQuat.a.w),
									1.0f
									);

					teMatrix4f bonet;
					bone.Transpose(bonet);

					pos = bonet.MultiplyMatrixOnVector3D(pos);

					if(!i)
						aabb = teAABB3df(pos, pos);
					else
						aabb.Unite(teAABB3df(pos, pos));
				}
			}

			#undef TE_PROCESS_POS_SKEL
		}

		/*
		void AnalizeSkelAnimation4(teSkeleton * skel_data)
		{
			#define TE_ANALIZE_STEP 26
			
			TE_ASSERT(TE_ANALIZE_STEP > 3);
			TE_ASSERT(skel_data->framesCount >= TE_ANALIZE_STEP + 1);
		
			u32 beginIndex = 0;
			u32 endIndex = beginIndex + TE_ANALIZE_STEP;
						
			const teSkeletonBoneKey * boneKey;
			const teSkeletonBoneKey * boneKey1;
			const teSkeletonBoneKey * boneKey2;
			
			f32 errorRate[7]; // relation of max error to average val 

			f32 maxErrors[7];
			u32 errorsIDs[7];
			long double errorsSum[7]; 
			//f32 minErrors[7];
			
			u32 counter;
			
			teQuaternionf slerpVal, tempQuat, quat1, quat2, quatError;
			
			teVector3df lerpVal, tempVec, vec1, vec2, vecError;
			
			u32 bonesKeysFrameCount = 0; // key counter
			
			u1 isOk = 0;
			
			//----- get statistic for all animation
			teSkeletonBoneKey critAv;
			teSkeletonBoneKey critDev;
			GetCrit(skel_data, &critAv, &critDev);
			
			//------- set all flags to 0
			for(int k = 0; k<skel_data->framesCount; ++k)
			{	
				const_cast<teSkeletonFrame*>(const_cast<teSkeleton*>(skel_data)->GetFrames())[k].flags = 0;
			}
			
			bonesKeysFrameCount++; // first key always exist
			
			while (!isOk)
			{
				memset(maxErrors, 0, sizeof(f32) * 7);
				for (int i = 0; i < 7; i++) errorsIDs[i] = beginIndex + 1;
				
				memset(errorsSum, 0, sizeof(double long) * 7);
				
				counter = 0;
				
				for (int j = beginIndex + 1; j < endIndex; j++) // for values in lerp segment 
				{
	
					for (int i = 0; i < skel_data->bonesCount; i++) // for each bones
					{					
						
						boneKey1 = skel_data->GetBoneKey(i, beginIndex);
						boneKey2 = skel_data->GetBoneKey(i, endIndex);
						quat1 = teQuaternionf(boneKey1->rotation[0], boneKey1->rotation[1], boneKey1->rotation[2],  boneKey1->rotation[3]);
						quat2 = teQuaternionf(boneKey2->rotation[0], boneKey2->rotation[1], boneKey2->rotation[2],  boneKey2->rotation[3]);
						vec1  = teVector3df(boneKey1->translation[0], boneKey1->translation[1], boneKey1->translation[2]);
						vec2  = teVector3df(boneKey2->translation[0], boneKey2->translation[1], boneKey2->translation[2]);

						boneKey = skel_data->GetBoneKey(i, j);
						
						//----- SPHERICAL INTERP------------------
						tempQuat = teQuaternionf(boneKey->rotation[0], boneKey->rotation[1], boneKey->rotation[2],  boneKey->rotation[3]);
						
						slerpVal = quat1.Slerp(quat2, (f32)j/((f32)(endIndex - beginIndex)));
						
						quatError = slerpVal - tempQuat;
						
						quatError.x = teAbs(quatError.x);
						quatError.y = teAbs(quatError.y);
						quatError.z = teAbs(quatError.z);
						quatError.w = teAbs(quatError.w);
						
						//----- LINE INTERP------------------------
						tempVec = teVector3df(boneKey->translation[0], boneKey->translation[1], boneKey->translation[2]);
						
						lerpVal = vec1.LinearInterpolation(vec2, (f32)j/((f32)(endIndex - beginIndex)));
						
						vecError = lerpVal - tempVec;
						
						vecError.x = teAbs(vecError.x);
						vecError.y = teAbs(vecError.y);
						vecError.z = teAbs(vecError.z);
						
						//------ save max errors 	
						if (maxErrors[0] < quatError.x){ maxErrors[0] = quatError.x; errorsIDs[0] = j;}
						if (maxErrors[1] < quatError.y){ maxErrors[1] = quatError.y; errorsIDs[1] = j;}
						if (maxErrors[2] < quatError.z){ maxErrors[2] = quatError.z; errorsIDs[2] = j;}
						if (maxErrors[3] < quatError.w){ maxErrors[3] = quatError.w; errorsIDs[3] = j;}
						if (maxErrors[4] < vecError.x) {maxErrors[4] = vecError.x; errorsIDs[4] = j;}
						if (maxErrors[5] < vecError.y) {maxErrors[5] = vecError.y; errorsIDs[5] = j;}
						if (maxErrors[6] < vecError.z) {maxErrors[6] = vecError.z; errorsIDs[6] = j;}
						
						//------ save sum errors 
						errorsSum[0] += quatError.x;
						errorsSum[1] += quatError.y;
						errorsSum[2] += quatError.z;
						errorsSum[3] += quatError.w;
						errorsSum[4] += vecError.x;
						errorsSum[5] += vecError.y;
						errorsSum[6] += vecError.z;
						counter++;

					}// for bones
					
				} // for frames segment
				
				errorRate[0] = maxErrors[0]/(errorsSum[0]/(f32)counter);///critAv.rotation[0];
				errorRate[1] = maxErrors[1]/(errorsSum[1]/(f32)counter);///critAv.rotation[1];
				errorRate[2] = maxErrors[2]/(errorsSum[2]/(f32)counter);///critAv.rotation[2];
				errorRate[3] = maxErrors[3]/(errorsSum[3]/(f32)counter);///critAv.rotation[3];
				errorRate[4] = maxErrors[4]/(errorsSum[4]/(f32)counter);///critAv.translation[0];
				errorRate[5] = maxErrors[5]/(errorsSum[5]/(f32)counter);///critAv.translation[1];
				errorRate[6] = maxErrors[6]/(errorsSum[6]/(f32)counter);///critAv.translation[2];
				
				f32 maxRate = errorRate[0]; u32 maxRateID = 0;
				for (int i = 1; i < 7; i++)
					if (maxRate < errorRate[i])
					{
						maxRate = errorRate[i];
						maxRateID = i;
					}
			
				
				for(int k = beginIndex+1; k < errorsIDs[maxRateID]; ++k)
				{	
					// IF KEY k NOT BEGIN OR END  OF ANIMATION
					const_cast<teSkeletonFrame*>(const_cast<teSkeleton*>(skel_data)->GetFrames())[k].flags = 1;
				}				
					
				beginIndex = beginIndex = errorsIDs[maxRateID];
				endIndex = beginIndex + TE_ANALIZE_STEP;
				bonesKeysFrameCount++;
		
				if (endIndex >= skel_data->framesCount)
				{
					bonesKeysFrameCount++;
					isOk = 1;
				}
				
			} // isOk
		
			
			// TO DO : Additional check for BEGIN and END OF ANIMATION in skelet (set flag to 0)
			
			
			for(int k = 0; k<skel_data->framesCount; ++k)
			{	
				printf("%i, ", skel_data->GetFrames()[k].flags);
			}
			
			printf("\n framesCount before: %d framesCount after: %d", skel_data->framesCount, bonesKeysFrameCount);
			printf("\n Total key * bones: %d", bonesKeysFrameCount * skel_data->bonesCount);
			printf("\n Original key * bones: %d", skel_data->bonesCount * skel_data->framesCount);
			
		} // AnalizeSkelAnimation4
	
		void AnalizeSkelAnimationByBones(teSkeleton * skel_data)
		{
			#define TE_ANALIZE_STEP 26
			
			TE_ASSERT(TE_ANALIZE_STEP > 3);
			TE_ASSERT(skel_data->framesCount >= TE_ANALIZE_STEP + 1);
			
			u32 beginIndex = 0;
			u32 endIndex = beginIndex + TE_ANALIZE_STEP;
			
			const teSkeletonBoneKey * boneKey;
			const teSkeletonBoneKey * boneKey1;
			const teSkeletonBoneKey * boneKey2;
			
			f32 errorRate[7]; // relation of max error to average val 
			
			f32 maxErrors[7];
			u32 errorsIDs[7];
			long double errorsSum[7]; 
			//f32 minErrors[7];
			
			u32 counter;
			
			teQuaternionf slerpVal, tempQuat, quat1, quat2, quatError;
			
			teVector3df lerpVal, tempVec, vec1, vec2, vecError;
			
			u32 bonesKeysFrameCount[skel_data->bonesCount]; // key counter
			//memset(bonesKeysFrameCount, 0, skel_data->bonesCount * sizeof(u32));
			
			
			u1 isOk = 0;
			
			//----- get statistic for all animation
			teSkeletonBoneKey critAv;
			teSkeletonBoneKey critDev;
			GetCrit(skel_data, &critAv, &critDev);
			
			//------- set all flags to 0
			for(int k = 0; k<skel_data->framesCount; ++k)
			{	
				const_cast<teSkeletonFrame*>(const_cast<teSkeleton*>(skel_data)->GetFrames())[k].flags = 0;
			}
			
			for (int i = 0; i < skel_data->bonesCount; i++)
				bonesKeysFrameCount[i] = 1; // first key always exist
			
			
				for (int i = 0; i < skel_data->bonesCount; i++) // for each bones
				{
					isOk = 0;
					
					u32 beginIndex = 0;
					u32 endIndex = beginIndex + TE_ANALIZE_STEP;
					
					
					
					
					while (!isOk)
					{
						memset(maxErrors, 0, sizeof(f32) * 7);
						for (int l = 0; l < 7; l++) errorsIDs[l] = beginIndex + 1;
				
						memset(errorsSum, 0, sizeof(double long) * 7);
				
						counter = 0;
						
						for (int j = beginIndex + 1; j < endIndex; j++) // for values in lerp segment 
						{
							boneKey1 = skel_data->GetBoneKey(i, beginIndex);
							boneKey2 = skel_data->GetBoneKey(i, endIndex);
							quat1 = teQuaternionf(boneKey1->rotation[0], boneKey1->rotation[1], boneKey1->rotation[2],  boneKey1->rotation[3]);
							quat2 = teQuaternionf(boneKey2->rotation[0], boneKey2->rotation[1], boneKey2->rotation[2],  boneKey2->rotation[3]);
							vec1  = teVector3df(boneKey1->translation[0], boneKey1->translation[1], boneKey1->translation[2]);
							vec2  = teVector3df(boneKey2->translation[0], boneKey2->translation[1], boneKey2->translation[2]);
							
							boneKey = skel_data->GetBoneKey(i, j);
							
							//----- SPHERICAL INTERP------------------
							tempQuat = teQuaternionf(boneKey->rotation[0], boneKey->rotation[1], boneKey->rotation[2],  boneKey->rotation[3]);
							
							slerpVal = quat1.Slerp(quat2, (f32)j/((f32)(endIndex - beginIndex)));
							
							quatError = slerpVal - tempQuat;
							
							quatError.x = teAbs(quatError.x);
							quatError.y = teAbs(quatError.y);
							quatError.z = teAbs(quatError.z);
							quatError.w = teAbs(quatError.w);
							
							//----- LINE INTERP------------------------
							tempVec = teVector3df(boneKey->translation[0], boneKey->translation[1], boneKey->translation[2]);
							
							lerpVal = vec1.LinearInterpolation(vec2, (f32)j/((f32)(endIndex - beginIndex)));
							
							vecError = lerpVal - tempVec;
							
							vecError.x = teAbs(vecError.x);
							vecError.y = teAbs(vecError.y);
							vecError.z = teAbs(vecError.z);
							
							//------ save max errors 	
							if (maxErrors[0] < quatError.x){ maxErrors[0] = quatError.x; errorsIDs[0] = j;}
							if (maxErrors[1] < quatError.y){ maxErrors[1] = quatError.y; errorsIDs[1] = j;}
							if (maxErrors[2] < quatError.z){ maxErrors[2] = quatError.z; errorsIDs[2] = j;}
							if (maxErrors[3] < quatError.w){ maxErrors[3] = quatError.w; errorsIDs[3] = j;}
							if (maxErrors[4] < vecError.x) {maxErrors[4] = vecError.x; errorsIDs[4] = j;}
							if (maxErrors[5] < vecError.y) {maxErrors[5] = vecError.y; errorsIDs[5] = j;}
							if (maxErrors[6] < vecError.z) {maxErrors[6] = vecError.z; errorsIDs[6] = j;}
							
							//------ save sum errors 
							errorsSum[0] += quatError.x;
							errorsSum[1] += quatError.y;
							errorsSum[2] += quatError.z;
							errorsSum[3] += quatError.w;
							errorsSum[4] += vecError.x;
							errorsSum[5] += vecError.y;
							errorsSum[6] += vecError.z;
							counter++;
							
						}// for frames segment
					
						errorRate[0] = maxErrors[0]/(errorsSum[0]/(f32)counter);
						errorRate[1] = maxErrors[1]/(errorsSum[1]/(f32)counter);
						errorRate[2] = maxErrors[2]/(errorsSum[2]/(f32)counter);
						errorRate[3] = maxErrors[3]/(errorsSum[3]/(f32)counter);
						errorRate[4] = maxErrors[4]/(errorsSum[4]/(f32)counter);
						errorRate[5] = maxErrors[5]/(errorsSum[5]/(f32)counter);
						errorRate[6] = maxErrors[6]/(errorsSum[6]/(f32)counter);
					
					
						f32 maxRate = errorRate[0]; u32 maxRateID = 0;
						for (int k = 1; k < 7; k++)
							if (maxRate < errorRate[k])
							{
								maxRate = errorRate[k];
								maxRateID = k;
							}
					
						for(int k = beginIndex+1; k < errorsIDs[maxRateID]; ++k)
						{	
						// SET KEY FOR BONE THERE
						// IF KEY k NOT BEGIN OR END  OF ANIMATION
						//const_cast<teSkeletonFrame*>(const_cast<teSkeleton*>(skel_data)->GetFrames())[k].flags = 1;
						}
					
						beginIndex = beginIndex = errorsIDs[maxRateID];
						endIndex = beginIndex + TE_ANALIZE_STEP;
						bonesKeysFrameCount[i]++;
					
						if (endIndex >= skel_data->framesCount)
						{
							bonesKeysFrameCount[i]++;
							isOk = 1;
						}
					
					} // isOk
					
					
				} // for bones
		
			// TO DO : Additional check for BEGIN and END OF ANIMATION in skelet (set flag to 0)
		
			//for(int k = 0; k<skel_data->framesCount; ++k)
			//{	
			//	printf("%i, ", skel_data->GetFrames()[k].flags);
			//}
			
			for(int k = 0; k<skel_data->bonesCount; ++k)
			{	
				printf("\n Bone Keys  %d: %d", k, bonesKeysFrameCount[k]);
			}
			
			u32 totalKeyBones = 0;
			for(int k = 0; k < skel_data->bonesCount; k++)
				totalKeyBones += bonesKeysFrameCount[k];
			
			printf("\n Total key * bones: %d", totalKeyBones);
			
			//printf("\n framesCount before: %d framesCount after: %d", skel_data->framesCount, bonesKeysFrameCount);
			
		} //AnalizeSkelAnimationByBones
		
		
		
		void GetCrit(teSkeleton * skel_data, teSkeletonBoneKey * resultAv, teSkeletonBoneKey * resultDev)
		{
			memset(resultAv, 0, sizeof(teSkeletonBoneKey));
			memset(resultDev, 0, sizeof(teSkeletonBoneKey));
			

			teSkeletonBoneKey tempAv;
			teSkeletonBoneKey tempDev; 
			
			long double tempSum[7];
			memset(tempSum, 0, sizeof(long double) * 7);
			
			long double t_count = skel_data->framesCount * skel_data->bonesCount;

			
			const teSkeletonBoneKey * boneKey;
			
			//----- sum
			for (int i = 0; i < skel_data->framesCount; i++)  // for each cadr
			{
				for (int j = 0; j < skel_data->bonesCount; j++)  // for each bone
				{
					boneKey = skel_data->GetBoneKey(j, i);
					
					tempSum[0] += boneKey->rotation[0]; 
					tempSum[1] += boneKey->rotation[1];
					tempSum[2] += boneKey->rotation[2];
					tempSum[3] += boneKey->rotation[3];
					tempSum[4] += boneKey->translation[0];
					tempSum[5] += boneKey->translation[1];
					tempSum[6] += boneKey->translation[2];
				}
			}

			
			//------ average
			tempAv.rotation[0] = tempSum[0]/t_count;
			tempAv.rotation[1] = tempSum[1]/t_count;
			tempAv.rotation[2] = tempSum[2]/t_count;
			tempAv.rotation[3] = tempSum[3]/t_count;
			tempAv.translation[0] = tempSum[4]/t_count;
			tempAv.translation[1] = tempSum[5]/t_count;
			tempAv.translation[2] = tempSum[6]/t_count;
					
					
			//-----sum of (Xi - Xav)^2
			#define TE_QRT(a, b) (a - b)*(a - b)

			memset(tempSum, 0, sizeof(long double) * 7);
			for (int i = 0; i < skel_data->framesCount; i++)  // for each cadr
			{
				for (int j = 0; j < skel_data->bonesCount; j++)  // for each bone
				{
						boneKey = skel_data->GetBoneKey(j, i);
					
						tempSum[0] += TE_QRT(boneKey->rotation[0], tempAv.rotation[0]);
						tempSum[1] += TE_QRT(boneKey->rotation[1], tempAv.rotation[1]);
						tempSum[2] += TE_QRT(boneKey->rotation[2], tempAv.rotation[2]);
						tempSum[3] += TE_QRT(boneKey->rotation[3], tempAv.rotation[3]);
						tempSum[4] += TE_QRT(boneKey->translation[0], tempAv.translation[0]);
						tempSum[5] += TE_QRT(boneKey->translation[1], tempAv.translation[1]);
						tempSum[6] += TE_QRT(boneKey->translation[2], tempAv.translation[2]);
				}
			}
			#undef TE_QRT
					
			//----dev---
			f32 temp = 1.0f/t_count;
			tempDev.rotation[0] = teSqrt(temp * tempSum[0]);
			tempDev.rotation[1] = teSqrt(temp * tempSum[1]);
			tempDev.rotation[2] = teSqrt(temp * tempSum[2]);
			tempDev.rotation[3] = teSqrt(temp * tempSum[3]);
			tempDev.translation[0] = teSqrt(temp * tempSum[4]);
			tempDev.translation[1] = teSqrt(temp * tempSum[5]);
			tempDev.translation[2] = teSqrt(temp * tempSum[6]);
			
			memcpy(resultAv, &tempAv, sizeof(teSkeletonBoneKey));
			memcpy(resultDev, &tempDev, sizeof(teSkeletonBoneKey));
		
		} // get crit
		
		
		void GetCritInSegment(teSkeleton * skel_data, teSkeletonBoneKey * resultAv, teSkeletonBoneKey * resultDev, u32 frameBegin, u32 frameEnd)
		{
			memset(resultAv, 0, sizeof(teSkeletonBoneKey));
			memset(resultDev, 0, sizeof(teSkeletonBoneKey));
			
			teSkeletonBoneKey tempAv;
			teSkeletonBoneKey tempDev; 
			
			long double tempSum[7];
			memset(tempSum, 0, sizeof(long double) * 7);
			
			long double t_count = (frameEnd - frameBegin) * skel_data->bonesCount;
			
			
			const teSkeletonBoneKey * boneKey;
			
			//----- sum
			for (int i = frameBegin; i < frameEnd; i++)  // for each cadr
			{
				for (int j = 0; j < skel_data->bonesCount; j++)  // for each bone
				{
					boneKey = skel_data->GetBoneKey(j, i);
					
					tempSum[0] += boneKey->rotation[0]; 
					tempSum[1] += boneKey->rotation[1];
					tempSum[2] += boneKey->rotation[2];
					tempSum[3] += boneKey->rotation[3];
					tempSum[4] += boneKey->translation[0];
					tempSum[5] += boneKey->translation[1];
					tempSum[6] += boneKey->translation[2];
				}
			}
			
			
			//------ average
			tempAv.rotation[0] = tempSum[0]/t_count;
			tempAv.rotation[1] = tempSum[1]/t_count;
			tempAv.rotation[2] = tempSum[2]/t_count;
			tempAv.rotation[3] = tempSum[3]/t_count;
			tempAv.translation[0] = tempSum[4]/t_count;
			tempAv.translation[1] = tempSum[5]/t_count;
			tempAv.translation[2] = tempSum[6]/t_count;
			
			
			//-----sum of (Xi - Xav)^2
			#define TE_QRT(a, b) (a - b)*(a - b)
			
			memset(tempSum, 0, sizeof(long double) * 7);
			
			for (int i = frameBegin; i < frameEnd; i++)  // for each cadr
			{
				for (int j = 0; j < skel_data->bonesCount; j++)  // for each bone
				{
					boneKey = skel_data->GetBoneKey(j, i);
					
					tempSum[0] += TE_QRT(boneKey->rotation[0], tempAv.rotation[0]);
					tempSum[1] += TE_QRT(boneKey->rotation[1], tempAv.rotation[1]);
					tempSum[2] += TE_QRT(boneKey->rotation[2], tempAv.rotation[2]);
					tempSum[3] += TE_QRT(boneKey->rotation[3], tempAv.rotation[3]);
					tempSum[4] += TE_QRT(boneKey->translation[0], tempAv.translation[0]);
					tempSum[5] += TE_QRT(boneKey->translation[1], tempAv.translation[1]);
					tempSum[6] += TE_QRT(boneKey->translation[2], tempAv.translation[2]);
				}
			}
			#undef TE_QRT
			
			//----dev---
			f32 temp = 1.0f/t_count;
			tempDev.rotation[0] = teSqrt(temp * tempSum[0]);
			tempDev.rotation[1] = teSqrt(temp * tempSum[1]);
			tempDev.rotation[2] = teSqrt(temp * tempSum[2]);
			tempDev.rotation[3] = teSqrt(temp * tempSum[3]);
			tempDev.translation[0] = teSqrt(temp * tempSum[4]);
			tempDev.translation[1] = teSqrt(temp * tempSum[5]);
			tempDev.translation[2] = teSqrt(temp * tempSum[6]);
			
			memcpy(resultAv, &tempAv, sizeof(teSkeletonBoneKey));
			memcpy(resultDev, &tempDev, sizeof(teSkeletonBoneKey));
			
		} // get crit in seg ()
		
		*/
	}
}
