#include "stdafx.h"
#include "Utilities.h"
#include "NamedPipe.h"
#include "Log.h"
#include <vector>

namespace Utilities
{
	static bool isInit = false;
	static NamedPipe *pipe = NULL;
	static const char pipeName[] = "\\\\.\\pipe\\bl2monitorpipeutils";
	static char serverPath[MAX_PATH] = { 0 };
	static char mainLuaPath[MAX_PATH] = { 0 };
	static char logPath[MAX_PATH] = { 0 };
	static char layoutPath[MAX_PATH] = { 0 };

	void SendRequest(const char*req, unsigned len, char* buffer, unsigned *buflen)
	{//Clean me up
		if (!isInit || !buffer)
			return;
		pipe->Flush();
		if (pipe->Write(req, len) != len)
		{
			*buflen = 0;
			*buffer = 0;
			Log::error("Failed to send request.");
			return;
		}
		*buffer = 0;
		unsigned total = 0;
		bool endCharFound = false;
		for (unsigned i = 0; 3 > i; i++){
			total += pipe->ReadImmediate(buffer + total, *buflen - total);
			for (unsigned j = 0; total > j; j++)
			{
				if (buffer[j] == '\n'){
					endCharFound = true;
					buffer[j] = 0;
					break;
				}
			}
			if (endCharFound)
				break;
		}
		if (endCharFound)
			*buflen = total;
		else{
			*buflen = 0;
			*buffer = 0;
		}
	}

	void Initialize()
	{
		if (isInit)
			return;
		isInit = true;

		pipe = new NamedPipe(pipeName);
		pipe->Open();
	}

	void CleanUp()
	{
		if (pipe)
		{
			pipe->Close();
			delete pipe;
			pipe = NULL;
		}
	}

	const char *ServerPath()
	{
		const char req[] = "PATH\n";
		if (*serverPath)
			return serverPath;
		unsigned l = sizeof(serverPath);
		SendRequest(req, sizeof(req)-1, serverPath, &l);
		return serverPath;
	}

	const char *MainLuaPath()
	{
		const char req[] = "LUAMAIN\n";
		if (*mainLuaPath)
			return mainLuaPath;
		unsigned l = sizeof(mainLuaPath);
		SendRequest(req, sizeof(req)-1, mainLuaPath, &l);
		return mainLuaPath;
	}

	const char *LogPath()
	{
		const char req[] = "LOG\n";
		if (*logPath)
			return logPath;
		unsigned l = sizeof(logPath);
		SendRequest(req, sizeof(req) - 1, logPath, &l);
		return logPath;
	}

	const char *LayoutPath()
	{
		const char req[] = "LAYOUT\n";
		if (*layoutPath)
			return layoutPath;
		unsigned l = sizeof(layoutPath);
		SendRequest(req, sizeof(req) - 1, layoutPath, &l);
		return layoutPath;
	}

	//Get top window
	static HWND	topWnd = NULL;
	struct EnumWindowsCallbackArgs {
		EnumWindowsCallbackArgs(DWORD p) : pid(p) { }
		const DWORD pid;
		std::vector<HWND> handles;
	};

	static BOOL CALLBACK EnumWindowsCallback(HWND hnd, LPARAM lParam)
	{
		EnumWindowsCallbackArgs *args = (EnumWindowsCallbackArgs *)lParam;
		DWORD windowPID;
		(void)::GetWindowThreadProcessId(hnd, &windowPID);
		if (windowPID == args->pid) {
			args->handles.push_back(hnd);
		}
		return TRUE;
	}

	HWND getToplevelWindows()
	{
		if (topWnd)
			return topWnd;
		EnumWindowsCallbackArgs args(::GetCurrentProcessId());
		if (::EnumWindows(&EnumWindowsCallback, (LPARAM)&args) == FALSE) {
			return NULL;
		}
		return topWnd = args.handles[0];
	}

	DWORD GetMainThreadId(DWORD dwPid)
	{
		LPVOID lpTid;
		_asm
		{
			mov eax, fs:[18h]
			add eax, 36
			mov[lpTid], eax
		}
		HANDLE hProcess = OpenProcess(PROCESS_VM_READ, FALSE, dwPid);
		if (hProcess == NULL)
			return NULL;
		DWORD dwTid;
		if (ReadProcessMemory(hProcess, lpTid, &dwTid, sizeof(dwTid), NULL) == FALSE)
		{
			CloseHandle(hProcess);
			return NULL;
		}
		CloseHandle(hProcess);
		return dwTid;
	}

	HANDLE GetMainThreadHandle(DWORD dwPid, DWORD dwDesiredAccess)
	{
		DWORD dwTid = GetMainThreadId(dwPid);
		if (dwTid == FALSE)
			return NULL;
		return OpenThread(dwDesiredAccess, FALSE, dwTid);
	}

}
