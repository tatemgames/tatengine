/*
 *  teRenderPipeline.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 12/1/11.
 *  Copyright 2011 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TERENDERPIPELINE_H
#define TE_TERENDERPIPELINE_H

#include "teTypes.h"
#include "teConstArray.h"

namespace te
{
	namespace scene
	{
		enum ERenderCommandType
		{
			RCT_INVALID = 0,
			RCT_BEGIN,
			RCT_END,
			RCT_BEGIN_TRANSLUCENT,
			RCT_END_TRANSLUCENT,

			RCT_SET_CAMERA, // use in json\ingame render program
			RCT_SET_FRAMEBUFFER,
			RCT_SET_PASS,
			RCT_CLEAR_SCREEN,

			RCT_DRAW_LAYERS, // use only in json render program

			RCT_DRAW_SPRITES, // use in ingame render program
			RCT_DRAW_SURFACES,
			RCT_DRAW_TEXT,
			RCT_DRAW_ACTOR
		};

		struct teRenderCommand
		{
			ERenderCommandType type;
			u32 from, count;

			TE_INLINE void Set(ERenderCommandType setType = RCT_INVALID, u32 setFrom = 0, u32 setCount = 0)
			{
				type = setType;
				from = setFrom;
				count = setCount;
			}

			TE_INLINE u1 IsDrawCommand() const
			{
				return (type >= RCT_DRAW_LAYERS) && (type <= RCT_DRAW_ACTOR);
			}
		};

		typedef teConstArray<teRenderCommand> teRenderProgram;
		class teAssetPack;
		class teContentPack;

		struct teRenderProgramSortKey
		{
			u32 index;
			u32 materialIndex;
			s16 layer;
			u8 type;

			void Set(const teAssetPack & pack, const teContentPack & content, u32 setIndex, s16 setLayer, u32 setMaterialIndex, u8 setType);
		};

		// TODO refactor sort buffer usage

		void FormRenderProgram(teRenderProgram & program, const teAssetPack & pack, const teContentPack & content, teConstArray<teRenderProgramSortKey> * sortBuffer = NULL);
		void UpdateRenderProgram(teRenderProgram & program, const teAssetPack & pack, const teContentPack & content);
	};
}

#endif