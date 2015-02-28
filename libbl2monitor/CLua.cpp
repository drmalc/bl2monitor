#include "stdafx.h"
#include "CLua.h"
#include "Log.h"
#pragma comment( lib, "lua51" )
#include "lua.hpp"
#include "Utilities.h"
#include "bl2Methods.h"

#if LUA_VERSION_NUM < 520
#define luaL_newlib(x, y) luaL_register(x, #y, y)
#endif

namespace CLua
{
	static lua_State	*m_pState = NULL;

	//Simple test function for lua scripts
	static int ping(lua_State*)
	{
		Log::lua("Hello World");
		return 0;
	}

	static int logServer(lua_State* L)
	{
		const char *str = luaL_checkstring(L, 1);
		Log::lua(str);
		return 0;
	}

	static int logConsole(lua_State* L)
	{
		const char *str = luaL_checkstring(L, 1);
		bl2Methods::logToConsole(str);
		return 0;
	}

	static const luaL_Reg base_funcs[] = {
		{ "ping", ping },
		{ NULL, NULL }
	};

	static const luaL_Reg log[] = {
		{ "server", logServer },
		{ "console", logConsole },
		{ NULL, NULL }
	};

	void Initialize()
	{
		if (m_pState)
			return;

		m_pState = luaL_newstate();
		luaL_openlibs(m_pState); //Opens all standard Lua libraries into the given state. 
		lua_pushvalue(m_pState, LUA_GLOBALSINDEX);
		luaL_register(m_pState, NULL, base_funcs); //register global functions

		//Register local libraries
		luaL_newlib(m_pState, log);

		const char	*mainLua = Utilities::MainLuaPath();
		Log::info("Lua initialized (" LUA_VERSION "). Loading %s...", mainLua);
		if (*mainLua)
		{
			int err = luaL_dofile(m_pState, mainLua);
			Log::info("...autorun.lua loaded.");
			if (err)
			{
				Log::error("luaL_dofile: %s\n", lua_tostring(m_pState, -1));
			}
		}
	}
}
