#pragma once
#include "lua.hpp"

#define FFI_EXPORT extern "C" __declspec(dllexport)

namespace CLua
{
	void Initialize();
	void Autorun();
	void CleanUp();
	lua_State* getLuaState();
	void SetupFunctions();
}

