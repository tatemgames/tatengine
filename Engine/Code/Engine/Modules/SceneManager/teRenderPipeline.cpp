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
			else if(key1.materialIndex != key2.materialIndex)
			{
				if(key2.layer % 2)
				{
					if(key1.materialIndex > key2.materialIndex)
						result = 1;
					else if(key1.materialIndex < key2.materialIndex)
						result = -1;
				}
				else
				{
					if(key1.materialIndex < key2.materialIndex)
						result = 1;
					else if(key1.materialIndex > key2.materialIndex)
						result = -1;
				}
			}
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

		teAssetPack * tempPack;
		teContentPack * tempContent;

		struct teSpriteTreeNode;
		void teSpriteTreeSort(s8 & result, const teSpriteTreeNode & keyt1, const teSpriteTreeNode & keyt2);

		struct teSpriteTreeNode
		{
			teAABB3df aabb;
			u32 sprite;
			teConstArray<teSpriteTreeNode> childrens;

			void Init()
			{
				aabb.Flush();
				sprite = u32Max;
				new (&childrens) teConstArray<teSpriteTreeNode>();
			}

			void Clear()
			{
				for(u32 i = 0; i < childrens.GetAlive(); ++i)
					childrens[i].Clear();
				childrens.Clear();
			}

			/*
			void CalculateAABB(const teAssetPack & pack)
			{
				for(u32 i = 0; i < childrens.GetAlive(); ++i)
					childrens[i].CalculateAABB(pack);

				if(sprite != u32Max)
					aabb = pack.sprites[sprite].renderAsset.aabb;
				else
					aabb.Flush();

				for(u32 i = 0; i < childrens.GetAlive(); ++i)
					aabb.Unite(childrens[i].aabb);
			}
			*/

			u1 IsIntersect(u32 sprite, const teAssetPack & pack)
			{
				return aabb.IsIntersectAABB(pack.sprites[sprite].renderAsset.aabb);
			}

			void PushSprite(u32 spriteInsert, const teAssetPack & pack)
			{
				for(u32 i = 0; i < childrens.GetAlive(); ++i)
					if(childrens[i].IsIntersect(spriteInsert, pack))
					{
						childrens[i].PushSprite(spriteInsert, pack);
						return;
					}

				if(childrens.GetAlive() >= childrens.GetSize())
					childrens.ReserveMore(32);

				teSpriteTreeNode * node = childrens.Request();
				node->Init();
				node->sprite = spriteInsert;

				if(spriteInsert != u32Max)
					node->aabb = pack.sprites[spriteInsert].renderAsset.aabb;
				else
					node->aabb.Flush();

				return;
			}

			void Dump(teRenderProgram & program)
			{
				#if 1

				childrens.ShellSort(&teSpriteTreeSort);

				for(u32 i = 0; i < childrens.GetAlive(); ++i)
					if(childrens[i].sprite != u32Max)
					{
						program.Request()->Set(RCT_DRAW_SPRITES, childrens[i].sprite, 1);
						u32 mi = (*tempPack).sprites[childrens[i].sprite].renderAsset.materialIndex;
						u32 mm = (*tempContent).materials[mi].metaMaterial;
						printf("%3i %3i %3i\n", childrens[i].sprite, mi, mm);

					}

				for(u32 i = 0; i < childrens.GetAlive(); ++i)
					childrens[i].Dump(program);

				#else

				childrens.ShellSort(&teSpriteTreeSort);

				if(sprite != u32Max)
				{
					program.Request()->Set(RCT_DRAW_SPRITES, sprite, 1);

					u32 mi = (*tempPack).sprites[sprite].renderAsset.materialIndex;
					u32 mm = (*tempContent).materials[mi].metaMaterial;
					printf("%3i %3i %3i\n", sprite, mi, mm);
				}

				for(u32 i = 0; i < childrens.GetAlive(); ++i)
					childrens[i].Dump(program);

				#endif
			}
		};

		void teSpriteTreeSort(s8 & result, const teSpriteTreeNode & key1, const teSpriteTreeNode & key2)
		{
			u32 mi1 = (*tempPack).sprites[key1.sprite].renderAsset.materialIndex;
			u32 mi2 = (*tempPack).sprites[key1.sprite].renderAsset.materialIndex;

			const video::teMaterial & m1 = (*tempContent).materials[mi1];
			const video::teMaterial & m2 = (*tempContent).materials[mi2];

			if(m1.metaMaterial > m2.metaMaterial)
				result = 1;
			else if(m1.metaMaterial < m2.metaMaterial)
				result = -1;
			else
				result = 0;
		}

		void teSpriteLayerSort(s8 & result, const u32 & key1, const u32 & key2)
		{
			s32 l1 = (key1 != u32Max) ? (*tempPack).sprites[key1].renderAsset.layer : s32Max;
			s32 l2 = (key2 != u32Max) ? (*tempPack).sprites[key2].renderAsset.layer : s32Max;

			if(l1 > l2)
				result = 1;
			else if(l1 < l2)
				result = -1;
			else
				result = 0;
		}

		void RecalculateMetaMaterials(const teConstArray<u32> & sprites)
		{
			for(u32 i = 0; i < sprites.GetAlive(); ++i)
			{
				if(sprites[i] == u32Max)
					continue;

				u32 mi = (*tempPack).sprites[sprites[i]].renderAsset.materialIndex;
				video::teMaterial & b = (*tempContent).materials[mi];
				b.metaMaterial = mi;

				for(u32 j = 0; j < tempContent->materials.GetAlive(); ++j)
				{
					if(j == mi)
						continue;

					video::teMaterial & c = tempContent->materials[j];

					u1 fail = false;
					for(u8 k = 0; k < video::teMaterialMaxTextures; ++k)
					{
						if(b.atlasSpriteIndex[k] == c.atlasSpriteIndex[k])
							continue;
						else if((b.atlasSpriteIndex[k] == u32Max) || (c.atlasSpriteIndex[k] == u32Max))
							fail = true;
						else if(tempContent->atlasSprites[b.atlasSpriteIndex[k]].textureIndex != tempContent->atlasSprites[c.atlasSpriteIndex[k]].textureIndex)
							fail = true;
					}
					if(fail)
						continue;

					fail = false;
					for(u8 k = 0; k < video::teMaterialMaxUniformUserData; ++k)
						if(!b.userData[k].IsEqual(c.userData[k]))
							fail = true;
					if(fail)
						continue;

					u1 ok = (b.shaderIndex == c.shaderIndex);

					if(ok)
					{
						b.metaMaterial = j;
						break;
					}
				}
			}
		}

		void UpdateRenderProgram(teRenderProgram & program, const teAssetPack & pack, const teContentPack & content)
		{
			tempPack = (teAssetPack*)&pack;
			tempContent = (teContentPack*)&content;

			u32 cacheSize = program.GetAlive() + pack.sprites.GetAlive();

			if(!cacheSize)
				return;

			teConstArray<teRenderCommand> updateCache;
			updateCache.Reserve(cacheSize);

			teConstArray<u32> spriteRender;
			spriteRender.Reserve(pack.sprites.GetAlive());
			spriteRender.Request(pack.sprites.GetAlive());
			for(u32 i = 0; i < spriteRender.GetAlive(); ++i)
				spriteRender[i] = u32Max;
			u32 spriteRenderIndex = 0;

			u1 analyzeSprites = false;

			for(u32 i = 0; i < program.GetAlive(); ++i)
			{
				switch(program[i].type)
				{
				case RCT_DRAW_SPRITES:
					{
						if(analyzeSprites)
						{
							for(u32 j = program[i].from; j < (program[i].count ? (program[i].from + program[i].count) : pack.sprites.GetAlive()); ++j)
								spriteRender[spriteRenderIndex++] = j;
						}
						else
						{
							//*updateCache.Request() = program[i];
						}

						break;
					}
				case RCT_DRAW_TEXT:
				case RCT_DRAW_SURFACES:
				case RCT_DRAW_ACTOR:
						break;
				default:
					{
						if(analyzeSprites)
						{
							#if 1
							teSpriteTreeNode root;
							root.Init();

							RecalculateMetaMaterials(spriteRender);

							spriteRender.ShellSort(&teSpriteLayerSort);

							for(u32 j = 0; j < spriteRenderIndex; ++j)
							{
								if(pack.transforms[pack.sprites[spriteRender[j]].renderAsset.transformIndex].inFrame)
									root.PushSprite(spriteRender[j], pack);
								else
									updateCache.Request()->Set(RCT_DRAW_SPRITES, spriteRender[j], 1);

								spriteRender[j] = u32Max;
							}

							spriteRenderIndex = 0;

							root.Dump(updateCache);

							root.Clear();

							printf("----\n");

							#else
							for(u32 j = 0; j < spriteRenderIndex; ++j)
								updateCache.Request()->Set(RCT_DRAW_SPRITES, spriteRender[j], 1);
							spriteRenderIndex = 0;
							#endif
						}

						*updateCache.Request() = program[i];

						if(program[i].type == RCT_SET_CAMERA)
						{
							if(pack.cameras[program[i].from].cameraType == CT_ORTHOGRAPHIC)
								analyzeSprites = true;
							else
								analyzeSprites = false;
						}

						break;
					}
				}
			}

			program.Clear();
			program.Reserve(updateCache.GetAlive());
			program.Request(updateCache.GetAlive());
			memcpy(program.GetPool(), updateCache.GetPool(), updateCache.GetAlive() * sizeof(teRenderCommand));
		}
	};
}
