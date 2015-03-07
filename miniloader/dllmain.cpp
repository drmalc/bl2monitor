// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include <Windows.h>
#include <tchar.h> 
#include <stdio.h>
#include <strsafe.h>
#pragma comment(lib, "User32.lib")

#define MYNAME "miniloader.dll"
#define MAINDLLNAME "libbl2monitor.dll"

#define SUCCESS                     0L
#define FAILURE_NULL_ARGUMENT       1L
#define FAILURE_API_CALL            2L
#define FAILURE_INSUFFICIENT_BUFFER 3L

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		//Yes loading dlls in here is bad, but it doesn't matter here.
		TCHAR	path[MAX_PATH] = { 0 };

		GetModuleFileName(GetModuleHandle(_T(MYNAME)), path, MAX_PATH);
		TCHAR basePath[MAX_PATH] = { 0 };
		ZeroMemory(basePath, MAX_PATH*sizeof(TCHAR));
		TCHAR *c = basePath + MAX_PATH;

		StringCchCopy(basePath, MAX_PATH, path);
		while (c > basePath)
		{
			if (*c == '\\' || *c == '/')
			{
				*c = 0;
				break;
			}
			else
			{
				*c = 0;
				c--;
			}
		}

		WIN32_FIND_DATA ffd;
		HANDLE hFind = INVALID_HANDLE_VALUE;
		DWORD dwError = 0;
		TCHAR searchPath[MAX_PATH] = { 0 };

		StringCchCopy(searchPath, MAX_PATH, basePath);
		StringCchCat(searchPath, MAX_PATH, TEXT("\\*"));
		hFind = FindFirstFile(searchPath, &ffd);

		if (INVALID_HANDLE_VALUE == hFind)
		{
			MessageBox(NULL, searchPath, _T("Failed to load dll."), MB_OK);
			break;
		}

		do
		{
			if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				TCHAR	*filename = ffd.cFileName;

				if (_tcsstr(filename, _T(".dll")))
				{
					if (!_tcsstr(filename, _T(MYNAME)) && !_tcsstr(filename, _T(MAINDLLNAME)))
					{
						_stprintf_s(path, _T("%s\\%s"), basePath, filename);

						HMODULE h = NULL;
						h = LoadLibrary(path);
						if (INVALID_HANDLE_VALUE == h)
						{
							MessageBox(NULL, path, _T("Failed to load dll."), MB_OK);
							break;
						}
					}
				}

			}
		} while (FindNextFile(hFind, &ffd) != 0);
		FindClose(hFind);

		HMODULE h = NULL;

		_stprintf_s(path, _T("%s\\%s"), basePath, _T(MAINDLLNAME));
		h = LoadLibrary(path);
		if (INVALID_HANDLE_VALUE == h)
		{
			MessageBox(NULL, path, _T("Failed to load dll."), MB_OK);
			break;
		}

		break;
	}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

