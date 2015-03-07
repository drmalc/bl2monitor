#include "stdafx.h"
#include "CLua.h"
#include "Log.h"
#include "lua.hpp"
#include "Utilities.h"
#include "bl2Methods.h"

#if LUA_VERSION_NUM > 501
#define lua_strlen lua_rawlen
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

	//Simple test class for lua scripts
	class TestClass
	{
	private:
		int myInt;
	public:
		TestClass(int i){ myInt = i; }
		~TestClass(){
			Log::info("TestClass: Destroying object.");
		}
		int PrintSomething(){
			Log::info("TestClass: PrintSomething called with myInt=%d", myInt);
			return myInt;
		}
	};

	static void getLuaDirPath(char *buffer)
	{
		const char *str = Utilities::MainLuaPath();

		::strncpy_s(buffer, MAX_PATH, str, MAX_PATH);
		unsigned l = strnlen_s(buffer, MAX_PATH);
		char *c = buffer + l;
		while (*c != '/' && *c != '\\'){
			*c = 0;
			c--;
		}
	}

	static int luaPath(lua_State* L)
	{
		char buffer[MAX_PATH];
		getLuaDirPath(buffer);
		lua_pushstring(L, buffer);
		return 1;
	}

	static int luaModPath(lua_State* L)
	{
		char buffer[MAX_PATH];
		getLuaDirPath(buffer);
		strncat_s(buffer, "mods", sizeof(buffer));
		lua_pushstring(L, buffer);
		return 1;
	}

	static int luaDestroy(lua_State* L)
	{
		const void *v = lua_topointer(L, 1);
		delete v;
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

	//Global functions
	static const luaL_Reg base_funcs[] = {
		{ "ping", ping },
		{ "luaPath", luaPath },
		{ "luaModPath", luaModPath },
		{ "luaDestroy", luaDestroy },
		{ NULL, NULL }
	};

	//"log" library
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

		//register global functions
#if LUA_VERSION_NUM > 501
		lua_getglobal(m_pState, "_G");
		luaL_setfuncs(m_pState, base_funcs, 0);
#else
		lua_pushvalue(m_pState, LUA_GLOBALSINDEX);
		luaL_register(m_pState, NULL, base_funcs);
#endif

		//Register local libraries
#if LUA_VERSION_NUM > 501
		lua_newtable(m_pState);
		luaL_setfuncs(m_pState, log, 0);
		lua_pushvalue(m_pState, -1);
		lua_setglobal(m_pState, "log");
#else
		lua_pushvalue(m_pState, LUA_GLOBALSINDEX);
		luaL_register(m_pState, "log", log);
#endif

		//reset the stack (not required)
		lua_settop(m_pState, 0);
	}

	void Autorun()
	{
		const char	*mainLua = Utilities::MainLuaPath();
		Log::info("Lua initialized (" LUA_VERSION "). Loading %s...", mainLua);
		if (*mainLua)
		{
			int err = luaL_loadfile(m_pState, mainLua);
			if (err)
			{
				Log::error("luaL_loadfile: %s\n", lua_tostring(m_pState, -1));
			}
			else
			{
				err = lua_pcall(m_pState, 0, LUA_MULTRET, 0); //run lua
				if (err)
				{
					Log::error("lua_pcall: %s\n", lua_tostring(m_pState, -1));
				}
				else
				{
					lua_getglobal(m_pState, "initialize"); //get init function
					err = lua_pcall(m_pState, 0, 0, 0); //run init function
					if (err)
					{
						Log::error("Failed to load autorun: %s\n", lua_tostring(m_pState, -1));
					}
				}
			}
			Log::info("...autorun.lua loaded.");
		}
	}

	void CleanUp()
	{
		if (!m_pState)
			return;
		int err;

		lua_getglobal(m_pState, "cleanup"); //get cleanup function
		err = lua_pcall(m_pState, 0, 0, 0); //run cleanup function
		if (err)
		{
			Log::error("Failed to cleanup autorun: %s\n", lua_tostring(m_pState, -1));
		}

		lua_close(m_pState);
		m_pState = NULL;
	}
}
