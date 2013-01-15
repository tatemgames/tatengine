/*
 *  teRenderContext.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 11/27/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TERENDERCONTEXT_H
#define TE_TERENDERCONTEXT_H

#include "teTypes.h"
#include "teReferenceCounter.h"
#include "tePlatform.h"
#include "teRenderSystem.h"
#include "teTexture.h"

namespace te
{
	namespace video
	{
		//! Render Context
		class teRenderContext : public te::teReferenceCounter
		{
		protected:
			EGAPIVersion CurrentVersion;
			
			struct teRenderContextDevice;
			
			teRenderContextDevice * ContextDevice;

			teVector2di CurrentSize;
			
		public:
			teRenderContext();
			teRenderContext(EGAPIVersion Version);
			~teRenderContext();
			
			EGAPIVersion GetVersion()
			{
				return CurrentVersion;
			}
			
			void Bind();
			void UnBind();
			
			u1 IsComplete();
			
			void PresentCurrentTexture();
			
			void * GetGLContext();

			void SetSize(const teVector2di & Size)
			{
				if(CurrentSize != Size)
					CurrentSize = Size;
			}

			TE_INLINE const teVector2di & GetSize() const {return CurrentSize;}
		};
	}
}

#endif
