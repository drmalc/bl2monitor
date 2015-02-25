#include "stdafx.h"
#include <Windows.h>
#include <iostream>
#include <fstream>
#include "Log.h"
#include "UHook.h"
#include "bl2Methods.h"

using namespace std;

DWORD WINAPI mainLoop(LPVOID lpParam)
{
	Log::setLogDebug(true); //Debug will produces a huge amount of log. True = we send everything and let the server decide what to log.
	Log::info("Library successfully loaded into target.");
	UHook::hook(); //Initialization.

	while (1)
	{
		Sleep(2000);
		//bl2Methods::logToConsole("Hello World");
	}
}
