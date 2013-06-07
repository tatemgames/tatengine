/*
 *  teLinkTools.cpp
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 12/1/11.
 *  Copyright 2011 Tatem Games. All rights reserved.
 *
 */

#include "teLinkTools.h"
#include "teAssetPack.h"
#include "teContentPack.h"

namespace te
{
	namespace scene
	{
		void * LinkResolve(teLink & link, teAssetPack & pack, teContentPack & content)
		{
			switch(link.data)
			{
			case LD_RAW: return (void*)(uintptr_t)link.index;

			case LD_MATERIAL: return content.materials.At(link.index);
			case LD_MATERIAL_COLOR_A: return &content.materials[link.index].color.a;
			case LD_MATERIAL_COLOR_R: return &content.materials[link.index].color.r;
			case LD_MATERIAL_COLOR_G: return &content.materials[link.index].color.g;
			case LD_MATERIAL_COLOR_B: return &content.materials[link.index].color.b;

			case LD_ATLAS_SPRITE: return content.atlasSprites.At(link.index);
			case LD_ATLAS_SPRITE_SIZE_X: return &content.atlasSprites[link.index].size.x;
			case LD_ATLAS_SPRITE_SIZE_Y: return &content.atlasSprites[link.index].size.y;
			case LD_ATLAS_SPRITE_ORIGIN_X: return &content.atlasSprites[link.index].origin.x;
			case LD_ATLAS_SPRITE_ORIGIN_Y: return &content.atlasSprites[link.index].origin.y;

			case LD_SKELETON: return content.skeletonData.At(link.index);

			case LD_GEOMETRY: return content.surfaceData.At(link.index);

			case LD_VERTEX_LAYERS: return content.surfaceLayers.At(link.index);

			case LD_FONT: return content.fontData.At(link.index);

			case LD_SOUND: return content.soundsData.At(link.index);

			case LD_STRING: return content.stringsData.At(link.index);

			case LD_CONFIG: return content.configsData.At(link.index);

			case LD_TRANSFORM: return pack.transforms.At(link.index);
			case LD_TRANSFORM_POSITION_X: return &pack.transforms[link.index].position.x;
			case LD_TRANSFORM_POSITION_Y: return &pack.transforms[link.index].position.y;
			case LD_TRANSFORM_POSITION_Z: return &pack.transforms[link.index].position.z;
			case LD_TRANSFORM_ROTATION_X: return &pack.transforms[link.index].rotation.x;
			case LD_TRANSFORM_ROTATION_Y: return &pack.transforms[link.index].rotation.y;
			case LD_TRANSFORM_ROTATION_Z: return &pack.transforms[link.index].rotation.z;
			case LD_TRANSFORM_ROTATION_W: return &pack.transforms[link.index].rotation.w;
			case LD_TRANSFORM_SCALE_X: return &pack.transforms[link.index].scale.x;
			case LD_TRANSFORM_SCALE_Y: return &pack.transforms[link.index].scale.y;
			case LD_TRANSFORM_SCALE_Z: return &pack.transforms[link.index].scale.z;
			case LD_TRANSFORM_VISIBLE: return &pack.transforms[link.index].visible;

			case LD_CAMERA: return pack.cameras.At(link.index);
			case LD_CAMERA_PLANE_NEAR: return &pack.cameras[link.index].nearPlane;
			case LD_CAMERA_PLANE_FAR: return &pack.cameras[link.index].farPlane;
			case LD_CAMERA_FOV: return &pack.cameras[link.index].fov;

			case LD_SPRITE: return pack.sprites.At(link.index);
			case LD_SPRITE_LAYER: return &pack.sprites[link.index].renderAsset.layer;
			case LD_SPRITE_COLOR_A: return &pack.sprites[link.index].color.a;
			case LD_SPRITE_COLOR_R: return &pack.sprites[link.index].color.r;
			case LD_SPRITE_COLOR_G: return &pack.sprites[link.index].color.g;
			case LD_SPRITE_COLOR_B: return &pack.sprites[link.index].color.b;

			case LD_SURFACE: return pack.surfaces.At(link.index);
			case LD_SURFACE_LAYER: return &pack.surfaces[link.index].renderAsset.layer;

			case LD_TEXT: return pack.texts.At(link.index);
			case LD_TEXT_LAYER: return &pack.texts[link.index].renderAsset.layer;
			case LD_TEXT_HALIGN: return &pack.texts[link.index].options.align;
			case LD_TEXT_ORIGIN: return &pack.texts[link.index].options.origin;
			case LD_TEXT_LEADING: return &pack.texts[link.index].options.leading;
			case LD_TEXT_EXTRA_TRACKING: return &pack.texts[link.index].options.extraTracking;
			case LD_TEXT_IGNORE_KERNING: return &pack.texts[link.index].options.ignoreKerning;
			case LD_TEXT_DRAW_SHADOW: return &pack.texts[link.index].options.drawShadow;

			case LD_DRAW_ACTOR: return pack.drawActors.At(link.index);
			case LD_DRAW_ACTOR_LAYER: return &pack.drawActors[link.index].renderAsset.layer;

			case LD_VARIABLE: return pack.variables.At(link.index);
			case LD_VARIABLE_VALUE: return &pack.variables[link.index].vf32;

			case LD_ARRAY: return pack.arrays.At(link.index);

			case LD_ACTOR: return pack.actorsMachine.GetActor(link.index);

			case LD_MAX: return NULL;
			default:
				TE_ASSERT(false);
				return NULL;
			}
		}

		ELinkDataType GetLinkType(const teLink & link, const teAssetPack & pack)
		{
			switch(link.data)
			{
			case LD_MATERIAL_COLOR_A:
			case LD_MATERIAL_COLOR_R:
			case LD_MATERIAL_COLOR_G:
			case LD_MATERIAL_COLOR_B:
				return LDT_U8;

			case LD_ATLAS_SPRITE_SIZE_X:
			case LD_ATLAS_SPRITE_SIZE_Y:
				return LDT_U16;
			case LD_ATLAS_SPRITE_ORIGIN_X:
			case LD_ATLAS_SPRITE_ORIGIN_Y:
				return LDT_F32;

			case LD_TRANSFORM_POSITION_X:
			case LD_TRANSFORM_POSITION_Y:
			case LD_TRANSFORM_POSITION_Z:
			case LD_TRANSFORM_ROTATION_X:
			case LD_TRANSFORM_ROTATION_Y:
			case LD_TRANSFORM_ROTATION_Z:
			case LD_TRANSFORM_ROTATION_W:
			case LD_TRANSFORM_SCALE_X:
			case LD_TRANSFORM_SCALE_Y:
			case LD_TRANSFORM_SCALE_Z:
				return LDT_F32;
			case LD_TRANSFORM_VISIBLE:
				return LDT_U1;

			case LD_CAMERA_PLANE_NEAR:
			case LD_CAMERA_PLANE_FAR:
			case LD_CAMERA_FOV:
				return LDT_F32;

			case LD_SPRITE_LAYER:
				return LDT_S16;
			case LD_SPRITE_COLOR_A:
			case LD_SPRITE_COLOR_R:
			case LD_SPRITE_COLOR_G:
			case LD_SPRITE_COLOR_B:
				return LDT_U8;

			case LD_SURFACE_LAYER:
				return LDT_S16;

			case LD_TEXT_LAYER:
				return LDT_S16;
			case LD_TEXT_HALIGN:
			case LD_TEXT_ORIGIN:
				return LDT_U8;
			case LD_TEXT_LEADING:
			case LD_TEXT_EXTRA_TRACKING:
				return LDT_F32;
			case LD_TEXT_IGNORE_KERNING:
				return LDT_U1;

			case LD_DRAW_ACTOR_LAYER:
				return LDT_S16;

			case LD_VARIABLE_VALUE:
				return (pack.variables[link.index].type == AVT_F32) ? LDT_F32 : LDT_S32;

			default: return LDT_UNKNOWN;
			}
		}

	}
}
