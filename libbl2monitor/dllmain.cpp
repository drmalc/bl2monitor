#include "stdafx.h"
#include <Windows.h>

DWORD WINAPI mainLoop(LPVOID lpParam);

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		//MessageBoxW(NULL, L"Load OK", L"", MB_OK);
		CreateThread(0, 0x1000, &mainLoop, 0, 0, NULL);
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

