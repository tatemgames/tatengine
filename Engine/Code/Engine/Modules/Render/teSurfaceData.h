/*
 *  teSurfaceData.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 12/1/11.
 *  Copyright 2011 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TESURFACEDATA_H
#define TE_TESURFACEDATA_H

#include "teTypes.h"
#include "teDebug.h"
#include "teRenderSystem.h"
#include "teMath.h"

namespace te
{
	namespace video
	{
		enum ERenderOperationType
		{
			ROT_POINTS = 0,
			ROT_LINES,
			ROT_LINE_LOOP,
			ROT_LINE_STRIP,
			ROT_TRIANGLES,
			ROT_TRIANGLE_STRIP,
			ROT_FAN
		};

		#if defined(TE_RENDER_GL)
		TE_FUNC GLenum GetRenderOperationGLType(ERenderOperationType type)
		{
			switch(type)
			{
			case ROT_POINTS:			return GL_POINTS;
			case ROT_LINES:				return GL_LINES;
			case ROT_LINE_LOOP:			return GL_LINE_LOOP;
			case ROT_LINE_STRIP:		return GL_LINE_STRIP;
			case ROT_TRIANGLES:			return GL_TRIANGLES;
			case ROT_TRIANGLE_STRIP:	return GL_TRIANGLE_STRIP;
			case ROT_FAN:				return GL_TRIANGLE_FAN;
			default: TE_ASSERT(false);	return GL_NONE;
			}
		}
		#endif

		enum ESurfaceLayerType // 12 layers for align in teSurfaceLayers
		{
			SLT_POSITION = 0,
			SLT_NORMAL,
			SLT_UV_0,
			SLT_UV_1,
			SLT_UV_2,
			SLT_UV_3,
			SLT_COLOR,
			SLT_TANGENT,
			SLT_BONES_IDS,
			SLT_BONES_W,
			SLT_USER_0,
			SLT_USER_1,
			SLT_USER_2,
			SLT_INDEXES,
			SLT_MAX
		};

		enum ESurfaceVariableType
		{
			SVT_BYTE = 0,
			SVT_UNSIGNED_BYTE,
			SVT_SHORT,
			SVT_UNSIGNED_SHORT,
			SVT_INT,
			SVT_UNSIGNED_INT,
			SVT_HALF_FLOAT,
			SVT_FLOAT,
			SVT_DOUBLE
		};

		#if defined(TE_RENDER_GL)
		TE_FUNC GLenum GetSurfaceVariableGLType(ESurfaceVariableType type)
		{
			switch(type)
			{
			case SVT_BYTE:				return GL_BYTE;
			case SVT_UNSIGNED_BYTE:		return GL_UNSIGNED_BYTE;
			case SVT_SHORT:				return GL_SHORT;
			case SVT_UNSIGNED_SHORT:	return GL_UNSIGNED_SHORT;
			case SVT_UNSIGNED_INT:		return GL_UNSIGNED_INT;
			case SVT_FLOAT:				return GL_FLOAT;
			#if defined(TE_OPENGL_15) || defined(TE_OPENGL_21)
			case SVT_INT:				return GL_INT;
			case SVT_HALF_FLOAT:		return GL_HALF_FLOAT;
			case SVT_DOUBLE:			return GL_DOUBLE;
			#endif
			default: TE_ASSERT(false);	return GL_NONE;
			}
		}
		#endif

		TE_FUNC u8 GetSurfaceVariableSize(ESurfaceVariableType type)
		{
			switch(type)
			{
			case SVT_BYTE:				return 1;
			case SVT_UNSIGNED_BYTE:		return 1;
			case SVT_SHORT:				return 2;
			case SVT_UNSIGNED_SHORT:	return 2;
			case SVT_INT:				return 4;
			case SVT_UNSIGNED_INT:		return 4;
			case SVT_HALF_FLOAT:		return 2;
			case SVT_FLOAT:				return 4;
			case SVT_DOUBLE:			return 8;
			default: TE_ASSERT(false);	return 0;
			}
		}

		struct teSurfaceLayers
		{
			u8 variablesPerObject[SLT_MAX];
			u8 variableType[SLT_MAX];
			u16 stride[SLT_MAX];
			u32 offset[SLT_MAX];
			u32 normalized;

			TE_INLINE void Clear()
			{
				for(u8 i = 0; i < SLT_MAX; ++i)
				{
					variablesPerObject[i] = 0;
					variableType[i] = 0;
					stride[i] = 0;
					offset[i] = 0;
					normalized = 0;
				}
			}

			TE_INLINE void SetLayer(ESurfaceLayerType layer, u8 setVariablesPerObject, ESurfaceVariableType setVariableType, u16 setStride, u32 setOffset, u1 setNormalized)
			{
				variablesPerObject[layer] = setVariablesPerObject;
				variableType[layer] = setVariableType;
				stride[layer] = setStride;
				offset[layer] = setOffset;
				TE_SET_BIT(normalized, layer, setNormalized);
			}

			#if defined(TE_RENDER_GL)
			TE_INLINE GLenum GetVariableGLType(u8 layer) const {return GetSurfaceVariableGLType((ESurfaceVariableType)variableType[layer]);}
			#endif

			TE_INLINE u8 GetVariableSize(u8 layer) const {return GetSurfaceVariableSize((ESurfaceVariableType)variableType[layer]);}
		};

		#pragma pack(push, 1)

		struct teSurfaceData
		{
			u32 materialIndex;
			u32 skeletonIndex;
			u32 vertexCount, indexCount;
			u32 dataSize;
			u32 indexesOffset;
			u16 reserved;
			u8 operationType;
			u8 layersIndex;
			u8 data[];

			#if defined(TE_RENDER_GL)
			TE_INLINE GLenum GetOperationGLType() const {return GetRenderOperationGLType((ERenderOperationType)operationType);}
			#endif

			TE_INLINE u8 * Get(const teSurfaceLayers & layers, ESurfaceLayerType layer, u32 index = 0) {return data + layers.offset[layer] + layers.stride[layer] * index + (layer == SLT_INDEXES ? indexesOffset : 0);}
			TE_INLINE const u8 * Get(const teSurfaceLayers & layers, ESurfaceLayerType layer, u32 index = 0) const {return data + layers.offset[layer] + layers.stride[layer] * index + (layer == SLT_INDEXES ? indexesOffset : 0);}

			TE_INLINE u8 * GetVertexes(){return data;}
			TE_INLINE const u8 * GetVertexes() const {return data;}

			TE_INLINE u8 * GetIndexes(const teSurfaceLayers & layers){return data + indexesOffset + layers.offset[SLT_INDEXES];}
			TE_INLINE const u8 * GetIndexes(const teSurfaceLayers & layers) const {return data + indexesOffset + layers.offset[SLT_INDEXES];}

			TE_INLINE u1 IsMaterialValid() const {return materialIndex != u32Max;}
			TE_INLINE u1 IsEmpty() const {return !indexCount;}

			TE_INLINE void Clear()
			{
				materialIndex = u32Max;
				skeletonIndex = u32Max;
				vertexCount = 0;
				indexCount = 0;
				dataSize = 0;
				indexesOffset = 0;
				reserved = 0;
				operationType = u8Max;
				layersIndex = u8Max;
			}
		};

		#pragma pack(pop)
	}
}

#endif