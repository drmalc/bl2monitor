#pragma once

#define PATH_SEPARATOR "\\"

namespace Utilities
{
	void Initialize();
	const char *ServerPath();
	const char *MainLuaPath();
	const char *LogPath();
	const char *LayoutPath();
	HWND getToplevelWindows();
	DWORD GetMainThreadId(DWORD dwPid);
	HANDLE GetMainThreadHandle(DWORD dwPid, DWORD dwDesiredAccess);
}
