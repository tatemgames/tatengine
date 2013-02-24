/*
 *  teActorScriptLua.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 10/7/12.
 *  Copyright 2012 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEACTORSCRIPTLUA_H
#define TE_TEACTORSCRIPTLUA_H

#include "teActor.h"

extern "C"
{
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}

/*
TatEngine lua binding

tutorial program :

enableTest = 0; -- will be global variable for all scripts
local t = 0; -- will be local variable

function metaname.init()
end

function metaname.deinit()
end

function metaname.onUpdate() -- will be called on update
	if en == 1 then
		t = t + 1;
		local a = t * math.pi / 180;
		metaname.link0.pos_x = math.cos(a) * 128;
		metaname.link0.pos_y = math.sin(a) * 128;
		metaname.link1.color_r = math.random() * 255;
	end
end

function metaname.slot0(a, b, c, d) -- will be called on slot0
	en = 1 - en;
	signal0(metaname.actor, 1, 2, 3, 4); -- call actor signal
end
*/

namespace te
{
	namespace scene
	{
		#define TE_LUA_SPECIAL_ALLOC

		#ifdef TE_LUA_SPECIAL_ALLOC

		// experimental lua allocator
		// make less memory fragmentation with pool stacks system
		class teLuaAlloc
		{
		public:
			teLuaAlloc()
			{
				Flush();
			}

			~teLuaAlloc()
			{
				Flush();
			}

			TE_INLINE void Flush()
			{
				for(u32 i = 0; i < pools.GetAlive(); ++i)
					pools[i].Clear();
				pools.Clear();
			}

			TE_INLINE void * Allocate(u32 size)
			{
				u32 poolSize = GetObjectSize(size);

				for(u32 i = 0; i < pools.GetAlive(); ++i)
					if((pools[i].objectSize == poolSize) && pools[i].AnyFree())
						return pools[i].Allocate();

				if(pools.GetAlive() >= pools.GetSize())
					pools.ReserveMore(16);

				teObjectPool * pool = pools.Request();
				pool->Clear();
				pool->objectSize = poolSize;

				return pool->Allocate();
			}

			TE_INLINE void Free(void * ptr, u32 size)
			{
				u32 poolSize = GetObjectSize(size);

				for(u32 i = 0; i < pools.GetAlive(); ++i)
					if(pools[i].objectSize == poolSize)
					{
						u1 result = pools[i].Free(ptr);

						if(result && pools[i].AllFree())
						{
							pools[i].states.Clear();
							pools[i].bytes.Clear();
						}
					}

				for(u32 i = 0; i < pools.GetAlive(); ++i)
					if(!pools[i].AllFree())
						return;

				pools.Clear();
			}

			TE_INLINE void * Reallocate(void * ptr, u32 oldSize, u32 newSize)
			{
				void * newPtr = Allocate(newSize);
				memcpy(newPtr, ptr, teMin(oldSize, newSize));
				Free(ptr, oldSize);
				return newPtr;
			}

		protected:
			struct teObjectPool
			{
				teConstArray<u8> states;
				teConstArray<u8> bytes;
				u32 objectSize;

				TE_INLINE void Clear()
				{
					bytes.Clear();
					states.Clear();
					objectSize = u32Max;
				}

				TE_INLINE void * Allocate()
				{
					if(states.GetAlive() == 0)
						Reserve();

					for(u32 i = 0; i < states.GetAlive(); ++i)
						if(states[i] == 0)
						{
							states[i] = 1;
							memset(bytes.At(i * objectSize), 0, objectSize);
							return bytes.At(i * objectSize);
						}

					return NULL;
				}

				TE_INLINE u1 Free(void * ptr)
				{
					if(states.GetAlive() == 0)
						return false;

					teptr_t a = (teptr_t)&bytes.GetFirst();
					teptr_t b = (teptr_t)&bytes.GetLast();
					teptr_t c = (teptr_t)ptr;

					if((c < a) || (c > b))
						return false;

					u32 i = bytes.GetIndexInArray((u8*)ptr) / objectSize;

					if(i < states.GetAlive())
					{
						states[i] = 0;
						return true;
					}
					else
						return false;
				}

				TE_INLINE u1 AnyFree()
				{
					for(u32 i = 0; i < states.GetAlive(); ++i)
						if(states[i] == 0)
							return true;
					return false;
				}

				TE_INLINE u1 AllFree()
				{
					for(u32 i = 0; i < states.GetAlive(); ++i)
						if(states[i] != 0)
							return false;
					return true;
				}

				TE_INLINE void Reserve(u32 ptrs = 96)
				{
					states.Reserve(ptrs);
					states.Request(states.GetSize());

					for(u32 i = 0; i < states.GetAlive(); ++i)
						states[i] = 0;

					bytes.Reserve(states.GetSize() * objectSize);
					bytes.Request(bytes.GetSize());
				}
			};

			teConstArray<teObjectPool> pools;

			TE_INLINE u32 GetObjectSize(u32 x) const
			{
				if(!x)
					return 0;

				--x;
				x |= x >> 1;
				x |= x >> 2;
				x |= x >> 4;
				x |= x >> 8;
				x |= x >> 16;
				x = x + 1;

				return (x / 2) * 2; // small trick, make result be 2, 8, 16, ...
			}
		};

		#endif

		class teActorScriptLua
		{
		public:
			teActorScriptLua(teFastScene * setScene)
				:scene(setScene)
			{
				InitLua();

				metaId = GetNewMetaId(scene);
				FormMetaName();

				NamespaceInitialization();
				CompileScript();

				lua_getglobal(L, metaName);
				lua_getfield(L, -1, "init");

				if(lua_type(L,-1) <= LUA_TNIL)
					lua_pop(L, -2);
				else
				{
					lua_remove(L, -2);
					s32 s = lua_pcall(L, 0, 0, 0);
					CheckError(s);
				}
			}

			~teActorScriptLua()
			{
				lua_getglobal(L, metaName);
				lua_getfield(L, -1, "deinit");

				if(lua_type(L,-1) <= LUA_TNIL)
					lua_pop(L, -2);
				else
				{
					lua_remove(L, -2);
					s32 s = lua_pcall(L, 0, 0, 0);
					CheckError(s);
				}

				CloseLua();
				//MemoryLog();
			}

			void OnUpdate()
			{
				//if(input::GetInputManager()->IsKeyHit(input::KC_SPACE))
				//	MemoryLog();

				lua_getglobal(L, metaName);
				lua_getfield(L, -1, "onUpdate");

				if(lua_type(L,-1) <= LUA_TNIL)
				{
					lua_pop(L, -2);
				}
				else
				{
					lua_remove(L, -2);
					s32 s = lua_pcall(L, 0, 0, 0);
					CheckError(s);
				}
			}

			TE_INLINE void OnSlot(u8 index, f32 a = 0.0f, f32 b = 0.0f, f32 c = 0.0f, f32 d = 0.0f)
			{
				c8 slotName[6] = {'s', 'l', 'o', 't', '0' + index, '\0'};

				lua_getglobal(L, metaName);
				lua_getfield(L, -1, slotName);

				if(lua_type(L,-1) <= LUA_TNIL)
				{
					lua_pop(L, -2);
					return;
				}

				lua_remove(L, -2);

				lua_pushnumber(L, a);
				lua_pushnumber(L, b);
				lua_pushnumber(L, c);
				lua_pushnumber(L, d);

				s32 s = lua_pcall(L, 4, 0, 0);
				CheckError(s);
			}

			TE_INLINE void OnSignal(u8 index, f32 a, f32 b, f32 c, f32 d)
			{
				switch(index)
				{
				case 0: Signal0(a, b, c, d); break;
				case 1: Signal1(a, b, c, d); break;
				case 2: Signal2(a, b, c, d); break;
				case 3: Signal3(a, b, c, d); break;
				case 4: Signal4(a, b, c, d); break;
				case 5: Signal5(a, b, c, d); break;
				case 6: Signal6(a, b, c, d); break;
				case 7: Signal7(a, b, c, d); break;
				default: break;
				}
			}

			TE_INLINE void Slot0(f32 a, f32 b, f32 c, f32 d) {OnSlot(0, a, b, c, d);}
			TE_INLINE void Slot1(f32 a, f32 b, f32 c, f32 d) {OnSlot(1, a, b, c, d);}
			TE_INLINE void Slot2(f32 a, f32 b, f32 c, f32 d) {OnSlot(2, a, b, c, d);}
			TE_INLINE void Slot3(f32 a, f32 b, f32 c, f32 d) {OnSlot(3, a, b, c, d);}
			TE_INLINE void Slot4(f32 a, f32 b, f32 c, f32 d) {OnSlot(4, a, b, c, d);}
			TE_INLINE void Slot5(f32 a, f32 b, f32 c, f32 d) {OnSlot(5, a, b, c, d);}
			TE_INLINE void Slot6(f32 a, f32 b, f32 c, f32 d) {OnSlot(6, a, b, c, d);}
			TE_INLINE void Slot7(f32 a, f32 b, f32 c, f32 d) {OnSlot(7, a, b, c, d);}

			TE_ACTOR_SIGNAL(0, Signal0);
			TE_ACTOR_SIGNAL(1, Signal1);
			TE_ACTOR_SIGNAL(2, Signal2);
			TE_ACTOR_SIGNAL(3, Signal3);
			TE_ACTOR_SIGNAL(4, Signal4);
			TE_ACTOR_SIGNAL(5, Signal5);
			TE_ACTOR_SIGNAL(6, Signal6);
			TE_ACTOR_SIGNAL(7, Signal7);

		protected:
			teStringInfo * script;
			void * link0;
			void * link1;
			void * link2;
			void * link3;
			void * link4;
			void * link5;
			void * link6;
			void * link7;

			teFastScene * scene;

			struct tePrimitiveLinkData
			{
				void * ptr;
				ELinkDataType dataType;
			};

			tePrimitiveLinkData primitiveLinks[4];

			u32 metaId;
			c8 metaName[9];

			// ---------------------------------------------------------------------------------------- Utils

			void NamespaceInitialization()
			{
				lua_newtable(L);
				lua_setglobal(L, metaName);

				lua_getglobal(L, metaName);
				lua_pushlightuserdata(L, this);
				lua_setfield(L, -2, "actor");

				BindLink("link0", link0, 0);
				BindLink("link1", link1, 1);
				BindLink("link2", link2, 2);
				BindLink("link3", link3, 3);
				BindLink("link4", link4, 4);
				BindLink("link5", link5, 5);
				BindLink("link6", link6, 6);
				BindLink("link7", link7, 7);
			}

			void CompileScript()
			{
				if(!script)
					return;

				c8 * temp = (c8*)TE_ALLOCATE(script->size + 1);
				strcpy(temp, script->data);

				{
					c8 * temp2 = temp;
					while(temp2 = strstr(temp2, "metaname"))
						memcpy(temp2, metaName, 8);
				}

				s32 s = luaL_dostring(L, temp);
				CheckError(s);

				TE_FREE(temp);
			}

			// ---------------------------------------------------------------------------------------- Namespace management

			static u32 metaIdCounter;
			static u8 metaIdCounterLastStage;

			static u32 GetNewMetaId(teFastScene * checkScene)
			{
				if(metaIdCounterLastStage != checkScene->GetStage())
				{
					metaIdCounterLastStage = checkScene->GetStage();
					metaIdCounter = 0;
				}
				TE_ASSERT_NODEBUG(metaIdCounter < 1000);
				return metaIdCounter++;
			}
			void FormMetaName()
			{
				s32 l = sprintf(metaName, "meta%04i", metaId);
				TE_ASSERT((l + 1) <= 9);
			}

			// ---------------------------------------------------------------------------------------- Memory management
			#ifdef TE_DEBUG
				struct teLuaMemoryUsage
				{
					s32 allocCount;
					s32 reallocCount;
					s32 freeCount;
					s32 allocSize;
					s32 reallocSize;
					s32 totalSize;
					s32 totalMaxSize;

					teLuaMemoryUsage()
					{
						allocCount = 0;
						reallocCount = 0;
						freeCount = 0;
						allocSize = 0;
						reallocSize = 0;
						totalSize = 0;
						totalMaxSize = 0;
					}
				};
			#endif

			#ifdef TE_DEBUG
				static teLuaMemoryUsage memoryUsage;
			#endif

			#ifdef TE_LUA_SPECIAL_ALLOC
				static teLuaAlloc luaSpecAlloc;
			#endif

			static void * teLuaAlloc(void * userData, void * ptr, size_t oldSize, size_t newSize)
			{
				if(oldSize == 0)
				{
					if(newSize > 0)
					{
						#ifdef TE_DEBUG
							memoryUsage.allocCount++;
							memoryUsage.allocSize += newSize;
							memoryUsage.totalSize += newSize;
							if(memoryUsage.totalSize > memoryUsage.totalMaxSize)
								memoryUsage.totalMaxSize = memoryUsage.totalSize;
						#endif
						#ifdef TE_LUA_SPECIAL_ALLOC
							return luaSpecAlloc.Allocate(newSize);
						#else
							return TE_ALLOCATE(newSize);
						#endif
					}
					else
						return ptr;
				}
				else
				{
					if(newSize > 0)
					{
						if(ptr)
						{
							#ifdef TE_DEBUG
								memoryUsage.reallocCount++;
								memoryUsage.reallocSize += newSize;
								memoryUsage.totalSize += newSize - oldSize;
								if(memoryUsage.totalSize > memoryUsage.totalMaxSize)
									memoryUsage.totalMaxSize = memoryUsage.totalSize;
							#endif
							#ifdef TE_LUA_SPECIAL_ALLOC
								return luaSpecAlloc.Reallocate(ptr, oldSize, newSize);
							#else
								return TE_REALLOCATE(ptr, newSize);
							#endif
						}
						else
						{
							#ifdef TE_DEBUG
								memoryUsage.allocCount++;
								memoryUsage.allocSize += newSize;
								memoryUsage.totalSize += newSize;
								if(memoryUsage.totalSize > memoryUsage.totalMaxSize)
									memoryUsage.totalMaxSize = memoryUsage.totalSize;
							#endif
							#ifdef TE_LUA_SPECIAL_ALLOC
								return luaSpecAlloc.Allocate(newSize);
							#else
								return TE_ALLOCATE(newSize);
							#endif
						}
					}
					else
					{
						if(ptr)
						{
							#ifdef TE_LUA_SPECIAL_ALLOC
								luaSpecAlloc.Free(ptr, oldSize);
							#else
								TE_FREE(ptr);
							#endif

							#ifdef TE_DEBUG
								memoryUsage.freeCount++;
								memoryUsage.totalSize -= oldSize;
							#endif
						}

						return NULL;
					}
				}
			}

			static void MemoryLog()
			{
				#ifdef TE_DEBUG
				TE_LOG("total lua memory : ");
				TE_LOG("allocs/reallocs/free : %i, %i, %i", memoryUsage.allocCount, memoryUsage.reallocCount, memoryUsage.freeCount);
				TE_LOG("allocs size/reallocs size/allocs+reallocs s : %i, %i", memoryUsage.allocSize, memoryUsage.reallocSize, memoryUsage.allocSize + memoryUsage.reallocSize);
				TE_LOG("size now : %i, max size : %i", memoryUsage.totalSize, memoryUsage.totalMaxSize);
				#endif
			}

			// ---------------------------------------------------------------------------------------- Bindings

			static s32 LuaSignal(lua_State * L, u8 index)
			{
				s32 count = lua_gettop(L);
				teActorScriptLua * data = (teActorScriptLua*)lua_topointer(L, 1);

				f32 a[4] = {0.0f, 0.0f, 0.0f, 0.0f};
				for(s32 i = 2; i <= count; ++i)
					a[i - 2] = lua_tonumber(L, i);

				switch(index)
				{
				case 0: data->Signal0(a[0], a[1], a[2], a[3]); break;
				case 1: data->Signal1(a[0], a[1], a[2], a[3]); break;
				case 2: data->Signal2(a[0], a[1], a[2], a[3]); break;
				case 3: data->Signal3(a[0], a[1], a[2], a[3]); break;
				case 4: data->Signal4(a[0], a[1], a[2], a[3]); break;
				case 5: data->Signal5(a[0], a[1], a[2], a[3]); break;
				case 6: data->Signal6(a[0], a[1], a[2], a[3]); break;
				case 7: data->Signal7(a[0], a[1], a[2], a[3]); break;
				default: break;
				}
  				return 0;
			}

			static s32 LuaSignal0(lua_State * L) {return LuaSignal(L, 0);}
			static s32 LuaSignal1(lua_State * L) {return LuaSignal(L, 1);}
			static s32 LuaSignal2(lua_State * L) {return LuaSignal(L, 2);}
			static s32 LuaSignal3(lua_State * L) {return LuaSignal(L, 3);}
			static s32 LuaSignal4(lua_State * L) {return LuaSignal(L, 4);}
			static s32 LuaSignal5(lua_State * L) {return LuaSignal(L, 5);}
			static s32 LuaSignal6(lua_State * L) {return LuaSignal(L, 6);}
			static s32 LuaSignal7(lua_State * L) {return LuaSignal(L, 7);}

			static s32 LuaToType(lua_State * L)
			{
				void * data = (void*)lua_topointer(L, 1);
				const c8 * key = luaL_checkstring(L, 2);

				*(void**)lua_newuserdata(L, sizeof(void*)) = data; // TODO probably bad
				luaL_getmetatable(L, key);
				lua_setmetatable(L, -2);

				return 1;
			}

			static s32 LuaSetMaterial(lua_State * L)
			{
				teActorScriptLua * actor = (teActorScriptLua*)lua_topointer(L, 1);
				video::teMaterial * material = *(video::teMaterial**)lua_touserdata(L, 3);

				u32 materialIndex = (u32)actor->scene->GetContentPack().materials.GetIndexInArray(material);

				teAssetSprite * sprite = *(teAssetSprite**)lua_topointer(L, 2);
				if(sprite)
				{
					sprite->renderAsset.materialIndex = materialIndex; // TODO wtf ?
					return 0;
				}

				return 0;
			}

			static s32 LuaGetStage(lua_State * L)
			{
				teActorScriptLua * data = (teActorScriptLua*)lua_topointer(L, 1);
				lua_pushnumber(L, data->scene->GetStage());
				return 1;
			}

			static s32 BindPrimitiveSet(lua_State * L)
			{
				tePrimitiveLinkData * data = *(tePrimitiveLinkData**)luaL_checkudata(L, 1, "tePrimitive");
				const c8 * key = luaL_checkstring(L, 2);

				if(strcmp(key, "data") == 0)
				{
					f32 number = luaL_checknumber(L, 3);
					SetLinkVariable(data->ptr, data->dataType, number);
				}

				return 0;
			}

			static s32 BindPrimitiveGet(lua_State * L)
			{
				tePrimitiveLinkData * data = *(tePrimitiveLinkData**)luaL_checkudata(L, 1, "tePrimitive");
				const c8 * key = luaL_checkstring(L, 2);

				if(strcmp(key, "data") == 0)
				{
					f32 number = 0.0f;
					GetLinkVariable(data->ptr, data->dataType, number);
					lua_pushnumber(L, number);
					return 1;
				}

				return 0;
			}

			static s32 BindAssetMaterialSet(lua_State * L)
			{
				video::teMaterial * data = *(video::teMaterial**)luaL_checkudata(L, 1, "teMaterial");
				const c8 * key = luaL_checkstring(L, 2);

				/*
				if(strcmp(key, "pos_x") == 0) data->position.x = luaL_checknumber(L, 3);
				else if(strcmp(key, "pos_y") == 0) data->position.y = luaL_checknumber(L, 3);
				*/

				return 0;
			}

			static s32 BindAssetMaterialGet(lua_State * L)
			{
				video::teMaterial * data = *(video::teMaterial**)luaL_checkudata(L, 1, "teMaterial");
				const c8 *key = luaL_checkstring(L, 2);

				/*
				if(strcmp(key, "pos_x") == 0) lua_pushnumber(L, data->position.x);
				else if(strcmp(key, "pos_y") == 0) lua_pushnumber(L, data->position.y);
				*/

				return 0;
			}

			static s32 BindAssetAtlasSpriteSet(lua_State * L)
			{
				video::teAtlasSprite * data = *(video::teAtlasSprite**)luaL_checkudata(L, 1, "teAtlasSprite");
				const c8 * key = luaL_checkstring(L, 2);

				/*
				if(strcmp(key, "pos_x") == 0) data->position.x = luaL_checknumber(L, 3);
				else if(strcmp(key, "pos_y") == 0) data->position.y = luaL_checknumber(L, 3);
				*/

				return 0;
			}

			static s32 BindAssetAtlasSpriteGet(lua_State * L)
			{
				video::teAtlasSprite * data = *(video::teAtlasSprite**)luaL_checkudata(L, 1, "teAtlasSprite");
				const c8 *key = luaL_checkstring(L, 2);

				/*
				if(strcmp(key, "pos_x") == 0) lua_pushnumber(L, data->position.x);
				else if(strcmp(key, "pos_y") == 0) lua_pushnumber(L, data->position.y);
				*/

				return 0;
			}

			static s32 BindAssetSkeletonSet(lua_State * L)
			{
				teSkeleton * data = *(teSkeleton**)luaL_checkudata(L, 1, "teSkeleton");
				const c8 * key = luaL_checkstring(L, 2);

				/*
				if(strcmp(key, "pos_x") == 0) data->position.x = luaL_checknumber(L, 3);
				else if(strcmp(key, "pos_y") == 0) data->position.y = luaL_checknumber(L, 3);
				*/

				return 0;
			}

			static s32 BindAssetSkeletonGet(lua_State * L)
			{
				teSkeleton * data = *(teSkeleton**)luaL_checkudata(L, 1, "teSkeleton");
				const c8 *key = luaL_checkstring(L, 2);

				/*
				if(strcmp(key, "pos_x") == 0) lua_pushnumber(L, data->position.x);
				else if(strcmp(key, "pos_y") == 0) lua_pushnumber(L, data->position.y);
				*/

				return 0;
			}

			static s32 BindAssetGeometrySet(lua_State * L)
			{
				video::teSurfaceData * data = *(video::teSurfaceData**)luaL_checkudata(L, 1, "teSurfaceData");
				const c8 * key = luaL_checkstring(L, 2);

				/*
				if(strcmp(key, "pos_x") == 0) data->position.x = luaL_checknumber(L, 3);
				else if(strcmp(key, "pos_y") == 0) data->position.y = luaL_checknumber(L, 3);
				*/

				return 0;
			}

			static s32 BindAssetGeometryGet(lua_State * L)
			{
				video::teSurfaceData * data = *(video::teSurfaceData**)luaL_checkudata(L, 1, "teSurfaceData");
				const c8 *key = luaL_checkstring(L, 2);

				/*
				if(strcmp(key, "pos_x") == 0) lua_pushnumber(L, data->position.x);
				else if(strcmp(key, "pos_y") == 0) lua_pushnumber(L, data->position.y);
				*/

				return 0;
			}

			static s32 BindAssetFontSet(lua_State * L)
			{
				font::teFont * data = *(font::teFont**)luaL_checkudata(L, 1, "teFont");
				const c8 * key = luaL_checkstring(L, 2);

				/*
				if(strcmp(key, "pos_x") == 0) data->position.x = luaL_checknumber(L, 3);
				else if(strcmp(key, "pos_y") == 0) data->position.y = luaL_checknumber(L, 3);
				*/

				return 0;
			}

			static s32 BindAssetFontGet(lua_State * L)
			{
				font::teFont * data = *(font::teFont**)luaL_checkudata(L, 1, "teFont");
				const c8 *key = luaL_checkstring(L, 2);

				/*
				if(strcmp(key, "pos_x") == 0) lua_pushnumber(L, data->position.x);
				else if(strcmp(key, "pos_y") == 0) lua_pushnumber(L, data->position.y);
				*/

				return 0;
			}

			static s32 BindAssetSoundSet(lua_State * L)
			{
				sound::teSound * data = *(sound::teSound**)luaL_checkudata(L, 1, "teSound");
				const c8 * key = luaL_checkstring(L, 2);

				/*
				if(strcmp(key, "pos_x") == 0) data->position.x = luaL_checknumber(L, 3);
				else if(strcmp(key, "pos_y") == 0) data->position.y = luaL_checknumber(L, 3);
				*/

				return 0;
			}

			static s32 BindAssetSoundGet(lua_State * L)
			{
				sound::teSound * data = *(sound::teSound**)luaL_checkudata(L, 1, "teSound");
				const c8 *key = luaL_checkstring(L, 2);

				/*
				if(strcmp(key, "pos_x") == 0) lua_pushnumber(L, data->position.x);
				else if(strcmp(key, "pos_y") == 0) lua_pushnumber(L, data->position.y);
				*/

				return 0;
			}

			static s32 BindAssetStringSet(lua_State * L)
			{
				teStringInfo * data = *(teStringInfo**)luaL_checkudata(L, 1, "teStringInfo");
				const c8 * key = luaL_checkstring(L, 2);

				c8 * raw = teContentPack::GetStringRaw(data);
				u32 size = teContentPack::GetStringSize(data);

				if(strcmp(key, "data") == 0)
				{
					const c8 * str = luaL_checkstring(L, 3);

					s32 l = sprintf(raw, "%s", str);
					TE_ASSERT_NODEBUG((l + 1) < (s32)size);
				}

				return 0;
			}

			static s32 BindAssetStringGet(lua_State * L)
			{
				teStringInfo * data = *(teStringInfo**)luaL_checkudata(L, 1, "teStringInfo");
				const c8 *key = luaL_checkstring(L, 2);

				c8 * raw = teContentPack::GetStringRaw(data);
				u32 size = teContentPack::GetStringSize(data);

				if(strcmp(key, "data") == 0) lua_pushstring(L, raw);
				else if(strcmp(key, "size") == 0) lua_pushinteger(L, (s32)size);
				else return 0;

				return 1;
			}

			static s32 BindAssetTransformSet(lua_State * L)
			{
				teAssetTransform * data = *(teAssetTransform**)luaL_checkudata(L, 1, "teAssetTransform");
				const c8 * key = luaL_checkstring(L, 2);

				if(strcmp(key, "pos_x") == 0) data->position.x = luaL_checknumber(L, 3);
				else if(strcmp(key, "pos_y") == 0) data->position.y = luaL_checknumber(L, 3);
				else if(strcmp(key, "pos_z") == 0) data->position.z = luaL_checknumber(L, 3);
				else if(strcmp(key, "rot_x") == 0) data->rotation.x = luaL_checknumber(L, 3);
				else if(strcmp(key, "rot_y") == 0) data->rotation.y = luaL_checknumber(L, 3);
				else if(strcmp(key, "rot_z") == 0) data->rotation.z = luaL_checknumber(L, 3);
				else if(strcmp(key, "rot_w") == 0) data->rotation.w = luaL_checknumber(L, 3);
				else if(strcmp(key, "scl_x") == 0) data->scale.x = luaL_checknumber(L, 3);
				else if(strcmp(key, "scl_y") == 0) data->scale.y = luaL_checknumber(L, 3);
				else if(strcmp(key, "scl_z") == 0) data->scale.z = luaL_checknumber(L, 3);
				else if(strcmp(key, "visible") == 0) data->visible = luaL_checkint(L, 3) ? true : false;

				return 0;
			}

			static s32 BindAssetTransformGet(lua_State * L)
			{
				teAssetTransform * data = *(teAssetTransform**)luaL_checkudata(L, 1, "teAssetTransform");
				const c8 *key = luaL_checkstring(L, 2);

				if(strcmp(key, "pos_x") == 0) lua_pushnumber(L, data->position.x);
				else if(strcmp(key, "pos_y") == 0) lua_pushnumber(L, data->position.y);
				else if(strcmp(key, "pos_z") == 0) lua_pushnumber(L, data->position.z);
				else if(strcmp(key, "rot_x") == 0) lua_pushnumber(L, data->rotation.x);
				else if(strcmp(key, "rot_y") == 0) lua_pushnumber(L, data->rotation.y);
				else if(strcmp(key, "rot_z") == 0) lua_pushnumber(L, data->rotation.z);
				else if(strcmp(key, "rot_w") == 0) lua_pushnumber(L, data->rotation.w);
				else if(strcmp(key, "scl_x") == 0) lua_pushnumber(L, data->scale.x);
				else if(strcmp(key, "scl_y") == 0) lua_pushnumber(L, data->scale.y);
				else if(strcmp(key, "scl_z") == 0) lua_pushnumber(L, data->scale.z);
				else if(strcmp(key, "parent") == 0) lua_pushinteger(L, data->parent);
				else if(strcmp(key, "visible") == 0) lua_pushinteger(L, data->visible);
				else if(strcmp(key, "in_frame") == 0) lua_pushinteger(L, data->inFrame);
				else return 0;

				return 1;
			}

			static s32 BindAssetCameraSet(lua_State * L)
			{
				teAssetCamera * data = *(teAssetCamera**)luaL_checkudata(L, 1, "teAssetCamera");
				const c8 * key = luaL_checkstring(L, 2);

				/*
				if(strcmp(key, "pos_x") == 0) data->position.x = luaL_checknumber(L, 3);
				else if(strcmp(key, "pos_y") == 0) data->position.y = luaL_checknumber(L, 3);
				*/

				return 0;
			}

			static s32 BindAssetCameraGet(lua_State * L)
			{
				teAssetCamera * data = *(teAssetCamera**)luaL_checkudata(L, 1, "teAssetCamera");
				const c8 *key = luaL_checkstring(L, 2);

				/*
				if(strcmp(key, "pos_x") == 0) lua_pushnumber(L, data->position.x);
				else if(strcmp(key, "pos_y") == 0) lua_pushnumber(L, data->position.y);
				*/

				return 0;
			}

			static s32 BindAssetSpriteSet(lua_State * L)
			{
				teAssetSprite * data = *(teAssetSprite**)luaL_checkudata(L, 1, "teAssetSprite");
				const c8 * key = luaL_checkstring(L, 2);

				if(strcmp(key, "color_a") == 0) data->color.a = (u8)luaL_checkint(L, 3);
				else if(strcmp(key, "color_r") == 0) data->color.r = (u8)luaL_checkint(L, 3);
				else if(strcmp(key, "color_g") == 0) data->color.g = (u8)luaL_checkint(L, 3);
				else if(strcmp(key, "color_b") == 0) data->color.b = (u8)luaL_checkint(L, 3);

				return 0;
			}

			static s32 BindAssetSpriteGet(lua_State * L)
			{
				teAssetSprite * data = *(teAssetSprite**)luaL_checkudata(L, 1, "teAssetSprite");
				const c8 *key = luaL_checkstring(L, 2);

				if(strcmp(key, "color_a") == 0) lua_pushinteger(L, data->color.a);
				else if(strcmp(key, "color_r") == 0) lua_pushinteger(L, data->color.r);
				else if(strcmp(key, "color_g") == 0) lua_pushinteger(L, data->color.g);
				else if(strcmp(key, "color_b") == 0) lua_pushinteger(L, data->color.b);
				else return 0;

				return 1;
			}

			static s32 BindAssetSurfaceSet(lua_State * L)
			{
				teAssetSurface * data = *(teAssetSurface**)luaL_checkudata(L, 1, "teAssetSurface");
				const c8 * key = luaL_checkstring(L, 2);

				/*
				if(strcmp(key, "pos_x") == 0) data->position.x = luaL_checknumber(L, 3);
				else if(strcmp(key, "pos_y") == 0) data->position.y = luaL_checknumber(L, 3);
				*/

				return 0;
			}

			static s32 BindAssetSurfaceGet(lua_State * L)
			{
				teAssetSurface * data = *(teAssetSurface**)luaL_checkudata(L, 1, "teAssetSurface");
				const c8 *key = luaL_checkstring(L, 2);

				/*
				if(strcmp(key, "pos_x") == 0) lua_pushnumber(L, data->position.x);
				else if(strcmp(key, "pos_y") == 0) lua_pushnumber(L, data->position.y);
				*/

				return 0;
			}

			static s32 BindAssetTextSet(lua_State * L)
			{
				teAssetText * data = *(teAssetText**)luaL_checkudata(L, 1, "teAssetText");
				const c8 * key = luaL_checkstring(L, 2);

				/*
				if(strcmp(key, "pos_x") == 0) data->position.x = luaL_checknumber(L, 3);
				else if(strcmp(key, "pos_y") == 0) data->position.y = luaL_checknumber(L, 3);
				*/

				return 0;
			}

			static s32 BindAssetTextGet(lua_State * L)
			{
				teAssetText * data = *(teAssetText**)luaL_checkudata(L, 1, "teAssetText");
				const c8 *key = luaL_checkstring(L, 2);

				/*
				if(strcmp(key, "pos_x") == 0) lua_pushnumber(L, data->position.x);
				else if(strcmp(key, "pos_y") == 0) lua_pushnumber(L, data->position.y);
				*/

				return 0;
			}

			static s32 BindAssetDrawActorSet(lua_State * L)
			{
				teAssetDrawActor * data = *(teAssetDrawActor**)luaL_checkudata(L, 1, "teAssetDrawActor");
				const c8 * key = luaL_checkstring(L, 2);

				/*
				if(strcmp(key, "pos_x") == 0) data->position.x = luaL_checknumber(L, 3);
				else if(strcmp(key, "pos_y") == 0) data->position.y = luaL_checknumber(L, 3);
				*/

				return 0;
			}

			static s32 BindAssetDrawActorGet(lua_State * L)
			{
				teAssetDrawActor * data = *(teAssetDrawActor**)luaL_checkudata(L, 1, "teAssetDrawActor");
				const c8 *key = luaL_checkstring(L, 2);

				/*
				if(strcmp(key, "pos_x") == 0) lua_pushnumber(L, data->position.x);
				else if(strcmp(key, "pos_y") == 0) lua_pushnumber(L, data->position.y);
				*/

				return 0;
			}

			static s32 BindAssetVariableSet(lua_State * L)
			{
				teAssetVariable * data = *(teAssetVariable**)luaL_checkudata(L, 1, "teAssetVariable");
				const c8 * key = luaL_checkstring(L, 2);

				if(strcmp(key, "s32") == 0) data->vs32 = luaL_checkint(L, 3);
				else if(strcmp(key, "f32") == 0) data->vf32 = luaL_checknumber(L, 3);
				else if(strcmp(key, "type") == 0) data->type = (u8)luaL_checkint(L, 3);

				return 0;
			}

			static s32 BindAssetVariableGet(lua_State * L)
			{
				teAssetVariable * data = *(teAssetVariable**)luaL_checkudata(L, 1, "teAssetVariable");
				const c8 *key = luaL_checkstring(L, 2);

				if(strcmp(key, "s32") == 0) lua_pushinteger(L, data->GetS32());
				else if(strcmp(key, "f32") == 0) lua_pushnumber(L, data->GetF32());
				else if(strcmp(key, "type") == 0) lua_pushnumber(L, data->type);
				else return 0;

				return 1;
			}

			static s32 BindAssetArraySet(lua_State * L)
			{
				teAssetArray * data = *(teAssetArray**)luaL_checkudata(L, 1, "teAssetArray");
				const c8 * key = luaL_checkstring(L, 2);

				return 0;
			}

			static s32 BindAssetArrayGet(lua_State * L)
			{
				teAssetArray * data = *(teAssetArray**)luaL_checkudata(L, 1, "teAssetArray");
				const c8 * key = luaL_checkstring(L, 2);

				if(strcmp(key, "count") == 0)
				{
					lua_pushinteger(L, (s32)data->count);
					return 1;
				}
				else if(key)
				{
					s32 index = atoi(key);
					lua_pushlightuserdata(L, reinterpret_cast<void**>(data->data)[index]);
					return 1;
				}

				return 0;
			}

			static void SetupEngineBinding()
			{
				lua_register(L, "signal0", &LuaSignal0);
				lua_register(L, "signal1", &LuaSignal1);
				lua_register(L, "signal2", &LuaSignal2);
				lua_register(L, "signal3", &LuaSignal3);
				lua_register(L, "signal4", &LuaSignal4);
				lua_register(L, "signal5", &LuaSignal5);
				lua_register(L, "signal6", &LuaSignal6);
				lua_register(L, "signal7", &LuaSignal7);

				lua_register(L, "toType", &LuaToType);
				lua_register(L, "setMaterial", &LuaSetMaterial);
				lua_register(L, "getStage", &LuaGetStage);

				#define TE_LUA_REG(__name, __set, __get) \
				{ \
					const struct luaL_Reg __name[] = {{"__index", __get}, {"__newindex", __set}, {NULL, NULL}}; \
					luaL_newmetatable(L, #__name); \
					luaL_setfuncs(L, __name, 0); \
				}

				TE_LUA_REG(tePrimitive,			BindPrimitiveSet,			BindPrimitiveGet);
				TE_LUA_REG(teMaterial,			BindAssetMaterialSet,		BindAssetMaterialGet);
				TE_LUA_REG(teAtlasSprite,		BindAssetAtlasSpriteSet,	BindAssetAtlasSpriteGet);
				TE_LUA_REG(teSkeleton,			BindAssetSkeletonSet,		BindAssetSkeletonGet);
				TE_LUA_REG(teSurfaceData,		BindAssetGeometrySet,		BindAssetGeometryGet);
				TE_LUA_REG(teFont,				BindAssetFontSet,			BindAssetFontGet);
				TE_LUA_REG(teSound,				BindAssetSoundSet,			BindAssetSoundGet);
				TE_LUA_REG(teStringInfo,		BindAssetStringSet,			BindAssetStringGet);
				TE_LUA_REG(teAssetTransform,	BindAssetTransformSet,		BindAssetTransformGet);
				TE_LUA_REG(teAssetCamera,		BindAssetCameraSet,			BindAssetCameraGet);
				TE_LUA_REG(teAssetSprite,		BindAssetSpriteSet,			BindAssetSpriteGet);
				TE_LUA_REG(teAssetSurface,		BindAssetSurfaceSet,		BindAssetSurfaceGet);
				TE_LUA_REG(teAssetText,			BindAssetTextSet,			BindAssetTextGet);
				TE_LUA_REG(teAssetDrawActor,	BindAssetDrawActorSet,		BindAssetDrawActorGet);
				TE_LUA_REG(teAssetVariable,		BindAssetVariableSet,		BindAssetVariableGet);
				TE_LUA_REG(teAssetArray,		BindAssetArraySet,			BindAssetArrayGet);
			}

			TE_INLINE void BindLink(const teString & name, void * link, u8 index)
			{
				if(!link)
					return;

				ELinkDataType dataType = scene->GetAssetPack().GetLinkDataType(link);

				if(dataType == LDT_UNKNOWN)
				{
					ELinkData data = scene->GetAssetPack().GetLinkData(link);

					lua_getglobal(L, metaName);
					*(void**)lua_newuserdata(L, sizeof(void*)) = link;

					switch(data)
					{
					case LD_MATERIAL:			luaL_getmetatable(L, "teMaterial"); lua_setmetatable(L, -2); break;
					case LD_ATLAS_SPRITE:		luaL_getmetatable(L, "teAtlasSprite"); lua_setmetatable(L, -2); break;
					case LD_SKELETON:			luaL_getmetatable(L, "teSkeleton"); lua_setmetatable(L, -2); break;
					case LD_GEOMETRY:			luaL_getmetatable(L, "teSurfaceData"); lua_setmetatable(L, -2); break;
					case LD_VERTEX_LAYERS: break;
					case LD_FONT:				luaL_getmetatable(L, "teFont"); lua_setmetatable(L, -2); break;
					case LD_SOUND:				luaL_getmetatable(L, "teSound"); lua_setmetatable(L, -2); break;
					case LD_STRING:				luaL_getmetatable(L, "teStringInfo"); lua_setmetatable(L, -2); break;
					case LD_TRANSFORM:			luaL_getmetatable(L, "teAssetTransform"); lua_setmetatable(L, -2); break;
					case LD_CAMERA:				luaL_getmetatable(L, "teAssetCamera"); lua_setmetatable(L, -2); break;
					case LD_SPRITE:				luaL_getmetatable(L, "teAssetSprite"); lua_setmetatable(L, -2); break;
					case LD_SURFACE:			luaL_getmetatable(L, "teAssetSurface"); lua_setmetatable(L, -2); break;
					case LD_TEXT:				luaL_getmetatable(L, "teAssetText"); lua_setmetatable(L, -2); break;
					case LD_DRAW_ACTOR:			luaL_getmetatable(L, "teAssetDrawActor"); lua_setmetatable(L, -2); break;
					case LD_VARIABLE:			luaL_getmetatable(L, "teAssetVariable"); lua_setmetatable(L, -2); break;
					case LD_ARRAY:				luaL_getmetatable(L, "teAssetArray"); lua_setmetatable(L, -2); break;
					case LD_ACTOR: break;
					default: break;
					}

					lua_setfield(L, -2, name.c_str());
				}
				else
				{
					primitiveLinks[index].dataType = dataType;
					primitiveLinks[index].ptr = link;

					lua_getglobal(L, metaName);
					*(void**)lua_newuserdata(L, sizeof(void*)) = link;
					luaL_getmetatable(L, "tePrimitive");
					lua_setmetatable(L, -2);
					lua_setfield(L, -2, name.c_str());
				}
			}

			// ---------------------------------------------------------------------------------------- State

			static lua_State * L;
			static u32 stateUsageCount;

			static TE_INLINE void CheckError(s32 state)
			{
				if(state == 0)
					return;

				TE_LOG_ERR("lua : '%s'", lua_tostring(L, -1));
				lua_pop(L, 1);
			}

			static void InitLua()
			{
				if(L == NULL)
				{
					L = lua_newstate(&teLuaAlloc, NULL);
					luaL_openlibs(L);

					SetupEngineBinding();

					stateUsageCount = 0;
				}

				++stateUsageCount;
			}

			static void CloseLua()
			{
				if(stateUsageCount)
				{
					--stateUsageCount;

					if(!stateUsageCount)
					{
						lua_close(L);
						L = NULL;
					}
				}
			}
		};

		TE_ACTOR_PROXY(teActorScriptLua);
		TE_ACTOR_SLOT_4(teActorScriptLua, Slot0);
		TE_ACTOR_SLOT_4(teActorScriptLua, Slot1);
		TE_ACTOR_SLOT_4(teActorScriptLua, Slot2);
		TE_ACTOR_SLOT_4(teActorScriptLua, Slot3);
		TE_ACTOR_SLOT_4(teActorScriptLua, Slot4);
		TE_ACTOR_SLOT_4(teActorScriptLua, Slot5);
		TE_ACTOR_SLOT_4(teActorScriptLua, Slot6);
		TE_ACTOR_SLOT_4(teActorScriptLua, Slot7);

		TE_FUNC void RegisterScriptLua(teActorsTI * ti)
		{
			ti->AddClass(sizeof(teActorScriptLua), "scriptLua", TE_ACTOR_PROXY_NAMES(teActorScriptLua));
			ti->AddLink("script");
			ti->AddLink("link0");
			ti->AddLink("link1");
			ti->AddLink("link2");
			ti->AddLink("link3");
			ti->AddLink("link4");
			ti->AddLink("link5");
			ti->AddLink("link6");
			ti->AddLink("link7");
			ti->AddSignal("Signal0");
			ti->AddSignal("Signal1");
			ti->AddSignal("Signal2");
			ti->AddSignal("Signal3");
			ti->AddSignal("Signal4");
			ti->AddSignal("Signal5");
			ti->AddSignal("Signal6");
			ti->AddSignal("Signal7");
			ti->AddSlot("Slot0", TE_ACTOR_SLOT_PROXY(teActorScriptLua, Slot0));
			ti->AddSlot("Slot1", TE_ACTOR_SLOT_PROXY(teActorScriptLua, Slot1));
			ti->AddSlot("Slot2", TE_ACTOR_SLOT_PROXY(teActorScriptLua, Slot2));
			ti->AddSlot("Slot3", TE_ACTOR_SLOT_PROXY(teActorScriptLua, Slot3));
			ti->AddSlot("Slot4", TE_ACTOR_SLOT_PROXY(teActorScriptLua, Slot4));
			ti->AddSlot("Slot5", TE_ACTOR_SLOT_PROXY(teActorScriptLua, Slot5));
			ti->AddSlot("Slot6", TE_ACTOR_SLOT_PROXY(teActorScriptLua, Slot6));
			ti->AddSlot("Slot7", TE_ACTOR_SLOT_PROXY(teActorScriptLua, Slot7));
		}
	}
}

#endif