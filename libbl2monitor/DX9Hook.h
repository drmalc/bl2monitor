#pragma once
#include <vector>

#define LIVEHOOK 1

struct IDirect3DDevice9;
namespace DX9Hook
{
	typedef void(*dx9DeviceAvailableType)();
	typedef void(*endSceneCallbackType)();

	void SetDeviceAvailableCallback(dx9DeviceAvailableType cb);
	void SetEndSceneCallback(endSceneCallbackType cb);
	bool Initialize();
	long CanvasWidth();
	long CanvasHeight();
	IDirect3DDevice9 *Device();
	std::vector<HWND> getToplevelWindows();
}
