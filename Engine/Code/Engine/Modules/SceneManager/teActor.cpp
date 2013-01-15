/*
 *  teActor.cpp
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 12/1/11.
 *  Copyright 2011 Tatem Games. All rights reserved.
 *
 */

#include "teActor.h"
#include "teBufferSecurity.h"

namespace te
{
	namespace scene
	{
		/*
		internal structure :

		rtti - run-time type information
		ctti - compile-time type information

		------- rtti -------
		teClassTable[]
		teptr_t slots[]

		------- ctti -------
		u32 actorsClassesCount;
		teClassTableCT[]
		u32 offsetsToStringPool[]
		teClassTableCTName[]

		------- ctti offsetsToStringPool array -------
		each entry is u32

		0) actor name
		1) link name #0
		...
		N) link name #N - 1
		N + 1) signal name #0
		...
		N + M) signal name #M - 1
		N + M + 1) slot name #0
		...
		N + M + K) slot name #K - 1

		------- actors -------
		teActorsTable
		{
			__align_bytes // not used 0 to 3 bytes for actor memory align
			variables_array // TODO optional : if this class use global variables - optimizing for processor cache
			actor memory // aligned to 4 bytes
			u32[] // signal slot table offsets (offset of begin of this array), optional : if we have signals in class
			teSlotsTable // optional : if we have signals in class
		}[]

		*/

		// ---------------------------------------------------------------------- actors type information

		void teActorsTI::SetCalculateSizeMode(u1 enabled)
		{
			if(enabled)
			{
				enableSizeCalculation = true;
				reserveClassCount = 0;
				reserveDataCount = 0;
				reserveLinksCount = 0;
				reserveSignalsCount = 0;
				reserveSlotsCount = 0;
				reserveTotalStringsBytes = 0;
				crc = 0;
			}
			else
			{
				enableSizeCalculation = false;
				Reserve();
			}
		}

		void teActorsTI::AddClass(u32 size, const c8 * name, __actor::teClassServiceConstruct constructor, __actor::teClassService destructor, __actor::teClassService onUpdate, const c8 * description)
		{
			if(!enableSizeCalculation)
			{
				#ifdef TE_CTTI
				core::GetLogManager()->GetConcate().Add(name).Add("_%d", size);
				teString classString = core::GetLogManager()->GetConcate().BakeToString();
				crc ^= core::GetCRC32((u8*)classString.c_str(), classString.GetSize());
				//printf("crc stage %u '%s'\n", crc, classString.c_str());
				#endif
			}

			if(enableSizeCalculation)
			{
				++reserveClassCount;
				#ifdef TE_CTTI
				reserveTotalStringsBytes += strlen(name) + 1;
				#endif
				return;
			}

			__actor::teClassTableRT * actorRT = GetClassRTTI(reserveClassCount);
			actorRT->constructor = reinterpret_cast<teptr_t>(constructor);
			actorRT->destructor = reinterpret_cast<teptr_t>(destructor);
			actorRT->onUpdate = reinterpret_cast<teptr_t>(onUpdate);
			actorRT->size = size;
			actorRT->signalsCount = 0;
			actorRT->dataCount = 0;
			actorRT->slotsCount = 0;
			actorRT->slotsOffset = u32Max;

			#ifdef TE_CTTI

			__actor::teClassTableCT * actorCT = GetClassCTTI(reserveClassCount);
			actorCT->linksCount = 0;
			actorCT->signalsCount = 0;
			actorCT->slotsCount = 0;
			actorCT->namesOffset = cttiNameOffsetsPosition;

			SetNameAtIndex(actorCT, name, 0);

			#endif

			++reserveClassCount;
		}

		void teActorsTI::AddData(const c8 * name, u1 linkable, const c8 * description)
		{
			if(enableSizeCalculation)
			{
				++reserveDataCount;
				#ifdef TE_CTTI
				reserveTotalStringsBytes += strlen(name) + 1;
				#endif
				return;
			}

			__actor::teClassTableRT * actorRT = GetClassRTTI(reserveClassCount - 1);
			++actorRT->dataCount;

			#ifdef TE_CTTI

			__actor::teClassTableCT * actorCT = GetClassCTTI(reserveClassCount - 1);
			SetNameAtIndex(actorCT, name, 1 + actorCT->dataCount);
			++actorCT->dataCount;

			#endif
		}

		void teActorsTI::AddLink(const c8 * name, u1 optional, const c8 * acceptableTypes, const c8 * description)
		{
			if(enableSizeCalculation)
			{
				++reserveLinksCount;
				#ifdef TE_CTTI
				reserveTotalStringsBytes += strlen(name) + 1;
				#endif
				return;
			}

			#ifdef TE_CTTI

			__actor::teClassTableCT * actorCT = GetClassCTTI(reserveClassCount - 1);
			SetNameAtIndex(actorCT, name, 1 + actorCT->dataCount + actorCT->linksCount);
			++actorCT->linksCount;

			#endif
		}

		void teActorsTI::AddSignal(const c8 * name, u8 argumentsCount, const c8 * description)
		{
			if(enableSizeCalculation)
			{
				++reserveSignalsCount;
				#ifdef TE_CTTI
				reserveTotalStringsBytes += strlen(name) + 1;
				#endif
				return;
			}

			++GetClassRTTI(reserveClassCount - 1)->signalsCount;

			#ifdef TE_CTTI

			__actor::teClassTableCT * actorCT = GetClassCTTI(reserveClassCount - 1);
			SetNameAtIndex(actorCT, name, 1 + actorCT->dataCount + actorCT->linksCount + actorCT->signalsCount);
			++actorCT->signalsCount;

			#endif
		}

		void teActorsTI::AddSlot(const c8 * name, __actor::teSlotType4 slotPtr, u8 argumentsCount, const c8 * description)
		{
			if(enableSizeCalculation)
			{
				++reserveSlotsCount;
				#ifdef TE_CTTI
				reserveTotalStringsBytes += strlen(name) + 1;
				#endif
				return;
			}

			__actor::teClassTableRT * actorRT = GetClassRTTI(reserveClassCount - 1);

			u32 slotOffset = 0;
			teptr_t & ptr = *reinterpret_cast<teptr_t*>(rtti.Request(sizeof(teptr_t), &slotOffset));
			if(actorRT->slotsOffset == u32Max) 	actorRT->slotsOffset = slotOffset;
			ptr = reinterpret_cast<teptr_t>(slotPtr);
			++actorRT->slotsCount;

			#ifdef TE_CTTI

			__actor::teClassTableCT * actorCT = GetClassCTTI(reserveClassCount - 1);
			SetNameAtIndex(actorCT, name, 1 + actorCT->dataCount + actorCT->linksCount + actorCT->signalsCount + actorCT->slotsCount);
			++actorCT->slotsCount;

			#endif
		}

		#ifdef TE_CTTI

		void teActorsTI::Load(core::IBuffer * buffer)
		{
			buffer->Lock(core::BLT_READ);
			buffer->SetPosition(0);
			buffer->Read(&crc, 4);
			
			buffer->SetPosition(8);
			
			rtti.Load(buffer);
			ctti.Load(buffer);

			buffer->Unlock();
		}

		void teActorsTI::Save(core::IBuffer * buffer)
		{
			crc ^= core::GetCRC32(ctti.GetPool(), ctti.GetSize());

			//printf("crc result %u\n", crc);
			
			if(!buffer)
				return;

			u32 size = 8;
			size += rtti.GetSystemSize();
			size += ctti.GetSystemSize();

			buffer->Lock(core::BLT_WRITE);
			buffer->Allocate(size);
			buffer->SetPosition(0);
			buffer->Write(&crc, 4);
			buffer->Write("CTTI", 4);

			rtti.Save(buffer);
			ctti.Save(buffer);

			buffer->Unlock();
		}

		void teActorsTI::CTTIToJSON(core::IBuffer * buffer) const
		{
			teStringPool pool(64 * 1024); // TODO set size
			teStringConcate out;
			out.SetBuffer(pool.Allocate(pool.GetSize()));

			out.Add("[\n");

			for(u32 i = 0; i < CTTIGetActorsCount(); ++i)
			{
				const __actor::teClassTableCT * actor = GetClassCTTI(i);

				out.Add("\t{\n");

				out.Add("\t\t\"name\" : \"%s\",\n", CTTIGetNameByIndex(actor, 0)->name);
				out.Add("\n");

				out.Add("\t\t\"data\" :\n");
				out.Add("\t\t[\n");
				for(u16 j = 0; j < actor->dataCount; ++j)
					out.Add("\t\t\t{\"name\" : \"%s\"},\n", CTTIGetNameByIndex(actor, 1 + j)->name);
				out.Add("\t\t],\n");
				out.Add("\n");

				out.Add("\t\t\"links\" :\n");
				out.Add("\t\t[\n");
				for(u16 j = 0; j < actor->linksCount; ++j)
					out.Add("\t\t\t{\"name\" : \"%s\"},\n", CTTIGetNameByIndex(actor, 1 + actor->dataCount + j)->name);
				out.Add("\t\t],\n");
				out.Add("\n");

				out.Add("\t\t\"signals\" :\n");
				out.Add("\t\t[\n");
				for(u8 j = 0; j < actor->signalsCount; ++j)
					out.Add("\t\t\t{\"name\" : \"%s\"},\n", CTTIGetNameByIndex(actor, 1 + actor->dataCount + actor->linksCount + j)->name);
				out.Add("\t\t],\n");
				out.Add("\n");

				out.Add("\t\t\"slots\" :\n");
				out.Add("\t\t[\n");
				for(u8 j = 0; j < actor->slotsCount; ++j)
					out.Add("\t\t\t{\"name\" : \"%s\"},\n", CTTIGetNameByIndex(actor, 1 + actor->dataCount + actor->linksCount + actor->signalsCount + j)->name);
				out.Add("\t\t]\n");

				out.Add("\t},\n");
			}

			out.Add("]\n");

			teString str = out.BakeToString();
			u32 bytes = str.GetSize();

			buffer->Lock(core::BLT_WRITE);
			buffer->Allocate(bytes);
			buffer->SetPosition(0);
			buffer->Write(str.c_str(), bytes);
			buffer->Unlock();
		}

		void teActorsTI::Debug() const
		{
			TE_LOG_DBG("type information debug :");
			TE_LOG_DBG("rtti %i (all %i) bytes", rtti.GetAlive(), rtti.GetSize());
			TE_LOG_DBG("ctti %i (all %i) bytes", ctti.GetAlive(), ctti.GetSize());
			TE_LOG_DBG("ctti actors count %i", CTTIGetActorsCount());

			for(u16 i = 0; i < CTTIGetActorsCount(); ++i)
			{
				const __actor::teClassTableCT * actor = GetClassCTTI(i);

				TE_LOG_DBG("actor id %i (sz %i, dc %i, lc %i, sc %i, sl %i)", i, GetClassRTTI(i)->size, actor->dataCount, actor->linksCount, actor->slotsCount, actor->signalsCount);
				TE_LOG_DBG(" | actor name %s", CTTIGetNameByIndex(actor, 0)->name);

				for(u16 j = 0; j < actor->dataCount; ++j)
					TE_LOG_DBG(" | data id %i name %s", j, CTTIGetNameByIndex(actor, 1 + j)->name);

				for(u16 j = 0; j < actor->linksCount; ++j)
					TE_LOG_DBG(" | link id %i name %s", j, CTTIGetNameByIndex(actor, 1 + actor->dataCount + j)->name);

				for(u8 j = 0; j < actor->signalsCount; ++j)
					TE_LOG_DBG(" | signal id %i name %s", j, CTTIGetNameByIndex(actor, 1 + actor->dataCount + actor->linksCount + j)->name);

				for(u8 j = 0; j < actor->slotsCount; ++j)
					TE_LOG_DBG(" | slot id %i name %s", j, CTTIGetNameByIndex(actor, 1 + actor->dataCount + actor->linksCount + actor->signalsCount + j)->name);
			}
		}

		#endif

		void teActorsTI::Reserve()
		{
			rtti.Reserve(reserveClassCount * sizeof(__actor::teClassTableRT) + reserveSlotsCount * sizeof(teptr_t));
			rtti.Request(reserveClassCount * sizeof(__actor::teClassTableRT));

			#ifdef TE_CTTI

			ctti.Reserve(sizeof(u32) + reserveClassCount * sizeof(__actor::teClassTableCT) + (reserveClassCount + reserveDataCount + reserveLinksCount + reserveSignalsCount + reserveSlotsCount) * (sizeof(u32) + sizeof(__actor::teClassTableCTName)) + reserveTotalStringsBytes);
			ctti.Request(sizeof(u32) + reserveClassCount * sizeof(__actor::teClassTableCT));
			ctti.Request((reserveClassCount + reserveDataCount + reserveLinksCount + reserveSignalsCount + reserveSlotsCount) * sizeof(u32), &cttiNameOffsetsPosition);
			CTTIGetActorsCount() = reserveClassCount;

			#endif

			reserveClassCount = 0;
		}

		// ---------------------------------------------------------------------- actors machine

		// finalize pointers after after loading
		void teActorsMachine::ActorsFinalize()
		{
			if(!pool.GetAlive())
				return;
			
			__actor::teActorsTable * table = GetActorsTable();

			for(u32 i = 0; i < table->count; ++i)
			{
				if(!ti->GetClassRTTI(table->offsets[i].classType)->signalsCount)
					continue;

				u32 * signalOffsets = reinterpret_cast<u32*>(pool.At(table->offsets[i].offset + ti->GetClassRTTI(table->offsets[i].classType)->size));

				for(u8 j = 0; j < ti->GetClassRTTI(table->offsets[i].classType)->signalsCount; ++j)
				{
					if(!signalOffsets[j])
						continue;

					__actor::teSlotsTable * slotTable = reinterpret_cast<__actor::teSlotsTable*>(pool.At(table->offsets[i].offset + signalOffsets[j]));

					for(u32 k = 0; k < slotTable->count; ++k)
					{
						slotTable->entrys[k].actorPointer = reinterpret_cast<teptr_t>(pool.At((size_t)slotTable->entrys[k].actorPointer));
						slotTable->entrys[k].methodPointer = ti->GetSlotPtr((teptr_t)slotTable->entrys[k].methodPointer);
					}
				}
			}
		}

		u32 teActorsMachine::GetSystemSize() const
		{
			return pool.GetSystemSize();
		}

		void teActorsMachine::Save(core::IBuffer * buffer) const
		{
			pool.Save(buffer);
		}

		void teActorsMachine::Load(core::IBuffer * buffer)
		{
			pool.Load(buffer);
		}

		void teActorsMachine::Clear()
		{
			pool.Clear();
		}

		// compile-time methods (bake-time)

		#ifdef TE_CTTI

		void teActorsMachine::SetCalculateSizeMode(u1 enabled)
		{
			if(enabled)
			{
				enableSizeCalculation = true;
				totalActorsCount = 0;
				totalActorsBytes = 0;
				totalSignalsCount = 0;
				totalSignalLinksCount = 0;
			}
			else
			{
				enableSizeCalculation = false;
				Reserve();
			}
		}

		u32 teActorsMachine::ActorCreate(const c8 * typeName, u32 signalLinksCount)
		{
			if(enableSizeCalculation)
			{
				u16 classType = ti->GetClassType(typeName);
				TE_ASSERT(classType != u32Max);

				++totalActorsCount;
				totalActorsBytes += ti->GetClassRTTI(classType)->size;
				totalSignalsCount += ti->GetClassCTTI(classType)->signalsCount;
				totalSignalLinksCount += signalLinksCount;

				return 0;
			}

			u16 classType = ti->GetClassType(typeName);

			__actor::teActorsTable * table = GetActorsTable();

			table->offsets[table->count].classType = classType;
			memset(pool.Request(ti->GetClassRTTI(classType)->size, &table->offsets[table->count].offset), 0, ti->GetClassRTTI(classType)->size);

			u32 signalsDataSize = ti->GetClassCTTI(classType)->signalsCount * (sizeof(u32) + sizeof(__actor::teSlotsTable)) + signalLinksCount * sizeof(__actor::teSlotsEntry);
			memset(pool.Request(signalsDataSize), 0, signalsDataSize);

			lastActorIndex = u32Max;

			return table->count++;
		}

		void teActorsMachine::ActorConnect(u32 actor1Index, const c8 * signalName, u32 actor2Index, const c8 * slotName)
		{
			if(enableSizeCalculation)
				return;

			__actor::teActorsTable * table = GetActorsTable();

			if(lastActorIndex != actor1Index)
			{
				lastActorIndex = actor1Index;
				lastSignalOffset = ti->GetClassRTTI(table->offsets[actor1Index].classType)->size + ti->GetClassCTTI(table->offsets[actor1Index].classType)->signalsCount * sizeof(u32);
			}

			u32 & signalOffset = *reinterpret_cast<u32*>(pool.At(table->offsets[actor1Index].offset + ti->GetClassRTTI(table->offsets[actor1Index].classType)->size + ti->GetClassSignalIndex(table->offsets[actor1Index].classType, signalName) * sizeof(u32)));

			if(signalOffset == 0)
			{
				signalOffset = lastSignalOffset;
				lastSignalOffset += sizeof(__actor::teSlotsTable);
			}

			__actor::teSlotsTable * signalTable = reinterpret_cast<__actor::teSlotsTable*>(pool.At(table->offsets[actor1Index].offset + signalOffset));

			signalTable->entrys[signalTable->count].actorPointer = table->offsets[actor2Index].offset;
			signalTable->entrys[signalTable->count].methodPointer = ti->GetClassRTTI(table->offsets[actor2Index].classType)->slotsOffset + ti->GetClassSlotIndex(table->offsets[actor2Index].classType, slotName) * sizeof(teptr_t);
			++signalTable->count;

			lastSignalOffset += sizeof(__actor::teSlotsEntry);
		}

		void teActorsMachine::Debug()
		{
			TE_LOG_DBG("actors machine debug :");
			TE_LOG_DBG("pool size %i (all %i) bytes", pool.GetAlive(), pool.GetSize());

			__actor::teActorsTable * table = GetActorsTable();

			if(!table)
				return;

			TE_LOG_DBG("actors count %i", table->count);

			for(u32 i = 0; i < table->count; ++i)
			{
				TE_LOG_DBG("actor id %i classType %i offset %#010x signals count %i", i, table->offsets[i].classType, table->offsets[i].offset, ti->GetClassRTTI(table->offsets[i].classType)->signalsCount);

				if(!ti->GetClassRTTI(table->offsets[i].classType)->signalsCount)
					continue;

				u32 * signalOffsets = reinterpret_cast<u32*>(pool.At(table->offsets[i].offset + ti->GetClassRTTI(table->offsets[i].classType)->size));

				for(u8 j = 0; j < ti->GetClassRTTI(table->offsets[i].classType)->signalsCount; ++j)
				{
					if(!signalOffsets[j])
						continue;

					__actor::teSlotsTable * slotTable = reinterpret_cast<__actor::teSlotsTable*>(pool.At(table->offsets[i].offset + signalOffsets[j]));

					for(u32 k = 0; k < slotTable->count; ++k)
					{
						TE_LOG_DBG(" signal %i link %i to actor %#010x slot %#010x", j, k, slotTable->entrys[k].actorPointer, slotTable->entrys[k].methodPointer);
					}
				}
			}
		}

		void teActorsMachine::Reserve()
		{
			pool.Reserve(sizeof(__actor::teActorsTable) + totalActorsCount * sizeof(__actor::teActorInfo) + totalActorsBytes + totalSignalsCount * (sizeof(u32) + sizeof(__actor::teSlotsTable)) + totalSignalLinksCount * sizeof(__actor::teSlotsEntry));
			pool.Request(sizeof(__actor::teActorsTable) + totalActorsCount * sizeof(__actor::teActorInfo));

			GetActorsTable()->count = 0;
		}

		#endif
	}
}
