/*
 *  teActorCombineArguments.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 02/25/12.
 *  Copyright 2012 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEACTORCOMBINEARGUMENTS_H
#define TE_TEACTORCOMBINEARGUMENTS_H

#include "teActor.h"
#include "teFastScene.h"

// TODO rename this file to teActorTools

namespace te
{
	namespace scene
	{
		class teActorCombineArguments
		{
		public:
			TE_INLINE void In(f32 a0, f32 a1, f32 a2, f32 a3)
			{
				if(setA0) a0 = setA0->GetF32();
				if(setA1) a1 = setA1->GetF32();
				if(setA2) a2 = setA2->GetF32();
				if(setA3) a3 = setA3->GetF32();

				Out(a0, a1, a2, a3);
			}

			TE_ACTOR_SIGNAL(0, Out);

		protected:
			teAssetVariable * setA0;
			teAssetVariable * setA1;
			teAssetVariable * setA2;
			teAssetVariable * setA3;
		};

		TE_ACTOR_SLOT_4(teActorCombineArguments, In);

		class teActorCombineSignals
		{
		public:
			TE_INLINE void InX(f32 a0) {x = a0;}
			TE_INLINE void InY(f32 a0) {y = a0;}
			TE_INLINE void InZ(f32 a0) {z = a0;}
			TE_INLINE void InW(f32 a0) {w = a0;}

			TE_INLINE void Trigger()
			{
				Out(x, y, z, w);
			}

			TE_ACTOR_SIGNAL(0, Out);

		protected:
			f32 x, y, z, w;
		};

		TE_ACTOR_SLOT_1(teActorCombineSignals, InX);
		TE_ACTOR_SLOT_1(teActorCombineSignals, InY);
		TE_ACTOR_SLOT_1(teActorCombineSignals, InZ);
		TE_ACTOR_SLOT_1(teActorCombineSignals, InW);
		TE_ACTOR_SLOT_0(teActorCombineSignals, Trigger);

		class teActorCombineVariables
		{
		public:
			TE_INLINE void In(f32 a0, f32 a1, f32 a2, f32 a3) {Out(a0 * x + a1 * y + a2 * z + a3 * w + c);}

			TE_ACTOR_SIGNAL(0, Out);
		protected:
			f32 x, y, z, w, c;
		};

		TE_ACTOR_SLOT_4(teActorCombineVariables, In);

		class teActorSetValue
		{
		public:
			TE_INLINE teActorSetValue(teFastScene * setScene)
			{
				valueType = setScene->GetAssetPack().GetLinkDataType(value);
			}

			TE_INLINE ~teActorSetValue()
			{
			}

			TE_INLINE void In(f32 a0)
			{
				SetLinkVariable(value, (ELinkDataType)valueType, a0);
			}

		protected:
			void * value;
			u8 valueType;
		};

		TE_ACTOR_PROXY_NU(teActorSetValue);
		TE_ACTOR_SLOT_1(teActorSetValue, In);

		class teActorMultiplexer
		{
		public:
			TE_INLINE void In0(f32 a0, f32 a1, f32 a2) {Out(a0, a1, a2, 0.0f);}
			TE_INLINE void In1(f32 a0, f32 a1, f32 a2) {Out(a0, a1, a2, 1.0f);}
			TE_INLINE void In2(f32 a0, f32 a1, f32 a2) {Out(a0, a1, a2, 2.0f);}
			TE_INLINE void In3(f32 a0, f32 a1, f32 a2) {Out(a0, a1, a2, 3.0f);}
			TE_INLINE void In4(f32 a0, f32 a1, f32 a2) {Out(a0, a1, a2, 4.0f);}
			TE_INLINE void In5(f32 a0, f32 a1, f32 a2) {Out(a0, a1, a2, 5.0f);}
			TE_INLINE void In6(f32 a0, f32 a1, f32 a2) {Out(a0, a1, a2, 6.0f);}
			TE_INLINE void In7(f32 a0, f32 a1, f32 a2) {Out(a0, a1, a2, 7.0f);}

			TE_ACTOR_SIGNAL(0, Out);
			
		protected:
			u32 _reserved;
		};

		TE_ACTOR_SLOT_3(teActorMultiplexer, In0);
		TE_ACTOR_SLOT_3(teActorMultiplexer, In1);
		TE_ACTOR_SLOT_3(teActorMultiplexer, In2);
		TE_ACTOR_SLOT_3(teActorMultiplexer, In3);
		TE_ACTOR_SLOT_3(teActorMultiplexer, In4);
		TE_ACTOR_SLOT_3(teActorMultiplexer, In5);
		TE_ACTOR_SLOT_3(teActorMultiplexer, In6);
		TE_ACTOR_SLOT_3(teActorMultiplexer, In7);

		class teActorDemultiplexer
		{
		public:
			TE_INLINE void In(f32 a0, f32 a1, f32 a2, f32 a3)
			{
				u8 index = (u8)a3;

				switch(index)
				{
				case 0: Out0(a0, a1, a2); break;
				case 1: Out1(a0, a1, a2); break;
				case 2: Out2(a0, a1, a2); break;
				case 3: Out3(a0, a1, a2); break;
				case 4: Out4(a0, a1, a2); break;
				case 5: Out5(a0, a1, a2); break;
				case 6: Out6(a0, a1, a2); break;
				case 7: Out7(a0, a1, a2); break;
				default: break;
				}
			}

			TE_ACTOR_SIGNAL(0, Out0);
			TE_ACTOR_SIGNAL(1, Out1);
			TE_ACTOR_SIGNAL(2, Out2);
			TE_ACTOR_SIGNAL(3, Out3);
			TE_ACTOR_SIGNAL(4, Out4);
			TE_ACTOR_SIGNAL(5, Out5);
			TE_ACTOR_SIGNAL(6, Out6);
			TE_ACTOR_SIGNAL(7, Out7);
			
		protected:
			u32 _reserved;
		};

		TE_ACTOR_SLOT_4(teActorDemultiplexer, In);

		TE_FUNC void RegisterCombineArguments(teActorsTI * ti)
		{
			ti->AddClass(sizeof(teActorCombineArguments), "combineArguments", TE_ACTOR_PROXY_NAMES_NONE);
			ti->AddLink("a0");
			ti->AddLink("a1");
			ti->AddLink("a2");
			ti->AddLink("a3");
			ti->AddSignal("Out");
			ti->AddSlot("In", TE_ACTOR_SLOT_PROXY(teActorCombineArguments, In));

			ti->AddClass(sizeof(teActorCombineSignals), "combineSignals", TE_ACTOR_PROXY_NAMES_NONE);
			ti->AddSignal("Out");
			ti->AddSlot("InX", TE_ACTOR_SLOT_PROXY(teActorCombineSignals, InX));
			ti->AddSlot("InY", TE_ACTOR_SLOT_PROXY(teActorCombineSignals, InY));
			ti->AddSlot("InZ", TE_ACTOR_SLOT_PROXY(teActorCombineSignals, InZ));
			ti->AddSlot("InW", TE_ACTOR_SLOT_PROXY(teActorCombineSignals, InW));
			ti->AddSlot("Trigger", TE_ACTOR_SLOT_PROXY(teActorCombineSignals, Trigger));

			ti->AddClass(sizeof(teActorCombineVariables), "combineVariables", TE_ACTOR_PROXY_NAMES_NONE);
			ti->AddData("x");
			ti->AddData("y");
			ti->AddData("z");
			ti->AddData("w");
			ti->AddData("c");
			ti->AddSignal("Out");
			ti->AddSlot("In", TE_ACTOR_SLOT_PROXY(teActorCombineVariables, In));

			ti->AddClass(sizeof(teActorSetValue), "setValue", TE_ACTOR_PROXY_NAMES_NU(teActorSetValue));
			ti->AddLink("value");
			ti->AddSlot("In", TE_ACTOR_SLOT_PROXY(teActorSetValue, In));

			ti->AddClass(sizeof(teActorMultiplexer), "multiplexer", TE_ACTOR_PROXY_NAMES_NONE);
			ti->AddSignal("Out");
			ti->AddSlot("In0", TE_ACTOR_SLOT_PROXY(teActorMultiplexer, In0));
			ti->AddSlot("In1", TE_ACTOR_SLOT_PROXY(teActorMultiplexer, In1));
			ti->AddSlot("In2", TE_ACTOR_SLOT_PROXY(teActorMultiplexer, In2));
			ti->AddSlot("In3", TE_ACTOR_SLOT_PROXY(teActorMultiplexer, In3));
			ti->AddSlot("In4", TE_ACTOR_SLOT_PROXY(teActorMultiplexer, In4));
			ti->AddSlot("In5", TE_ACTOR_SLOT_PROXY(teActorMultiplexer, In5));
			ti->AddSlot("In6", TE_ACTOR_SLOT_PROXY(teActorMultiplexer, In6));
			ti->AddSlot("In7", TE_ACTOR_SLOT_PROXY(teActorMultiplexer, In7));

			ti->AddClass(sizeof(teActorDemultiplexer), "demultiplexer", TE_ACTOR_PROXY_NAMES_NONE);
			ti->AddSignal("Out0");
			ti->AddSignal("Out1");
			ti->AddSignal("Out2");
			ti->AddSignal("Out3");
			ti->AddSignal("Out4");
			ti->AddSignal("Out5");
			ti->AddSignal("Out6");
			ti->AddSignal("Out7");
			ti->AddSlot("In", TE_ACTOR_SLOT_PROXY(teActorDemultiplexer, In));
		}
	}
}

#endif