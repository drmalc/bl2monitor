#include "stdafx.h"
#include <Windows.h>
#include <stdio.h>
#include "GameSDK.h"
#include "bl2Methods.h"
#include "Log.h"
#include "UHook.h"

namespace bl2Methods
{
	static UConsole* gameConsole = NULL;

	void logToConsole(const char *text)
	{
		if (!gameConsole)
		{
			// There should only be 1 instance so we should be right to just use it in this way
			gameConsole = UObject::FindObject<UConsole>("WillowConsole Transient.WillowGameEngine_0:WillowGameViewportClient_0.WillowConsole_0");
			if (gameConsole)
			{
				Log::info("Found console: 0x%p", gameConsole);
				Log::info("Console key is: %s", gameConsole->ConsoleKey.GetName());
				if (gameConsole->ConsoleKey == FName("None") || gameConsole->ConsoleKey == FName("Undefine"))
				{
					gameConsole->ConsoleKey = FName("Tilde");
					Log::info("Setting console key to Tilde.");
				}
			}
		}
		if (gameConsole && text && *text)
		{
			size_t requiredSize = 0;
			mbstowcs_s(&requiredSize, NULL, 0, text, 0);
			wchar_t* wcstring = new wchar_t[requiredSize + 1];
			mbstowcs_s(&requiredSize, wcstring, requiredSize + 1, text, requiredSize);

			UHook::getProcessEventHook()->Unpatch();
			gameConsole->eventOutputText(FString(wcstring));
			UHook::getProcessEventHook()->Patch();

			delete[] wcstring;
		}
	}
}
