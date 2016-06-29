#include "voi.h"
#include <lua.hpp>
#include <vector>

// http://www.lua.org/manual/5.2/manual.html

namespace {
	lua_State* l;
}

static void InitVecLib();
static void InitQuatLib();
static void InitRandLib();
static void InitEffectLib();
static void InitDebugLib();
static void InitEntityLib();

bool InitScripting() {
	l = luaL_newstate();
	luaL_openlibs(l);

	InitVecLib();
	InitRandLib();
	InitEffectLib();
	InitDebugLib();
	InitEntityLib();

	return true;
}

s32 LoadScript(const char* fname) {
	if(luaL_loadfile(l, fname)) {
		fprintf(stderr, "Error loading script '%s': %s\n", fname, luaL_checkstring(l, -1));
		return 0;
	}
	lua_newtable(l); // env
		lua_newtable(l); // mt
		lua_pushglobaltable(l);
		lua_setfield(l, -2, "__index");
		lua_setmetatable(l, -2);
	lua_pushvalue(l, -1); // dup env
	lua_insert(l, -3); // insert before chunk
	lua_setupvalue(l, -2, 1); // chunk._ENV = {__mt = {__index = _G}}

	lua_pushvalue(l, -1);
	if(lua_pcall(l, 0, 0, 0)) {
		fprintf(stderr, "Error running script '%s': %s\n", fname, luaL_checkstring(l, -1));
		lua_pop(l, 2);
		return 0;
	}

	return luaL_ref(l, LUA_REGISTRYINDEX);
}

void UnloadScript(s32 s) {
	if(s) luaL_unref(l, LUA_REGISTRYINDEX, s);
}

s32 GetCallbackFromScript(s32 script, const char* funcName) {
	if(!script) {
		fprintf(stderr, "Warning! Tried to get a function '%s' from invalid script\n", funcName);
		return 0;
	}

	lua_rawgeti(l, LUA_REGISTRYINDEX, script);
	if(lua_isnil(l, -1)) {
		fprintf(stderr, "Warning! Tried to get a function '%s' from invalid script (%d)\n", funcName, script);
		lua_pop(l, 1);
		return 0;
	}

	lua_getupvalue(l, -1, 1); // Get ENV
	lua_getfield(l, -1, funcName);
	if(!lua_isfunction(l, -1)) {
		fprintf(stderr, "Warning! Tried to get non-existant function '%s' from script (%d)\n", funcName, script);
		lua_pop(l, 3);
		return 0;
	}

	s32 func = luaL_ref(l, LUA_REGISTRYINDEX);
	lua_pop(l, 2); // pop ENV and script

	return func;
}

void RunCallback(u32 entId, s32 func) {
	if(!func) {
		fprintf(stderr, "Warning! Tried to run invalid script callback\n");
		return;
	}

	lua_rawgeti(l, LUA_REGISTRYINDEX, func);
	if(!lua_isfunction(l, -1)) {
		fprintf(stderr, "Warning! Tried to run a non-function object (%d) as a callback (type: %s)\n", 
			func, lua_typename(l, lua_type(l, -1)));
		lua_pop(l, 1);
		return;
	}

	lua_pushinteger(l, entId);
	if(lua_pcall(l, 1, 0, 0)) {
		fprintf(stderr, "Error running script callback: %s\n", luaL_checkstring(l, -1));
		lua_pop(l, 1);
	}
}

////////////////////////////////////////////////////////////////////////////////////
// Library /////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void stackdump(){
	int i;
	int top = lua_gettop(l);
	printf("[LuaStack] ");
	for (i = 1; i <= top; i++) {  /* repeat for each level */
		int t = lua_type(l, i);
		switch (t) {

			case LUA_TSTRING:  /* strings */
			printf("'%s'", lua_tostring(l, i));
			break;

			case LUA_TBOOLEAN:  /* booleans */
			printf(lua_toboolean(l, i) ? "true" : "false");
			break;

			case LUA_TNUMBER:  /* numbers */
			printf("%g", lua_tonumber(l, i));
			break;

			default:  /* other values */
			printf("%s", lua_typename(l, t));
			break;

		}
		printf("  ");  /* put a separator */
	}
	printf("\n");  /* end the listing */
}

static vec3* lCheckVecRef(s32 s) {
	return (vec3*)luaL_checkudata(l, s, "vecmt");
}
static vec3* lTestVecRef(s32 s) {
	return (vec3*)luaL_testudata(l, s, "vecmt");
}

static vec3* lNewVecUD() {
	auto ud = (vec3*) lua_newuserdata(l, sizeof(vec3));
	luaL_setmetatable(l, "vecmt");
	return ud;	
}

static void InitVecLib() {
	static const luaL_Reg vecmt[] = {
		{"__tostring", [](lua_State* l){
			auto v = lCheckVecRef(1);
			lua_pushfstring(l, "vec(%f, %f, %f)", v->x, v->y, v->z);
			return 1;
		}},

		#define OP(name, op) \
		{"__" #name, [](lua_State* l){ \
			auto v = lCheckVecRef(1); \
			auto ud = lNewVecUD(); \
			if(auto v2 = lTestVecRef(2)) { \
				*ud = *v op *v2; \
			}else if(lua_isnumber(l, 2)) { \
				*ud = *v op f32(lua_tonumber(l, 2)); \
			}else{ \
				return 0; \
			} \
			return 1; \
		}}
		OP(add, +),
		OP(sub, -),
		OP(mul, *),
		OP(div, /),
		#undef OP

		{"__unm", [](lua_State*){
			*lNewVecUD() = -*lCheckVecRef(1);
			return 1;
		}},

		{nullptr, nullptr}
	};

	static const luaL_Reg veclib[] = {
		{"normalize", [](lua_State*) {
			*lNewVecUD() = glm::normalize(*lCheckVecRef(1));
			return 1;
		}},
		{"dot", [](lua_State* l) {
			lua_pushnumber(l, glm::dot(*lCheckVecRef(1), *lCheckVecRef(2)));
			return 1;
		}},
		{"cross", [](lua_State*) {
			*lNewVecUD() = glm::cross(*lCheckVecRef(1), *lCheckVecRef(2));
			return 1;
		}},
		{"length", [](lua_State*) {
			lua_pushnumber(l, glm::length(*lCheckVecRef(1)));
			return 1;
		}},

		#define ACCESSOR(x) \
		{#x, [](lua_State*){ \
			s32 args = lua_gettop(l); \
			auto v = lCheckVecRef(1); \
			if(args == 1) { \
				lua_pushnumber(l, v->x); \
			}else if(args == 2) { \
				v->x = luaL_checknumber(l, 2); \
				lua_pushvalue(l, 1); \
			} \
			return 1; \
		}}

		ACCESSOR(x),
		ACCESSOR(y),
		ACCESSOR(z),
		#undef ACCESSOR

		{nullptr, nullptr}
	};

	luaL_newmetatable(l, "vecmt");
	luaL_setfuncs(l, vecmt, 0);

	lua_newtable(l); // veclib
	luaL_setfuncs(l, veclib, 0);

	lua_newtable(l); // mt
	lua_pushcfunction(l, [](lua_State* l) {
		s32 numArgs = lua_gettop(l)-1; // First arg is the veclib
		auto ud = lNewVecUD();

		if(numArgs == 3) {
			ud->x = luaL_checknumber(l, 2);
			ud->y = luaL_checknumber(l, 3);
			ud->z = luaL_checknumber(l, 4);
		}else if(numArgs == 2) {
			ud->x = luaL_checknumber(l, 2);
			ud->y = luaL_checknumber(l, 3);
			ud->z = 0.f;
		}else if(numArgs == 1) {
			if(auto o = lTestVecRef(2)) {
				*ud = *o;
			}else if(lua_isnumber(l, 2)){
				ud->x = ud->y = ud->z = lua_tonumber(l, 2);
			}else{
				*ud = vec3{0.f};
			}
		}else{
			*ud = vec3{0.f};
		}
		
		return 1;
	});
	lua_setfield(l, -2, "__call");
	lua_setmetatable(l, -2);

	lua_pushvalue(l, -1);
	lua_setglobal(l, "vec");
	lua_setfield(l, -2, "__index");
}

static void InitRandLib() {
	static const luaL_Reg randlib[] = {
		{"ball", [](lua_State* l) {
			*lNewVecUD() = glm::ballRand(luaL_checknumber(l, 1));
			return 1;
		}},
		{"gauss", [](lua_State* l) {
			if(auto mean = lTestVecRef(1)) {
				*lNewVecUD() = glm::gaussRand(*mean, *lCheckVecRef(2));
			}else if(lua_isnumber(l, 1)) {
				lua_pushnumber(l, glm::gaussRand(lua_tonumber(l, 1), luaL_checknumber(l, 2)));
			}else return 0;
			return 1;
		}},
		{"linear", [](lua_State* l) {
			if(auto min = lTestVecRef(1)) {
				*lNewVecUD() = glm::linearRand(*min, *lCheckVecRef(2));
			}else if(lua_isnumber(l, 1)) {
				lua_pushnumber(l, glm::linearRand(lua_tonumber(l, 1), luaL_checknumber(l, 2)));
			}else return 0;
			return 1;
		}},
		{"spherical", [](lua_State* l) {
			*lNewVecUD() = glm::sphericalRand(luaL_checknumber(l, 1));
			return 1;
		}},

		{nullptr, nullptr}
	};

	lua_newtable(l); // randlib
	luaL_setfuncs(l, randlib, 0);

	lua_setglobal(l, "rand");
}

static void InitEffectLib() {
	static const luaL_Reg fxlib[] = {
		{"fog", [](lua_State* l) {
			auto col = lCheckVecRef(1);
			f32 dist = luaL_checknumber(l, 2);
			f32 dens = luaL_checknumber(l, 3);
			f32 dura = luaL_optnumber(l, 4, 4.f);
			SetTargetFogParameters(*col, dist, dens, dura);
			return 0;
		}},

		{"fog_color", [](lua_State* l) {
			SetTargetFogColor(*lCheckVecRef(1), luaL_optnumber(l, 2, 4.f));
			return 0;
		}},

		{"fog_density", [](lua_State* l) {
			SetTargetFogDensity(luaL_checknumber(l, 1), luaL_optnumber(l, 2, 4.f));
			return 0;
		}},

		{"fog_distance", [](lua_State* l) {
			SetTargetFogDistance(luaL_checknumber(l, 1), luaL_optnumber(l, 2, 4.f));
			return 0;
		}},

		{"vignette", [](lua_State* l) {
			SetTargetVignetteLevel(luaL_checknumber(l, 1), luaL_optnumber(l, 2, 4.f));
			return 0;
		}},

		{nullptr, nullptr}
	};

	lua_newtable(l); // fxlib
	luaL_setfuncs(l, fxlib, 0);

	lua_setglobal(l, "effects");
}

static void InitDebugLib() {
	static const luaL_Reg lib[] = {
		{"point", [](lua_State*) {
			auto p = lCheckVecRef(1);
			if(auto col = lTestVecRef(2)) {
				DebugPoint(*p, *col);
			}else{
				DebugPoint(*p);
			}

			return 0;
		}},

		{"line", [](lua_State*){
			auto p1 = lCheckVecRef(1);
			auto p2 = lCheckVecRef(2);
			auto c1 = lTestVecRef(3);
			auto c2 = lTestVecRef(4);

			if(c1 && c2) {
				DebugLine(*p1, *p2, *c1, *c2);
			}else if(c1) {
				DebugLine(*p1, *p2, *c1);
			}else{
				DebugLine(*p1, *p2);
			}

			return 0;
		}},

		{nullptr, nullptr}
	};

	lua_newtable(l); // lib
	luaL_setfuncs(l, lib, 0);

	lua_setglobal(l, "debug");
}

static Entity* lCheckEnt(s32 s) {
	return *(Entity**) luaL_checkudata(l, s, "entmt");
}
static Entity* lTestEnt(s32 s) {
	auto e = (Entity**) luaL_testudata(l, s, "entmt");
	return (e)?*e:nullptr;
}

static Entity** lNewEntUD(Entity* e) {
	auto ud = (Entity**) lua_newuserdata(l, sizeof(Entity*));
	luaL_setmetatable(l, "entmt");
	*ud = e;
	return ud;	
}

static void InitEntityLib() {
	static const luaL_Reg mt[] = {
		{"__tostring", [](lua_State* l){
			auto e = lCheckEnt(1);
			luaL_Buffer b;
			luaL_buffinit(l, &b);
			auto buf = luaL_prepbuffsize(&b, e->nameLength+sizeof("entity()")-1);
			size_t size = std::sprintf(buf, "entity(%.*s)", e->nameLength, e->name);
			luaL_pushresultsize(&b, size);
			return 1;
		}},
	};

	static const luaL_Reg lib[] = {
		{"lookup", [](lua_State* l) {
			u32 id = luaL_checkinteger(l, 1);
			if(id > 65535) return 0;

			auto ent = GetEntity(id);
			if(ent) {
				lNewEntUD(ent);
			}else{
				lua_pushnil(l);
			}

			return 1;
		}},

		{"name", [](lua_State* l) {
			auto e = lCheckEnt(1);
			if(e) lua_pushlstring(l, e->name, e->nameLength);
			return e?1:0;
		}},

		{"id", [](lua_State* l) {
			auto e = lCheckEnt(1);
			if(e) lua_pushinteger(l, e->id);
			return e?1:0;
		}},

		{"type", [](lua_State* l) {
			auto e = lCheckEnt(1);
			if(e) lua_pushinteger(l, e->entityType);
			return e?1:0;
		}},
		{"type_name", [](lua_State* l) {
			auto e = lCheckEnt(1);
			if(e) lua_pushstring(l, GetEntityTypeName(e->entityType));
			return e?1:0;
		}},

		{"pos", [](lua_State*) {
			auto e = lCheckEnt(1);
			if(e) *lNewVecUD() = e->position;
			return e?1:0;
		}},

		{nullptr, nullptr}
	};

	luaL_newmetatable(l, "entmt");
	luaL_setfuncs(l, mt, 0);

	lua_newtable(l);
	luaL_setfuncs(l, lib, 0);

	lua_pushvalue(l, -1);
	lua_setglobal(l, "entity");
	lua_setfield(l, -2, "__index");
	lua_pop(l, 1);
}