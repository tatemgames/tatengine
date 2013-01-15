/*
 *  teShaderLibrary.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 01/31/12.
 *  Copyright 2012 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TESHADERLIBRARY_H
#define TE_TESHADERLIBRARY_H

#include "teShader.h"

namespace te
{
	namespace video
	{
		enum EShaderType
		{
			ST_DEFAULT = 0,
			ST_SPRITE,
			ST_FAILSAFE,
			ST_LIGHT_MAP,
			ST_SSAO,
			ST_SKIN,
			ST_FONT_DF,
			ST_USER_0,
			ST_USER_1,
			ST_USER_2,
			ST_USER_3,
			ST_USER_4,
			ST_USER_5,
			ST_USER_6,
			ST_USER_7,
			ST_USER_8,
			ST_USER_9,
			ST_USER_10,
			ST_USER_11,
			ST_USER_12,
			ST_USER_13,
			ST_USER_14,
			ST_USER_15,
			ST_MAX
		};

		class teShaderLibrary
		{
		public:
			void BuildShaders();

			TE_INLINE teShader & GetShader(EShaderType type) {return shader[type];}
			TE_INLINE const teShader & GetShader(EShaderType type) const {return shader[type];}

		protected:
			teShader shader[ST_MAX];
		};
	}
}

#endif