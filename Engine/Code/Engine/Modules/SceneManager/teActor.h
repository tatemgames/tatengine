/*
 *  teActor.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 12/1/11.
 *  Copyright 2011 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_IACTOR_H
#define TE_IACTOR_H

#include "teTypes.h"
#include "teConstArray.h"
#include "teLinkTools.h"
#include "teLogManager.h"

namespace te
{
	namespace scene
	{
		//#ifdef TE_DEBUG
		//	#define TE_ACTOR_DEBUG(__text) {printf(__text);}
		//#else
			#define TE_ACTOR_DEBUG(__text) {}
		//#endif

		class teFastScene;

		namespace __actor
		{
			typedef void (*teClassServiceConstruct)(void *, teFastScene *);
			typedef void (*teClassService)(void *);
			typedef void (*teSlotType4)(teptr_t, f32, f32, f32, f32);

			struct teSlotsEntry
			{
				teptr_t actorPointer;
				teptr_t methodPointer;
			};

			struct teSlotsTable
			{
				u32 count;
				teSlotsEntry entrys[];

				static TE_INLINE void CallTable(teptr_t table, f32 a0 = 0.0f, f32 a1 = 0.0f, f32 a2 = 0.0f, f32 a3 = 0.0f)
				{
					teSlotsTable * slotsTable = reinterpret_cast<teSlotsTable*>(table);
					for(u32 i = 0; i < slotsTable->count; ++i)
						(*reinterpret_cast<teSlotType4>(slotsTable->entrys[i].methodPointer))(slotsTable->entrys[i].actorPointer, a0, a1, a2, a3);
				}
			};

			struct teClassTableRT
			{
				teptr_t constructor;
				teptr_t destructor;
				teptr_t onUpdate;
				teptr_t onReset;
				u32 size;
				u32 slotsOffset;
				u16 dataCount;
				u8 signalsCount;
				u8 slotsCount;
			};

			struct teClassTableCTName
			{
				u16 length;
				c8 name[];

				TE_INLINE u1 IsEqual(const c8 * otherName, u16 otherNameLength) const
				{
					if(length == otherNameLength)
						return !strcmp(name, otherName);
					else
						return false;
				}
			};

			struct teClassTableCT
			{
				u32 namesOffset;
				u16 linksCount;
				u16 dataCount;
				u16 _reserved;
				u8 signalsCount;
				u8 slotsCount;
			};

			struct teActorInfo
			{
				u32 offset;
				u16 classType;
				u16 _reserved;
			};

			struct teActorsTable
			{
				u32 count;
				teActorInfo offsets[];
			};
		}

		// ---------------------------------------------------------------------- actors type information

		class teActorsTI
		{
		public:

			// run-time methods

			TE_INLINE void CallClassConstructor(u16 classType, void * actor, teFastScene * scene) const
			{
				teptr_t constructor = GetClassRTTI(classType)->constructor;

				if(constructor)
					(*reinterpret_cast<__actor::teClassServiceConstruct>(constructor))(actor, scene);
			}

			TE_INLINE void CallClassDestructor(u16 classType, void * actor) const
			{
				teptr_t destructor = GetClassRTTI(classType)->destructor;

				if(destructor)
					(*reinterpret_cast<__actor::teClassService>(destructor))(actor);
			}

			TE_INLINE void CallClassUpdate(u16 classType, void * actor) const
			{
				teptr_t onUpdate = GetClassRTTI(classType)->onUpdate;

				if(onUpdate)
					(*reinterpret_cast<__actor::teClassService>(onUpdate))(actor);
			}

			TE_INLINE void CallClassReset(u16 classType, void * actor) const
			{
				teptr_t onReset = GetClassRTTI(classType)->onReset;

				if(onReset)
					(*reinterpret_cast<__actor::teClassService>(onReset))(actor);
			}

			TE_INLINE __actor::teClassTableRT * GetClassRTTI(u16 classType)
			{
				return reinterpret_cast<__actor::teClassTableRT*>(rtti.At(classType * sizeof(__actor::teClassTableRT)));
			}

			TE_INLINE const __actor::teClassTableRT * GetClassRTTI(u16 classType) const
			{
				return reinterpret_cast<const __actor::teClassTableRT*>(rtti.At(classType * sizeof(__actor::teClassTableRT)));
			}

			TE_INLINE teptr_t GetSlotPtr(size_t rttiOffset) const
			{
				return *reinterpret_cast<const teptr_t*>(rtti.At(rttiOffset));
			}

			void SetCalculateSizeMode(u1 enabled);

			void AddClass(u32 size, const c8 * name, __actor::teClassServiceConstruct constructor = NULL, __actor::teClassService destructor = NULL, __actor::teClassService onUpdate = NULL, __actor::teClassService onReset = NULL, const c8 * description = NULL);

			void AddData(const c8 * name, u1 linkable = false, const c8 * description = NULL);

			void AddLink(const c8 * name, u1 optional = false, const c8 * acceptableTypes = "", const c8 * description = NULL);

			void AddSignal(const c8 * name, u8 argumentsCount = 4, const c8 * description = NULL);

			void AddSlot(const c8 * name, __actor::teSlotType4 slotPtr, u8 argumentsCount = 4, const c8 * description = NULL);
			
			// compile-time methods (bake-time)

			#ifdef TE_CTTI

			TE_INLINE u16 GetClassType(const c8 * name) const
			{
				u16 count = CTTIGetActorsCount();
				u16 nameLength = (u16)strlen(name);
				
				for(u16 i = 0; i < count; ++i)
					if(CTTIGetNameByIndex(GetClassCTTI(i), 0)->IsEqual(name, nameLength))
						return i;

				return u16Max;
			}

			TE_INLINE __actor::teClassTableCT * GetClassCTTI(u16 classType)
			{
				return reinterpret_cast<__actor::teClassTableCT*>(ctti.At(sizeof(u32) + classType * sizeof(__actor::teClassTableCT)));
			}

			TE_INLINE const __actor::teClassTableCT * GetClassCTTI(u16 classType) const
			{
				return reinterpret_cast<const __actor::teClassTableCT*>(ctti.At(sizeof(u32) + classType * sizeof(__actor::teClassTableCT)));
			}

			TE_INLINE u16 GetClassDataIndex(u16 classType, const c8 * name) const
			{
				const __actor::teClassTableCT * actor = GetClassCTTI(classType);
				u16 nameLength = (u16)strlen(name);

				for(u16 i = 0; i < actor->dataCount; ++i)
					if(CTTIGetNameByIndex(actor, 1 + i)->IsEqual(name, nameLength))
						return i;

				return u16Max;
			}

			TE_INLINE u16 GetClassLinkIndex(u16 classType, const c8 * name) const
			{
				const __actor::teClassTableCT * actor = GetClassCTTI(classType);
				u16 nameLength = (u16)strlen(name);
				
				for(u16 i = 0; i < actor->linksCount; ++i)
					if(CTTIGetNameByIndex(actor, 1 + actor->dataCount + i)->IsEqual(name, nameLength))
						return i;
				
				return u16Max;
			}

			TE_INLINE u8 GetClassSignalIndex(u16 classType, const c8 * name) const
			{
				const __actor::teClassTableCT * actor = GetClassCTTI(classType);
				u16 nameLength = (u16)strlen(name);
				
				for(u8 i = 0; i < actor->signalsCount; ++i)
					if(CTTIGetNameByIndex(actor, 1 + actor->dataCount + actor->linksCount + i)->IsEqual(name, nameLength))
						return i;
				
				return u8Max;
			}

			TE_INLINE u8 GetClassSlotIndex(u16 classType, const c8 * name) const
			{
				const __actor::teClassTableCT * actor = GetClassCTTI(classType);
				u16 nameLength = (u16)strlen(name);
				
				for(u8 i = 0; i < actor->slotsCount; ++i)
					if(CTTIGetNameByIndex(actor, 1 + actor->dataCount + actor->linksCount + actor->signalsCount + i)->IsEqual(name, nameLength))
						return i;
				
				return u8Max;
			}

			void Load(core::IBuffer * buffer);
			void Save(core::IBuffer * buffer);

			void CTTIToJSON(core::IBuffer * buffer) const;

			void Debug() const;
			
			TE_INLINE u32 GetCTTICRC() const {return crc;}

			#endif

		protected:

			// service run-time methods
			teConstArray<u8> rtti;
			u1 enableSizeCalculation;
			u16 reserveClassCount;
			u32 reserveDataCount;
			u32 reserveLinksCount;
			u32 reserveSignalsCount;
			u32 reserveSlotsCount;
			u32 reserveTotalStringsBytes;
			u32 crc;

			void Reserve();

			// service compile-time methods

			#ifdef TE_CTTI

			teConstArray<u8> ctti;
			u32 cttiNameOffsetsPosition;

			TE_INLINE u16 & CTTIGetActorsCount()
			{
				return *reinterpret_cast<u16*>(ctti.GetPool());
			}

			TE_INLINE const u16 & CTTIGetActorsCount() const
			{
				return *reinterpret_cast<const u16*>(ctti.GetPool());
			}

			TE_INLINE u32 * CTTIGetNameOffsetsArray(const __actor::teClassTableCT * actor)
			{
				return reinterpret_cast<u32*>(ctti.At(actor->namesOffset));
			}

			TE_INLINE const u32 * CTTIGetNameOffsetsArray(const __actor::teClassTableCT * actor) const
			{
				return reinterpret_cast<const u32*>(ctti.At(actor->namesOffset));
			}

			TE_INLINE __actor::teClassTableCTName * CTTIGetNameByIndex(const __actor::teClassTableCT * actor, u32 index)
			{
				return reinterpret_cast<__actor::teClassTableCTName*>(ctti.At(*reinterpret_cast<const u32*>(ctti.At(actor->namesOffset + sizeof(u32) * index))));
			}

			TE_INLINE const __actor::teClassTableCTName * CTTIGetNameByIndex(const __actor::teClassTableCT * actor, u32 index) const
			{
				return reinterpret_cast<const __actor::teClassTableCTName*>(ctti.At(*reinterpret_cast<const u32*>(ctti.At(actor->namesOffset + sizeof(u32) * index))));
			}

			TE_INLINE void SetNameAtIndex(const __actor::teClassTableCT * actor, const c8 * name, u32 index)
			{
				u16 nameLength = (u16)strlen(name);
				ctti.Request(sizeof(__actor::teClassTableCTName) + nameLength + 1, &CTTIGetNameOffsetsArray(actor)[index]);
				CTTIGetNameByIndex(actor, index)->length = nameLength;
				memcpy(CTTIGetNameByIndex(actor, index)->name, name, nameLength + 1);
				cttiNameOffsetsPosition += sizeof(u32);
			}

			#endif
		};

		// ---------------------------------------------------------------------- actors machine

		class teActorsMachine
		{
		public:

			// run-time methods

			TE_INLINE void ActorsConstruct(teFastScene * scene)
			{
				if(!pool.GetAlive())
					return;

				const __actor::teActorsTable * actors = GetActorsTable();
				
				for(u32 i = 0; i < actors->count; ++i)
					ti->CallClassConstructor(actors->offsets[i].classType, pool.At(actors->offsets[i].offset), scene);
			}

			TE_INLINE void ActorsDestruct()
			{
				if(!pool.GetAlive())
					return;

				const __actor::teActorsTable * actors = GetActorsTable();
				
				for(u32 i = 0; i < actors->count; ++i)
					ti->CallClassDestructor(actors->offsets[i].classType, pool.At(actors->offsets[i].offset));
			}

			TE_INLINE void ActorsUpdate()
			{
				if(!pool.GetAlive())
					return;

				const __actor::teActorsTable * actors = GetActorsTable();

				for(u32 i = 0; i < actors->count; ++i)
					ti->CallClassUpdate(actors->offsets[i].classType, pool.At(actors->offsets[i].offset));
			}

			TE_INLINE void ActorsReset() 
			{
				if(!pool.GetAlive())
					return;

				const __actor::teActorsTable * actors = GetActorsTable();

				for(u32 i = 0; i < actors->count; ++i)
					ti->CallClassReset(actors->offsets[i].classType, pool.At(actors->offsets[i].offset));
			}

			// finalize pointers after after loading
			void ActorsFinalize();

			TE_INLINE void SetTypeInformation(teActorsTI * setTI)
			{
				ti = setTI;
			}

			TE_INLINE teActorsTI * GetTypeInformation()
			{
				return ti;
			}

			TE_INLINE const teActorsTI * GetTypeInformation() const
			{
				return ti;
			}

			TE_INLINE void * GetActor(u32 actorIndex)
			{
				return reinterpret_cast<void*>(pool.At(GetActorsTable()->offsets[actorIndex].offset));
			}

			TE_INLINE f32 & GetActorData(u32 actorIndex, u16 dataIndex)
			{
				return *reinterpret_cast<f32*>(pool.At(GetActorsTable()->offsets[actorIndex].offset + dataIndex * sizeof(f32)));
			}

			TE_INLINE teptr_t & GetActorLink(u32 actorIndex, u16 linkIndex)
			{
				teptr_t baseOffset = GetActorsTable()->offsets[actorIndex].offset;
				teptr_t dataOffset = ti->GetClassRTTI(GetActorsTable()->offsets[actorIndex].classType)->dataCount * sizeof(f32);
				teptr_t linkOffset = linkIndex * sizeof(teptr_t);
				return *reinterpret_cast<teptr_t*>(pool.At(baseOffset + dataOffset + linkOffset));
			}

			u32 GetSystemSize() const;

			void Save(core::IBuffer * buffer) const;

			void Load(core::IBuffer * buffer);

			void Clear();

			// compile-time methods (bake-time)

			#ifdef TE_CTTI

			void SetCalculateSizeMode(u1 enabled);

			u32 ActorCreate(const c8 * typeName, u32 signalLinksCount);

			void ActorConnect(u32 actor1Index, const c8 * signalName, u32 actor2Index, const c8 * slotName); // all links for one signal must be set in one batch

			void Debug();

			#endif

		protected:
			teActorsTI * ti;

			// service run-time methods
			teConstArray<u8> pool;

			TE_INLINE __actor::teActorsTable * GetActorsTable()
			{
				if(pool.GetAlive())
					return reinterpret_cast<__actor::teActorsTable*>(pool.GetPool());
				else
					return NULL;
			}

			TE_INLINE const __actor::teActorsTable * GetActorsTable() const
			{
				if(pool.GetAlive())
					return reinterpret_cast<const __actor::teActorsTable*>(pool.GetPool());
				else
					return NULL;
			}

			// service compile-time methods

			#ifdef TE_CTTI

			u32 totalActorsCount;
			u32 totalActorsBytes;
			u32 totalSignalsCount;
			u32 totalSignalLinksCount;
			u32 lastActorIndex;
			u32 lastSignalOffset;
			u1 enableSizeCalculation;

			void Reserve();

			#endif
		};

		// --------------------------------------------------------------------------------- Actor factory functions

		#define TE_ACTOR_PROXY_NAMES_NONE NULL, NULL, NULL, NULL

		#define TE_ACTOR_PROXY_NU(__class_name) \
			namespace __actor \
			{ \
				TE_FUNC void __##__class_name##_Construct(void * at, teFastScene * scene) \
				{ \
					TE_ACTOR_DEBUG("__"#__class_name"_Construct\n"); \
					TE_NEW_S_P(reinterpret_cast<__class_name*>(at), __class_name(scene)); \
				} \
				\
				TE_FUNC void __##__class_name##_Destruct(void * at) \
				{ \
					TE_ACTOR_DEBUG("__"#__class_name"_Destruct\n"); \
					TE_DELETE_P(reinterpret_cast<__class_name*>(at), ~__class_name()); \
				} \
				\
				TE_FUNC void __##__class_name##_OnReset(void * at) \
				{ \
					TE_ACTOR_DEBUG("__"#__class_name"_OnReset\n"); \
					(reinterpret_cast<__class_name*>(at))->OnReset(); \
				} \
				\
			}

		#define TE_ACTOR_PROXY_NAMES_NU(__class_name) &__actor::__##__class_name##_Construct, &__actor::__##__class_name##_Destruct, NULL

		#define TE_ACTOR_PROXY(__class_name) \
			TE_ACTOR_PROXY_NU(__class_name) \
			namespace __actor \
			{ \
				TE_FUNC void __##__class_name##_OnUpdate(void * at) \
				{ \
					TE_ACTOR_DEBUG("__"#__class_name"_OnUpdate\n"); \
					(reinterpret_cast<__class_name*>(at))->OnUpdate(); \
				} \
			}

		#define TE_ACTOR_PROXY_NAMES(__class_name) &__actor::__##__class_name##_Construct, &__actor::__##__class_name##_Destruct, &__actor::__##__class_name##_OnUpdate, &__actor::__##__class_name##_OnReset

		#define TE_ACTOR_SIGNAL(__signal_index, __signal_name) \
			TE_INLINE void __signal_name(f32 arg0 = 0.0f, f32 arg1 = 0.0f, f32 arg2 = 0.0f, f32 arg3 = 0.0f) \
			{ \
				TE_ACTOR_DEBUG("sig("#__signal_index")->"#__signal_name"\n"); \
				u32 * offsets = reinterpret_cast<u32*>(reinterpret_cast<teptr_t>(this) + sizeof(*this)); \
				if(offsets[__signal_index]) \
					__actor::teSlotsTable::CallTable((teptr_t)this + (teptr_t)offsets[__signal_index], arg0, arg1, arg2, arg3); \
			}

		#define TE_ACTOR_SLOT_DECLARE(__class_name, __slot_name) \
			namespace __actor \
			{ \
				TE_FUNC void __##__class_name##_Slot##__slot_name(teptr_t at, f32 a0, f32 a1, f32 a2, f32 a3); \
			}

		#define TE_ACTOR_SLOT_EX(__class_name, __slot_name, __slot_args) \
			namespace __actor \
			{ \
				TE_FUNC void __##__class_name##_Slot##__slot_name(teptr_t at, f32 a0, f32 a1, f32 a2, f32 a3) \
				{ \
					TE_ACTOR_DEBUG("__"#__class_name"_Slot"#__slot_name"\n"); \
					(reinterpret_cast<__class_name*>(at))->__slot_name __slot_args; \
				} \
			}

		#define TE_ACTOR_SLOT_PROXY(__class_name, __slot_name) &__actor::__##__class_name##_Slot##__slot_name

		// slot with no arguments
		#define TE_ACTOR_SLOT(__class_name, __slot_name) TE_ACTOR_SLOT_EX(__class_name, __slot_name, ())
		#define TE_ACTOR_SLOT_0(__class_name, __slot_name) TE_ACTOR_SLOT_EX(__class_name, __slot_name, ())

		// slot with 1 argument
		#define TE_ACTOR_SLOT_1(__class_name, __slot_name) TE_ACTOR_SLOT_EX(__class_name, __slot_name, (a0))

		// slot with 2 arguments
		#define TE_ACTOR_SLOT_2(__class_name, __slot_name) TE_ACTOR_SLOT_EX(__class_name, __slot_name, (a0, a1))

		// slot with 3 arguments
		#define TE_ACTOR_SLOT_3(__class_name, __slot_name) TE_ACTOR_SLOT_EX(__class_name, __slot_name, (a0, a1, a2))

		// slot with 4 arguments
		#define TE_ACTOR_SLOT_4(__class_name, __slot_name) TE_ACTOR_SLOT_EX(__class_name, __slot_name, (a0, a1, a2, a3))

		// --------------------------------------------------------------------------------- Actor runtime data link

		struct teActorRTDataLink
		{
			u32 assetActorIndex;
			u16 actorLinkIndex;
			teLink link;

			TE_INLINE void Clear()
			{
				assetActorIndex = u32Max;
				actorLinkIndex = u16Max;
			}
		};
	}
}

#endif