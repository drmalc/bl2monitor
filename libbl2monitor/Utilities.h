#pragma once

#define PATH_SEPARATOR "\\"

namespace Utilities
{
	void Initialize();
	const char *ServerPath();
	const char *MainLuaPath();
	const char *ImagesPath();
	const char *LayoutPath();
}
