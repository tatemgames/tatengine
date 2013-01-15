/*
 *  CApplication.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 12/2/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_CAPPLICATION_H
#define TE_CAPPLICATION_H

#include "teApplicationManager.h"
#include "teFastScene.h"

namespace te
{
	namespace app
	{
		class CApplication : public te::app::IApplication
		{
		public:
			CApplication();
			~CApplication();
			
			void OnUpdate();
			void OnRender();

			TE_INLINE scene::teFastScene * GetFastScene() {return fastScene;}
		protected:
			scene::teFastScene * fastScene;
		};

		CApplication * GetApplication();
	}
}

#endif
