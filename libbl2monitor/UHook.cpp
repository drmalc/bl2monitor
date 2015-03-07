#include "stdafx.h"
#include "GameSDK.h"
#include "UHook.h"
#include "Log.h"
#include "CSigScan.h"
#include "Signatures.h"
#include "Hook.h"
#include "GameHooks.h"
#include "bl2Methods.h"
#include "CLua.h"
#include "Utilities.h"
#include "DX9Hook.h"
#include "CEGUIManager.h"
#include <Windows.h>
#include <tchar.h>
#include <winternl.h>
#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)
#define STATUS_PORT_NOT_SET ((NTSTATUS)0xC0000353L)

using namespace std;

namespace UHook
{
	bool isHooked = false;

	//Prototypes for hookable functions
	typedef void(__thiscall *tProcessEvent) (UObject*, UFunction*, void*, void*);
	typedef void(__thiscall *tCallFunction) (UObject*, FFrame&, void* const, UFunction*);
	typedef void(__thiscall *tFrameStep) (FFrame*, UObject*, void* const);
	typedef UObject* (*tStaticConstructObject) (UClass* inClass, UObject* outer, FName name, unsigned int flags, UObject* inTemplate, FOutputDevice* error, UObject* root, void* unk);
	typedef UPackage* (*tLoadPackage) (UPackage* outer, const wchar_t* filename, DWORD flags);
	typedef FArchive& (__thiscall *tByteOrderSerialize) (FArchive* Ar, void* V, int Length);

	//Internal functions prototypes
	bool GetCanvasPostRender(UObject* caller, UFunction* function, void* parms, void* result);
	bool InputKey(UObject* caller, UFunction* function, void* parms, void* result);

	//Scanned memory addresses
	void* pGObjects = NULL;
	void* pGNames = NULL;
	void* pGObjHash = NULL;
	void* pGCRCTable = NULL;
	void* pNameHash = NULL;
	void* pTextureFixLocation;
	tProcessEvent pProcessEvent;
	tCallFunction pCallFunction;
	tFrameStep pFrameStep;
	tStaticConstructObject pStaticConstructObject;
	tLoadPackage pLoadPackage;
	tByteOrderSerialize pByteOrderSerialize;
	FMalloc** pGMalloc;

	//Hard hooks
	Hook	*processEventHook = NULL, *callFunctionHook = NULL;

	Hook	*setInfoHook = NULL, *queryInfoHook = NULL;
	typedef NTSTATUS(WINAPI* tNtSIT) (HANDLE, THREAD_INFORMATION_CLASS, PVOID, ULONG);
	tNtSIT pNtSetInformationThread = nullptr;
	typedef NTSTATUS(WINAPI* tNtQIP) (HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG);
	tNtQIP pNtQueryInformationProcess = nullptr;

	// --- Callbacks for hooked functions
	void _stdcall hookedProcessEvent(UFunction* fct, void* Parms, void* result)
	{
		UObject* pThis;
		_asm mov pThis, ecx;

		std::string callerName = pThis->GetFullName();
		std::string functionName = fct->GetFullName();

		Log::debug("ProcessEvent: %s | %s", callerName.c_str(), functionName.c_str());

		if (!GameHooks::ProcessEngineHooks(pThis, fct, Parms, result))
		{
			// The engine hook manager told us not to pass this function to the engine
			return;
		}

		//Call original function
		processEventHook->Unpatch();
		pProcessEvent(pThis, fct, Parms, result);
		processEventHook->Patch();
	}

	void __stdcall hookedCallFunction(FFrame& stack, void* const result, UFunction* function)
	{
		UObject* pThis;
		_asm mov pThis, ecx;



		//Call original function
		callFunctionHook->Unpatch();
		pCallFunction(pThis, stack, result, function);
		callFunctionHook->Patch();
	}

	NTSTATUS NTAPI hkNtSetInformationThread(
		__in HANDLE ThreadHandle,
		__in THREAD_INFORMATION_CLASS ThreadInformationClass,
		__in PVOID ThreadInformation,
		__in ULONG ThreadInformationLength)
	{
		if (ThreadInformationClass == 17) // ThreadHideFromDebugger
		{
			Log::info("[AntiDebug] NtSetInformationThread called with ThreadHideFromDebugger");
			return STATUS_SUCCESS;
		}
		setInfoHook->Unpatch();
		NTSTATUS i = pNtSetInformationThread(ThreadHandle, ThreadInformationClass, ThreadInformation, ThreadInformationLength);
		setInfoHook->Patch();
		return i;
	}

	NTSTATUS WINAPI hkNtQueryInformationProcess(
		__in HANDLE ProcessHandle,
		__in PROCESSINFOCLASS ProcessInformationClass,
		__out PVOID ProcessInformation,
		__in ULONG ProcessInformationLength,
		__out_opt PULONG ReturnLength)
	{
		if (ProcessInformationClass == 30) // ProcessDebugObjectHandle
		{
			return STATUS_PORT_NOT_SET;
		}
		queryInfoHook->Unpatch();
		NTSTATUS i = pNtQueryInformationProcess(ProcessHandle, ProcessInformationClass, ProcessInformation, ProcessInformationLength, ReturnLength);
		queryInfoHook->Patch();
		return i;
	}

	void setupAntiAntiDebug()
	{
		Log::info("Setting up anti anti debugger");
		HMODULE hNtdll = GetModuleHandle(L"ntdll.dll");

		pNtSetInformationThread = (tNtSIT)GetProcAddress(hNtdll, "NtSetInformationThread");
		setInfoHook = new Hook((void*)pNtSetInformationThread, (void*)*hkNtSetInformationThread);
		setInfoHook->Patch();

		pNtQueryInformationProcess = (tNtQIP)GetProcAddress(hNtdll, "NtQueryInformationProcess");
		queryInfoHook = new Hook((void*)pNtQueryInformationProcess, (void*)*hkNtQueryInformationProcess);
		queryInfoHook->Patch();
	}

	//DX9Hook is informing us that the device is available
	void d3d9DeviceAvailable()
	{
		CLua::Initialize();
		CEGUIManager::Initialize(DX9Hook::Device(), CLua::getLuaState());
		CLua::SetupFunctions();
		CEGUIManager::RunLua("autorun.lua");
	}

	//This is where we draw our dx stuff.
	void renderScene()
	{
		CEGUIManager::Render();
	}

	// --- Soft hooks
	// This function is used to ensure that everything gets called in the game thread once the game itself has loaded
	bool GameReady(UObject* caller, UFunction* function, void* parms, void* result)
	{
		Log::info("StartupSDK soft hook called.");

		//Init console
		bl2Methods::logToConsole(NULL);

		GameHooks::EngineHookManager->RemoveStaticHook(function, "StartupSDK");
		GameHooks::EngineHookManager->Register("Function WillowGame.WillowGameViewportClient:PostRender", "GetCanvas", &GetCanvasPostRender);
		if (!DX9Hook::Initialize())
		{
			Log::error("Failed to hook d3d9.");
		}
		return true;
	}

	// This function is used to get the dimensions of the game window
	bool GetCanvasPostRender(UObject* caller, UFunction* function, void* parms, void* result)
	{
		Log::info("GetCanvas soft hook called.");

		GameHooks::EngineHookManager->RemoveStaticHook(function, "GetCanvas");
		GameHooks::EngineHookManager->Register("Function WillowGame.WillowGameViewportClient:InputKey", "InputKey", &InputKey);
		return true;
	}

	//Called for any input, including mouse clicks.
	bool InputKey(UObject* caller, UFunction* function, void* parms, void* result)
	{
		UWillowGameViewportClient_execInputKey_Parms* realParms = reinterpret_cast<UWillowGameViewportClient_execInputKey_Parms*>(parms);

		if (realParms->EventType == 0) //key down
		{
			const char* name = realParms->Key.GetName();
			if (strcmp(name, "F2") == 0)
			{
				//debug tests

				Log::info("Server path: %s", Utilities::ServerPath());
			}
			else if (strcmp(name, "F1") == 0)
			{
				Log::info("InputKey soft hook called for key F1.");

				/*//dump GObjObjects
				TArray< UObject* >* ar = UObject::GObjObjects();
				TArray< UObject* >	r = *ar;
				int k = ar->Count;
				for (int i = 0; k > i; i++)
				{
					Log::debug("GObjObjects | %s", ((UObject*)(r(i)))->GetFullName().c_str());
				}
				return true;*/

				//UObject::dumpObjects();

				UObject* gameEngine = bl2Methods::getGameEngine();
				Log::info("Game engine is: 0x%p", gameEngine);
				if (gameEngine)
				{
					UEngine *e = (UEngine*)gameEngine;
					ULocalPlayer *localPlayer = (ULocalPlayer*)e->GamePlayers(0);

					Log::info("Local Player: 0x%p", localPlayer);
					if (localPlayer)
					{
						UPlayer *player = (UPlayer*)localPlayer;
						APlayerController *actor = player->Actor;

						Log::info("Actor: 0x%p", actor);
						if (actor)
						{
							return true;

							FVector v = actor->Pawn->Mesh->GetBoneLocation(FName("Head"), 0);
							Log::info("Position: %.2f / %.2f / %.2f", v.X, v.Y, v.Z);

							// Doesnt work.
							// class UClass* SpawnClass, class AActor* SpawnOwner, struct FName SpawnTag, struct FVector SpawnLocation, struct FRotator SpawnRotation, class AActor* ActorTemplate, unsigned long bNoCollisionFail
							actor->Spawn(
								AWillowVehicle_WheeledVehicle::StaticClass(),
								NULL,
								FName("omgcar"),
								v,
								{0, 0, 0},
								(AActor*)UObject::GObjObjects()->Data[98318 - 1],
								0
								);
						}
					}

				}
			}
		}

		return true;
	}

	// --- Accessors
	Hook *getProcessEventHook(){
		return processEventHook;
	}

	// --- Find the offsets and install the hooks
	//This is were the initialization happens.
	bool hook()
	{
		if (isHooked)
			return true;

		//------ CEGUI Debug -------------
		TCHAR	exePath[MAX_PATH] = { 0 };
		GetModuleFileName(NULL, exePath, MAX_PATH);
		if (_tcsstr(exePath, _T("d3d9Test")))
		{
			//This is the test exe, not BL2.
			Log::info("Test module detected.");

			DX9Hook::SetDeviceAvailableCallback(&d3d9DeviceAvailable);
			DX9Hook::SetEndSceneCallback(&renderScene);
			if (!DX9Hook::Initialize())
			{
				Log::error("Failed to hook d3d9.");
			}
			isHooked = true;
			return true;
		}
		//---------------------------------

		Log::info("Attempting to find the addresses for the target...");
		HMODULE		module = GetModuleHandle(NULL);
		CSigScan	sigscan(module);

		// Sigscan for GOBjects
		pGObjects = *(void**)sigscan.Scan(Signatures::GObjects);
		Log::info("Address for pGObjects: 0x%p", pGObjects);

		// Sigscan for GNames
		pGNames = *(void**)sigscan.Scan(Signatures::GNames);
		Log::info("Address for pGNames: 0x%p", pGNames);

		// Sigscan for UObject::ProcessEvent which will be used for pretty much everything
		pProcessEvent = reinterpret_cast<tProcessEvent>(sigscan.Scan(Signatures::ProcessEvent));
		Log::info("Address for pProcessEvent: 0x%p", pProcessEvent);

		// Sigscan for UObject::GObjHash
		pGObjHash = *(void**)sigscan.Scan(Signatures::GObjHash);
		Log::info("Address for pGObjHash: 0x%p", pGObjHash);

		// Sigscan for GCRCTable
		pGCRCTable = *(void**)sigscan.Scan(Signatures::GCRCTable);
		Log::info("Address for pGCRCTable: 0x%p", pGCRCTable);

		// Sigscan for NameHash
		pNameHash = *(void**)sigscan.Scan(Signatures::NameHash);
		Log::info("Address for pNameHash: 0x%p", pNameHash);

		// Sigscan for Unreal exception handler
		void* addrUnrealEH = sigscan.Scan(Signatures::CrashHandler);
		Log::info("Address for addrUnrealEH: 0x%p", addrUnrealEH);

		// Sigscan for UObject::CallFunction
		pCallFunction = reinterpret_cast<tCallFunction>(sigscan.Scan(Signatures::CallFunction));
		Log::info("Address for pCallFunction: 0x%p", pCallFunction);

		// Sigscan for FFrame::Step
		pFrameStep = reinterpret_cast<tFrameStep>(sigscan.Scan(Signatures::FrameStep));
		Log::info("Address for pFrameStep: 0x%p", pFrameStep);

		// Sigscan for UObject::StaticConstructObject
		pStaticConstructObject = reinterpret_cast<tStaticConstructObject>(sigscan.Scan(Signatures::StaticConstructor));
		Log::info("Address for pStaticConstructObject: 0x%p", pStaticConstructObject);

		// Sigscan for UObject::LoadPackage
		pLoadPackage = reinterpret_cast<tLoadPackage>(sigscan.Scan(Signatures::LoadPackage));
		Log::info("Address for pLoadPackage: 0x%p", pLoadPackage);

		// Sigscan for FArchive::ByteOrderSerialize
		pByteOrderSerialize = reinterpret_cast<tByteOrderSerialize>(sigscan.Scan(Signatures::ByteOrderSerialize));
		Log::info("Address for pByteOrderSerialize: 0x%p", pByteOrderSerialize);

		// Sigscan for texture load fix location
		pTextureFixLocation = sigscan.Scan(Signatures::TextureFixLocation);
		Log::info("Address for pTextureFixLocation: 0x%p", pTextureFixLocation);

		// Sigscan for GMalloc and its virtual function table
		pGMalloc = *(FMalloc***)sigscan.Scan(Signatures::GMalloc);
		Log::info("Address for pGMalloc: 0x%p", pGMalloc);

		isHooked = true;

		//Create the hooks
		if (pProcessEvent)
		{
			processEventHook = new Hook((void*)pProcessEvent, (void*)*hookedProcessEvent);
			processEventHook->Patch();
		}
		else
		{
			Log::error("Failed to find address for pProcessEvent.");
			isHooked = false;
		}
		
		if (isHooked && pCallFunction)
		{
			callFunctionHook = new Hook((void*)pCallFunction, (void*)*hookedCallFunction);
			callFunctionHook->Patch();
		}
		else
		{
			Log::error("Failed to hook pCallFunction.");
			isHooked = false;
		}

		if (isHooked)
		{
			GameHooks::Initialize();
			GameHooks::EngineHookManager->Register("Function WillowGame.WillowGameInfo:InitGame", "StartupSDK", &GameReady);
		}
		else
		{
			Log::error("The process failed to load.");
			isHooked = false;
		}

		DX9Hook::SetDeviceAvailableCallback(&d3d9DeviceAvailable);
		DX9Hook::SetEndSceneCallback(&renderScene);

		setupAntiAntiDebug();

		return isHooked;
	}

}
