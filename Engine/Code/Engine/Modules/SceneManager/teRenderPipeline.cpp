/*
 *  teRenderPipeline.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 12/1/11.
 *  Copyright 2011 Tatem Games. All rights reserved.
 *
 */

#include "teRenderPipeline.h"
#include "teContentPack.h"
#include "teAssetPack.h"

namespace te
{
	namespace scene
	{
		struct teKey
		{
			u32 index;
			u32 materialIndex;
			s16 layer;
			u8 type;

			void Set(const teAssetPack & pack, const teContentPack & content, u32 setIndex, s16 setLayer, u32 setMaterialIndex, u8 setType)
			{
				index = setIndex;
				layer = setLayer;
				materialIndex = setMaterialIndex;
				type = setType;

				if((materialIndex != u32Max) && (content.materials[materialIndex].metaMaterial != u32Max))
					materialIndex = content.materials[materialIndex].metaMaterial;
			}
		};

		void teKeyCompare(s8 & result, const teKey & key1, const teKey & key2)
		{
			if(key1.layer > key2.layer)
				result = 1;
			else if(key1.layer < key2.layer)
				result = -1;
			else if(key1.materialIndex > key2.materialIndex)
				result = 1;
			else if(key1.materialIndex < key2.materialIndex)
				result = -1;
			else if(key1.type > key2.type)
				result = 1;
			else if(key1.type < key2.type)
				result = -1;
			else if(key1.index > key2.index)
				result = 1;
			else if(key1.index < key2.index)
				result = -1;
			else
				result = 0;
		}

		void FormRenderProgram(teRenderProgram & program, const teAssetPack & pack, const teContentPack & content)
		{
			const u32 keysCount = pack.sprites.GetAlive() + pack.surfaces.GetAlive() + pack.texts.GetAlive() + pack.drawActors.GetAlive();

			if(!keysCount)
				return;

			teConstArray<teKey> sort;
			sort.Reserve(keysCount);

			for(u32 i = 0; i < pack.sprites.GetAlive(); ++i)
				sort.Request()->Set(pack, content, i, pack.sprites[i].renderAsset.layer, pack.sprites[i].renderAsset.materialIndex, RCT_DRAW_SPRITES);

			for(u32 i = 0; i < pack.surfaces.GetAlive(); ++i)
				sort.Request()->Set(pack, content, i, pack.surfaces[i].renderAsset.layer, pack.surfaces[i].renderAsset.materialIndex, RCT_DRAW_SURFACES);

			for(u32 i = 0; i < pack.texts.GetAlive(); ++i)
				sort.Request()->Set(pack, content, i, pack.texts[i].renderAsset.layer, pack.texts[i].renderAsset.materialIndex, RCT_DRAW_TEXT);

			for(u32 i = 0; i < pack.drawActors.GetAlive(); ++i)
				sort.Request()->Set(pack, content, i, pack.drawActors[i].renderAsset.layer, pack.drawActors[i].renderAsset.materialIndex, RCT_DRAW_ACTOR);

			sort.ShellSort(&teKeyCompare);

			//for(u32 i = 0; i < sort.GetAlive(); ++i)
			//	printf("%i (%i, %i)\n", sort[i].index, sort[i].layer, sort[i].materialIndex);

			program.Clear();
			program.Reserve(sort.GetAlive() + 2 + pack.program.GetAlive()); // TODO optimize this value
			program.Request()->Set(RCT_BEGIN);

			for(u32 i = 0; i < pack.program.GetAlive(); ++i)
			{
				switch(pack.program[i].type)
				{
				case RCT_SET_CAMERA:
					{
						program.Request()->Set(RCT_SET_CAMERA, pack.program[i].from);
						break;
					}
				case RCT_SET_FRAMEBUFFER:
					{
						program.Request()->Set(RCT_SET_FRAMEBUFFER, pack.program[i].from);
						break;
					}
				case RCT_SET_PASS:
					{
						program.Request()->Set(RCT_SET_PASS, pack.program[i].from);
						break;
					}
				case RCT_CLEAR_SCREEN:
					{
						program.Request()->Set(RCT_CLEAR_SCREEN);
						break;
					}
				case RCT_DRAW_LAYERS:
					{
						for(u32 j = 0; j < sort.GetAlive(); ++j)
						{
							s16 layersFrom = (s16)(((s32)pack.program[i].from) + s16Min);
							s16 layersTo = layersFrom + (s16)pack.program[i].count;

							if((sort[j].layer < layersFrom) || (sort[j].layer >= layersTo))
								continue;

							ERenderCommandType command = (ERenderCommandType)sort[j].type;
							teRenderCommand & last = program.GetLast();

							if((last.type == command) && (last.from + last.count == sort[j].index))
								++last.count;
							else
								program.Request()->Set(command, sort[j].index, 1);
						}

						break;
					}
				}
			}

			program.Request()->Set(RCT_END);

			//for(u32 i = 0; i < program.GetAlive(); ++i)
			//	printf("rt %i (%i, %i)\n", program[i].type, program[i].from, program[i].count);
			//printf("total render pipeline commands %i\n", program.GetAlive());
		}

		void UpdateRenderProgram(teRenderProgram & program, const teAssetPack & pack, const teContentPack & content)
		{
		}
	};
}
