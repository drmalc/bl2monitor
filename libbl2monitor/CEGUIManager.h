#pragma once
#include "lua.hpp"

struct IDirect3DDevice9;
namespace CEGUIManager
{
	bool Initialize(IDirect3DDevice9 *dev, lua_State* luaState);
	void CleanUp();
	void Render();
	void RunLua(const char*path);
}

