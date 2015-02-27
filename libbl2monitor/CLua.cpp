#include "stdafx.h"
#include "CLua.h"
#pragma comment( lib, "lua51" )

#include "lua.hpp"

namespace CLua
{
	static lua_State	*m_pState = NULL;

	void Initialize()
	{
		if (m_pState)
			return;

		m_pState = luaL_newstate();
		luaL_openlibs(m_pState);
		
	}
}
