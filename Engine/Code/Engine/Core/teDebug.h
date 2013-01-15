/*
 *  teDebug.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 11/24/09.
 *  Copyright 2009 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEDEBUG_H
#define TE_TEDEBUG_H

#include "TatEngineCoreConfig.h"

#include "assert.h"
#define TE_ASSERT_NODEBUG(cond) { assert(cond); }

#ifdef TE_DEBUG
	#define TE_ASSERT(cond) TE_ASSERT_NODEBUG(cond)
#else
	#define TE_ASSERT(cond) { }
#endif

#define TE_BREAK(cond) TE_ASSERT( !(cond) )

#endif
