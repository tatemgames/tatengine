/*
 *  teSignal.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 10/7/10.
 *  Copyright 2010 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TESIGNAL_H
#define TE_TESIGNAL_H

// legacy code
// TODO remove this

#include "teConstArray.h"
#include "teDebug.h"

namespace te
{
	// core signal class
	// each slot contain class pointer and method point (actually in teSlotPointer)
	// signal contains slots in plain array in slots (const array)
	// it's possible bacuse we store teSlotPointer's in slots, but cast it to interface class teSlotPointerBase

	#define TE_SIGNAL_BASE(__BaseTemplate, __SignalClassName, __GetArguments, __CallWithArguments) \
	__BaseTemplate \
	class __SignalClassName \
	{ \
	protected: \
		class teSlotPointerBase \
		{ \
		public: \
			virtual ~teSlotPointerBase(){} \
			virtual void Call __GetArguments {} \
		}; \
		\
		template <typename ClassType> \
		class teSlotPointer : public teSlotPointerBase \
		{ \
		public: \
			typedef void (ClassType::*MethodPointerType) __GetArguments; \
			teSlotPointer(){} \
			teSlotPointer(ClassType * setClassPointer, MethodPointerType setMethodPointer) \
				:classPointer(setClassPointer), methodPointer(setMethodPointer){} \
			void Call __GetArguments {if(methodPointer)(classPointer->*methodPointer) __CallWithArguments;} \
		protected: \
			ClassType * classPointer; \
			MethodPointerType methodPointer; \
		}; \
		\
		typedef teSlotPointer<teSlotPointerBase> ReferenceSlotObjectType; \
		teConstArray<ReferenceSlotObjectType> slots; \
	public: \
		template <typename ClassType> \
		void Connect(ClassType * classPointer, typename teSlotPointer<ClassType>::MethodPointerType methodPointer, u1 atBegin = false) \
		{ \
			TE_BREAK(sizeof(teSlotPointer<ClassType>) != sizeof(ReferenceSlotObjectType)) \
			\
			teSlotPointer<ClassType> slotPointer(classPointer, methodPointer); \
			for(u32 i = 0; i < slots.GetSize(); ++i) \
				if(!memcmp(&slots[i], &slotPointer, sizeof(teSlotPointer<ClassType>))) \
					return; \
			\
			slots.ReserveMore(1); \
			slots.Request(); \
			if(atBegin) \
			{ \
				for(u32 i = slots.GetSize() - 1; i > 0; --i) \
					memcpy(&slots[i], &slots[i - 1], sizeof(teSlotPointer<ClassType>)); \
				memcpy(&slots.GetFirst(), &slotPointer, sizeof(teSlotPointer<ClassType>)); \
			} \
			else \
				memcpy(&slots.GetLast(), &slotPointer, sizeof(teSlotPointer<ClassType>)); \
		} \
		\
		template <typename ClassType> \
		void Remove(ClassType * classPointer, typename teSlotPointer<ClassType>::MethodPointerType methodPointer) \
		{ \
			TE_BREAK(sizeof(teSlotPointer<ClassType>) != sizeof(ReferenceSlotObjectType)) \
		\
			teSlotPointer<ClassType> slotPointer(classPointer, methodPointer); \
			for(u32 i = 0; i < slots.GetSize(); ++i) \
				if(!memcmp(&slots[i], &slotPointer, sizeof(teSlotPointer<ClassType>))) \
				{ \
					return;\
				} \
		} \
		\
		void operator () __GetArguments \
		{ \
			for(u8 i = 0; i < slots.GetSize(); ++i) \
				reinterpret_cast<teSlotPointerBase*>(&slots[i])->Call __CallWithArguments; \
		} \
	};
	
	// TODO

//	slots.Erase(i); \
//
	
	//! Signal without arguments
	#define TE_SIGNAL_TEMPLATE_ARG_0
	TE_SIGNAL_BASE(TE_SIGNAL_TEMPLATE_ARG_0, teSignal0, (), ())
	typedef teSignal0 teSignal;

	//! Signals with 1 argument
	#define TE_SIGNAL_TEMPLATE_ARG_1 template<typename ar1>
	TE_SIGNAL_BASE(TE_SIGNAL_TEMPLATE_ARG_1, teSignal1, (ar1 a1), (a1))

	//! Signals with 2 arguments
	#define TE_SIGNAL_TEMPLATE_ARG_2 template<typename ar1, typename ar2>
	TE_SIGNAL_BASE(TE_SIGNAL_TEMPLATE_ARG_2, teSignal2, (ar1 a1, ar2 a2), (a1, a2))

	//! Signals with 3 arguments
	#define TE_SIGNAL_TEMPLATE_ARG_3 template<typename ar1, typename ar2, typename ar3>
	TE_SIGNAL_BASE(TE_SIGNAL_TEMPLATE_ARG_3, teSignal3, (ar1 a1, ar2 a2, ar3 a3), (a1, a2, a3))

	//! Signals with 4 arguments
	#define TE_SIGNAL_TEMPLATE_ARG_4 template<typename ar1, typename ar2, typename ar3, typename ar4>
	TE_SIGNAL_BASE(TE_SIGNAL_TEMPLATE_ARG_4, teSignal4, (ar1 a1, ar2 a2, ar3 a3, ar4 a4), (a1, a2, a3, a4))

	//! Signals with 5 arguments
	#define TE_SIGNAL_TEMPLATE_ARG_5 template<typename ar1, typename ar2, typename ar3, typename ar4, typename ar5>
	TE_SIGNAL_BASE(TE_SIGNAL_TEMPLATE_ARG_5, teSignal5, (ar1 a1, ar2 a2, ar3 a3, ar4 a4, ar5 a5), (a1, a2, a3, a4, a5))

	//! Signals with 6 arguments
	#define TE_SIGNAL_TEMPLATE_ARG_6 template<typename ar1, typename ar2, typename ar3, typename ar4, typename ar5, typename ar6>
	TE_SIGNAL_BASE(TE_SIGNAL_TEMPLATE_ARG_6, teSignal6, (ar1 a1, ar2 a2, ar3 a3, ar4 a4, ar5 a5, ar6 a6), (a1, a2, a3, a4, a5, a6))
}

#endif
