/*
 *  teLinkTools.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 12/1/11.
 *  Copyright 2011 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TELINKTOOLS_H
#define TE_TELINKTOOLS_H

#include "teTypes.h"

namespace te
{
	namespace scene
	{
		enum ELinkData
		{
			LD_RAW = 0,

			LD_MATERIAL,
			LD_MATERIAL_COLOR_A,
			LD_MATERIAL_COLOR_R,
			LD_MATERIAL_COLOR_G,
			LD_MATERIAL_COLOR_B,

			LD_ATLAS_SPRITE,
			LD_ATLAS_SPRITE_SIZE_X,
			LD_ATLAS_SPRITE_SIZE_Y,
			LD_ATLAS_SPRITE_ORIGIN_X,
			LD_ATLAS_SPRITE_ORIGIN_Y,

			LD_SKELETON,

			LD_GEOMETRY,

			LD_VERTEX_LAYERS,

			LD_FONT,

			LD_SOUND,

			LD_STRING,

			LD_CONFIG, // TODO do we need this ?

			LD_TRANSFORM,
			LD_TRANSFORM_POSITION_X,
			LD_TRANSFORM_POSITION_Y,
			LD_TRANSFORM_POSITION_Z,
			LD_TRANSFORM_ROTATION_X,
			LD_TRANSFORM_ROTATION_Y,
			LD_TRANSFORM_ROTATION_Z,
			LD_TRANSFORM_ROTATION_W,
			LD_TRANSFORM_SCALE_X,
			LD_TRANSFORM_SCALE_Y,
			LD_TRANSFORM_SCALE_Z,
			LD_TRANSFORM_VISIBLE,

			LD_CAMERA,
			LD_CAMERA_PLANE_NEAR,
			LD_CAMERA_PLANE_FAR,
			LD_CAMERA_FOV,

			LD_SPRITE,
			LD_SPRITE_LAYER,
			LD_SPRITE_COLOR_A,
			LD_SPRITE_COLOR_R,
			LD_SPRITE_COLOR_G,
			LD_SPRITE_COLOR_B,

			LD_SURFACE,
			LD_SURFACE_LAYER,

			LD_TEXT,
			LD_TEXT_LAYER,
			LD_TEXT_HALIGN,
			LD_TEXT_ORIGIN,
			LD_TEXT_LEADING,
			LD_TEXT_EXTRA_TRACKING,
			LD_TEXT_IGNORE_KERNING,
			LD_TEXT_DRAW_SHADOW,

			LD_DRAW_ACTOR,
			LD_DRAW_ACTOR_LAYER,

			LD_VARIABLE,
			LD_VARIABLE_VALUE,

			LD_ARRAY,

			LD_ACTOR,

			LD_MAX = 0xFF
		};

		enum ELinkDataType
		{
			LDT_UNKNOWN,

			LDT_U1,

			LDT_U8,
			LDT_S8,
			LDT_C8,

			LDT_U16,
			LDT_S16,

			LDT_U32,
			LDT_S32,

			LDT_U64,
			LDT_S64,

			LDT_F32,
			LDT_F64,

			LDT_MAX
		};

		struct teLink
		{
			u32 index;
			u8 data;

			void Clear()
			{
				index = u32Max;
				data = LD_MAX;
			}
		};

		struct teLinkRTTI
		{
			teptr_t ptr;
			teLink link;
		};

		class teFastScene;
		class teAssetPack;
		class teContentPack;

		// TODO need inline here
		void * LinkResolve(teLink & link, teAssetPack & pack, teContentPack & content);
		ELinkDataType GetLinkType(const teLink & link, const teAssetPack & pack);
		
		// helper for set variable value by type
		template<typename T>
		TE_INLINE u1 SetLinkVariable(void * data, ELinkDataType type, const T & setData)
		{
			switch(type)
			{
			case LDT_U1:  (*(u1*)(data))  = (setData ? true : false); return true;
			case LDT_U8:  (*(u8*)(data))  = (u8)setData; return true;
			case LDT_S8:  (*(s8*)(data))  = (s8)setData; return true;
			case LDT_C8:  (*(c8*)(data))  = (c8)setData; return true;
			case LDT_U16: (*(u16*)(data)) = (u16)setData; return true;
			case LDT_S16: (*(s16*)(data)) = (s16)setData; return true;
			case LDT_U32: (*(u32*)(data)) = (u32)setData; return true;
			case LDT_S32: (*(s32*)(data)) = (s32)setData; return true;
			case LDT_U64: (*(u64*)(data)) = (u64)setData; return true;
			case LDT_S64: (*(s64*)(data)) = (s64)setData; return true;
			case LDT_F32: (*(f32*)(data)) = (f32)setData; return true;
			case LDT_F64: (*(f64*)(data)) = (f64)setData; return true;
			default: return false;
			}
		}

		// helper for get variable value by type
		template<typename T>
		TE_INLINE u1 GetLinkVariable(void * data, ELinkDataType type, T & getData)
		{
			switch(type)
			{
			case LDT_U1:  getData = (T)(*(u1*)(data));  return true;
			case LDT_U8:  getData = (T)(*(u8*)(data));  return true;
			case LDT_S8:  getData = (T)(*(s8*)(data));  return true;
			case LDT_C8:  getData = (T)(*(c8*)(data));  return true;
			case LDT_U16: getData = (T)(*(u16*)(data)); return true;
			case LDT_S16: getData = (T)(*(s16*)(data)); return true;
			case LDT_U32: getData = (T)(*(u32*)(data)); return true;
			case LDT_S32: getData = (T)(*(s32*)(data)); return true;
			case LDT_U64: getData = (T)(*(u64*)(data)); return true;
			case LDT_S64: getData = (T)(*(s64*)(data)); return true;
			case LDT_F32: getData = (T)(*(f32*)(data)); return true;
			case LDT_F64: getData = (T)(*(f64*)(data)); return true;
			default: return false;
			}
		}
	}
}

#endif