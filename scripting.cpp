#include "voi.h"
#include "lua-synth/synth.h"
#include <lua.hpp>
#include <vector>

// http://www.lua.org/manual/5.2/manual.html

struct CoroutineState {
	enum {
		StateDead,
		StateYielded,
		StateWaiting,
	};

	lua_State* th;
	s32 thidx;
	s32 state;

	union {
		f32 timeout;
	};
};

namespace {
	lua_State* l;
	s32 numCallbackParameters;

	std::vector<CoroutineState> coroutineStates;
}

#ifdef __GNUC__
#define LUALAMBDA [](__attribute__((unused)) lua_State* l) -> s32
#else
#define LUALAMBDA [](lua_State* l) -> s32
#endif

static void InitCoLib();
static void InitVecLib();
static void InitQuatLib();
static void InitRandLib();
static void InitEffectLib();
static void InitDebugLib();
static void InitEntityLib();

static void InitSynthExtensionLib();

bool InitScripting() {
	l = luaL_newstate();
	luaL_openlibs(l);

	InitCoLib();
	InitVecLib();
	InitRandLib();
	InitEffectLib();
	InitDebugLib();
	InitEntityLib();

	if(!synth::InitLuaLib(l))
		return false;

	InitSynthExtensionLib();

	return true;
}

s32 LoadScript(const char* fname) {
	if(luaL_loadfile(l, fname)) {
		LogError("Error loading script '%s': %s\n", fname, luaL_checkstring(l, -1));
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
		LogError("Error running script '%s': %s\n", fname, luaL_checkstring(l, -1));
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
		LogError("Warning! Tried to get a function '%s' from invalid script\n", funcName);
		return 0;
	}

	lua_rawgeti(l, LUA_REGISTRYINDEX, script);
	if(lua_isnil(l, -1)) {
		LogError("Warning! Tried to get a function '%s' from invalid script (%d)\n", funcName, script);
		lua_pop(l, 1);
		return 0;
	}

	lua_getupvalue(l, -1, 1); // Get ENV
	lua_getfield(l, -1, funcName);
	if(!lua_isfunction(l, -1)) {
		LogError("Warning! Tried to get non-existant function '%s' from script (%d)\n", funcName, script);
		lua_pop(l, 3);
		return 0;
	}

	s32 func = luaL_ref(l, LUA_REGISTRYINDEX);
	lua_pop(l, 2); // pop ENV and script

	return func;
}

void PushCallbackParameter(std::nullptr_t) {
	lua_pushnil(l);
	numCallbackParameters++;
}

void PushCallbackParameter(u32 v) {
	lua_pushunsigned(l, v);
	numCallbackParameters++;
}

void PushCallbackParameter(s32 v) {
	lua_pushinteger(l, v);
	numCallbackParameters++;
}

void PushCallbackParameter(f32 v) {
	lua_pushnumber(l, v);
	numCallbackParameters++;
}

void PushCallbackParameter(bool v) {
	lua_pushboolean(l, v);
	numCallbackParameters++;
}

void PushCallbackParameter(const char* v) {
	lua_pushstring(l, v);
	numCallbackParameters++;
}

void RunCallback(s32 func) {
	if(!func) {
		LogError("Warning! Tried to run invalid script callback\n");
		return;
	}

	// Get function from registry
	lua_rawgeti(l, LUA_REGISTRYINDEX, func);
	if(!lua_isfunction(l, -1)) {
		LogError("Warning! Tried to run a non-function object (%d) as a callback (type: %s)\n", 
			func, lua_typename(l, lua_type(l, -1)));
		lua_pop(l, 1);
		return;
	}

	// Move function underneath arguments on stack
	lua_insert(l, -numCallbackParameters-1);

	if(lua_pcall(l, numCallbackParameters, 0, 0)) {
		LogError("Error running script callback: %s\n", luaL_checkstring(l, -1));
		lua_pop(l, 1);
	}

	numCallbackParameters = 0;
}

////////////////////////////////////////////////////////////////////////////////////
// Library /////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void stackdump(lua_State* l){
	int i;
	int top = lua_gettop(l);
	LogError("[LuaStack] ");
	for (i = 1; i <= top; i++) {  /* repeat for each level */
		int t = lua_type(l, i);
		switch (t) {

			case LUA_TSTRING:  /* strings */
			LogError("'%s'", lua_tostring(l, i));
			break;

			case LUA_TBOOLEAN:  /* booleans */
			LogError(lua_toboolean(l, i) ? "true" : "false");
			break;

			case LUA_TNUMBER:  /* numbers */
			LogError("%g", lua_tonumber(l, i));
			break;

			default:  /* other values */
			LogError("%s", lua_typename(l, t));
			break;

		}
		LogError("  ");  /* put a separator */
	}
	LogError("\n");  /* end the listing */
}

static void InitCoLib() {
	static const luaL_Reg colib[] = {
		{"start", LUALAMBDA {
			auto th = lua_newthread(l);
			lua_insert(l, 1);
			lua_xmove(l, th, 1);

			CoroutineState state {};
			switch(lua_resume(th, l, 0)) {
				case LUA_OK: break;
				case LUA_YIELD:
					state.th = th;
					state.thidx = luaL_ref(l, LUA_REGISTRYINDEX);
					state.state = CoroutineState::StateYielded;

					if(lua_gettop(th) > 1) {
						auto st = *(s32*) luaL_checkudata(th, 1, "yieldmt");
						state.state = st;
						switch(st) {
							case CoroutineState::StateWaiting:
								state.timeout = luaL_checknumber(th, 2);
								break;

							default: break;
						}
					}

					coroutineStates.push_back(state);
					break;

				default:
					LogError("ERROR DURING StartCo: %s\n", lua_tostring(l, 1));
					break;
			}
			
			return 0;
		}},

		{"wait", LUALAMBDA {
			luaL_checknumber(l, 1);

			if(auto st = (s32*) lua_newuserdata(l, sizeof(s32))) {
				*st = CoroutineState::StateWaiting;
				luaL_getmetatable(l, "yieldmt");
				lua_setmetatable(l, -1);
				lua_insert(l, 1);

				return lua_yield(l, 2);
			}

			return 0;
		}},

		{"yield", LUALAMBDA {
			return lua_yield(l, lua_gettop(l));
		}},
	};

	luaL_newmetatable(l, "yieldmt");

	lua_newtable(l); // colib
	luaL_setfuncs(l, colib, 0);

	lua_setglobal(l, "co");
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
		{"__tostring", LUALAMBDA {
			auto v = lCheckVecRef(1);
			lua_pushfstring(l, "vec(%f, %f, %f)", v->x, v->y, v->z);
			return 1;
		}},

		#define OP(name, op) \
		{"__" #name, LUALAMBDA { \
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

		{"__unm", LUALAMBDA {
			*lNewVecUD() = -*lCheckVecRef(1);
			return 1;
		}},

		{nullptr, nullptr}
	};

	static const luaL_Reg veclib[] = {
		{"normalize", LUALAMBDA {
			*lNewVecUD() = glm::normalize(*lCheckVecRef(1));
			return 1;
		}},
		{"dot", LUALAMBDA {
			lua_pushnumber(l, glm::dot(*lCheckVecRef(1), *lCheckVecRef(2)));
			return 1;
		}},
		{"cross", LUALAMBDA {
			*lNewVecUD() = glm::cross(*lCheckVecRef(1), *lCheckVecRef(2));
			return 1;
		}},
		{"length", LUALAMBDA {
			lua_pushnumber(l, glm::length(*lCheckVecRef(1)));
			return 1;
		}},
		{"from_hsv", LUALAMBDA {
			*lNewVecUD() = HSVToRGB(*lCheckVecRef(1));
			return 1;
		}},
		{"to_hsv", LUALAMBDA {
			*lNewVecUD() = RGBToHSV(*lCheckVecRef(1));
			return 1;
		}},

		#define ACCESSOR(x) \
		{#x, LUALAMBDA { \
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
	lua_pushcfunction(l, LUALAMBDA {
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
		{"ball", LUALAMBDA {
			*lNewVecUD() = glm::ballRand(luaL_checknumber(l, 1));
			return 1;
		}},
		{"gauss", LUALAMBDA {
			if(auto mean = lTestVecRef(1)) {
				*lNewVecUD() = glm::gaussRand(*mean, *lCheckVecRef(2));
			}else if(lua_isnumber(l, 1)) {
				lua_pushnumber(l, glm::gaussRand(lua_tonumber(l, 1), luaL_checknumber(l, 2)));
			}else return 0;
			return 1;
		}},
		{"linear", LUALAMBDA {
			if(auto min = lTestVecRef(1)) {
				*lNewVecUD() = glm::linearRand(*min, *lCheckVecRef(2));
			}else if(lua_isnumber(l, 1)) {
				lua_pushnumber(l, glm::linearRand(lua_tonumber(l, 1), luaL_checknumber(l, 2)));
			}else return 0;
			return 1;
		}},
		{"spherical", LUALAMBDA {
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
		{"fog", LUALAMBDA {
			auto col = lCheckVecRef(1);
			f32 dist = luaL_checknumber(l, 2);
			f32 dens = luaL_checknumber(l, 3);
			f32 dura = luaL_optnumber(l, 4, 4.f);
			SetTargetFogParameters(*col, dist, dens, dura);
			return 0;
		}},

		{"fog_color", LUALAMBDA {
			SetTargetFogColor(*lCheckVecRef(1), luaL_optnumber(l, 2, 4.f));
			return 0;
		}},

		{"fog_density", LUALAMBDA {
			SetTargetFogDensity(luaL_checknumber(l, 1), luaL_optnumber(l, 2, 4.f));
			return 0;
		}},

		{"fog_distance", LUALAMBDA {
			SetTargetFogDistance(luaL_checknumber(l, 1), luaL_optnumber(l, 2, 4.f));
			return 0;
		}},

		{"vignette", LUALAMBDA {
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
		{"point", LUALAMBDA {
			auto p = lCheckVecRef(1);
			if(auto col = lTestVecRef(2)) {
				DebugPoint(*p, *col);
			}else{
				DebugPoint(*p);
			}

			return 0;
		}},

		{"line", LUALAMBDA {
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
		{"__tostring", LUALAMBDA {
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
		{"lookup", LUALAMBDA {
			if(lua_type(l, 1) == LUA_TSTRING) {
				auto e = FindEntity(lua_tostring(l, 1));
				if(e) {
					lNewEntUD(e);
				}else{
					lua_pushnil(l);
				}

				return 1;
			}

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

		{"name", LUALAMBDA {
			auto e = lCheckEnt(1);
			if(e) lua_pushlstring(l, e->name, e->nameLength);
			return e?1:0;
		}},

		{"id", LUALAMBDA {
			auto e = lCheckEnt(1);
			if(e) lua_pushinteger(l, e->id);
			return e?1:0;
		}},

		{"type", LUALAMBDA {
			auto e = lCheckEnt(1);
			if(e) lua_pushinteger(l, e->entityType);
			return e?1:0;
		}},
		{"type_name", LUALAMBDA {
			auto e = lCheckEnt(1);
			if(e) lua_pushstring(l, GetEntityTypeName(e->entityType));
			return e?1:0;
		}},

		{"pos", LUALAMBDA {
			auto e = lCheckEnt(1);
			if(e) *lNewVecUD() = e->position;
			return e?1:0;
		}},

		{"size", LUALAMBDA {
			auto e = lCheckEnt(1);
			if(e) *lNewVecUD() = e->extents * 2.f;
			return e?1:0;
		}},

		{"hidden", LUALAMBDA {
			auto e = lCheckEnt(1);
			if(e) lua_pushboolean(l, bool(e->flags&Entity::FlagHidden));
			return e?1:0;
		}},

		{"set_hidden", LUALAMBDA {
			if(auto e = lCheckEnt(1)) {
				if(lua_toboolean(l, 2)) {
					e->flags |= Entity::FlagHidden;
				}else{
					e->flags &= ~Entity::FlagHidden;
				}
			}

			return 0;
		}},

		{"layers", LUALAMBDA {
			if(auto e = lCheckEnt(1)) {
				u32 layers = e->layers;
				u32 numPushed = 0;
				for(u32 i = 0; i < 32; i++) {
					if((1<<i)&layers) {
						lua_pushinteger(l, i);
						numPushed++;
					}
				}
				return numPushed;
			}

			return 0;
		}},

		{"set_layers", LUALAMBDA {
			if(auto e = lCheckEnt(1)) {
				u32 nargs = lua_gettop(l);
				u32 layers = 0;
				for(u32 i = 2; i <= nargs; i++) {
					u32 ly = lua_tointeger(l, i);
					layers |= 1<<ly;
				}
				e->layers = layers;
				RefilterEntity(e);
			}
			return 0;
		}},

		{"attach_synth", LUALAMBDA {
			auto e = lCheckEnt(1);
			if(auto s = e?synth::GetSynthLua(l, 2):nullptr) {
				AttachSynthToEntity(e->id, s->id);
			}
			return 0;
		}},

		// Actions
		{"move_to", LUALAMBDA {
			auto e = lCheckEnt(1);
			auto t = *lCheckVecRef(2);
			f32 dur = luaL_optnumber(l, 3, 1.f);
			QueueEntityMoveToAnimation(e, t, dur);
			return 0;
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

void InitSynthExtensionLib() {
	static const luaL_Reg synthext[] = {
		{"set_falloff", LUALAMBDA {
			auto synth = synth::GetSynthLua(l, 1);
			const char* str = luaL_checkstring(l, 2);

			if(!strcmp(str, "constant")) {
				SetSynthFalloffMode(synth->id, FalloffConstant);
			}else if(!strcmp(str, "linear")) {
				SetSynthFalloffMode(synth->id, FalloffLinear);
			}else if(!strcmp(str, "exponential")) {
				SetSynthFalloffMode(synth->id, FalloffExponential);
			}else if(!strcmp(str, "logarithmic")) {
				SetSynthFalloffMode(synth->id, FalloffLogarithmic);
			}else{
				luaL_argerror(l, 2, "Unrecognised falloff mode");
			}

			return 0;
		}},

		{"set_falloff_distance", LUALAMBDA {
			auto synth = synth::GetSynthLua(l, 1);
			f32 dist = luaL_checknumber(l, 2);

			SetSynthFalloffDistance(synth->id, dist);
			return 0;
		}},

		{nullptr, nullptr}
	};

	synth::ExtendSynthLib(synthext);
}
