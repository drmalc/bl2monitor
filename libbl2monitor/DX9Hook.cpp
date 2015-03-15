#include "stdafx.h"
#include "DX9Hook.h"
#include <vector>
#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")
#include "Hook.h"
#include "Log.h"
#include "Utilities.h"

#define PRESENT_INDEX		17
#define ENDSCENE_INDEX		42
#define CREATEDEVICE_INDEX	16

using namespace std;
namespace DX9Hook
{
	typedef HRESULT(_stdcall *RealPresentFuncType)(void*, const RECT*, const RECT*, HWND, const RGNDATA*);
	RealPresentFuncType RealPresent;
	HRESULT _stdcall Present(void *pThis, const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion);

	typedef HRESULT(_stdcall *RealEndSceneFuncType)(void *pThis);
	RealEndSceneFuncType RealEndScene;
	HRESULT _stdcall EndScene(void *pThis);

#if !LIVEHOOK
	typedef HRESULT(WINAPI* RealCreateDeviceFuncType)(IDirect3D9*, UINT, D3DDEVTYPE, HWND, DWORD, D3DPRESENT_PARAMETERS*, IDirect3DDevice9**);
	RealCreateDeviceFuncType RealCreateDevice = NULL;
	HRESULT WINAPI hkCreateDevice(IDirect3D9* pD3D, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters,
		IDirect3DDevice9** ppReturnedDeviceInterface);
	static Hook *d3d9CreateDeviceHook = NULL;
#endif

	static Hook *d3d9Hook = NULL;
	static bool isInit = false;
	static long xwidth, xheight = 0;
	static IDirect3DDevice9 *d3d9Device = NULL;
	static dx9DeviceAvailableType callback = NULL;
	static endSceneCallbackType endsceneCallback = NULL;

	//This is the hooked EndScene function.
	HRESULT _stdcall EndScene(void *pThis)
	{
		IDirect3DDevice9	*pI = (IDirect3DDevice9 *)pThis;
		IDirect3DStateBlock9	*stateBlock;
		pI->CreateStateBlock(D3DSBT_ALL, &stateBlock); //Save current state.

		//Get window size
		D3DVIEWPORT9	gViewPort;
		pI->GetViewport(&gViewPort);
		xwidth = (long)gViewPort.Width;
		xheight = (long)gViewPort.Height;
		//long	centerX = xwidth / 2;
		//long	centerY = xheight / 2;

		if (!isInit)
		{
			isInit = true;
			d3d9Device = pI;
			Log::info("D3d9 EndScene called for the first time.");
			if (callback)
			{
				(*callback)();
			}
		}

		//Set identity
		const D3DMATRIX	mm = {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
		};
		pI->SetTransform(D3DTS_PROJECTION, &mm);
		pI->SetTransform(D3DTS_WORLD, &mm);
		pI->SetTransform(D3DTS_VIEW, &mm);
		//Set render state
		pI->SetRenderState(D3DRS_ZENABLE, FALSE);
		pI->SetRenderState(D3DRS_LIGHTING, FALSE);
		pI->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

		//Visual test. Will show a red rectangle, top-left corner.
		//D3DRECT	rr = { 10, 10, 100, 40 }; pI->Clear(1, &rr, D3DCLEAR_TARGET, 0xff0000, 0.0, 0);

		if (endsceneCallback)
		{
			(*endsceneCallback)();
		}

		//Restore state, call original function
		stateBlock->Apply();
		stateBlock->Release();
		d3d9Hook->Unpatch();
		HRESULT hr = RealEndScene(pThis);
		d3d9Hook->Patch();
		return hr;
	}

#if !LIVEHOOK
	HRESULT WINAPI hkCreateDevice(IDirect3D9* pD3D, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow,
		DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters,
		IDirect3DDevice9** ppReturnedDeviceInterface)
	{
		d3d9CreateDeviceHook->Unpatch();
		HRESULT result = RealCreateDevice(pD3D, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

		Log::info("...successfully installed d3d9 hook.");

		IDirect3DDevice9	*pI = *ppReturnedDeviceInterface;
		PVOID	*pVTable = (PVOID*)*((DWORD*)pI);
		RealPresent = (RealPresentFuncType)pVTable[PRESENT_INDEX];
		RealEndScene = (RealEndSceneFuncType)pVTable[ENDSCENE_INDEX];

		//Install hook
		d3d9Hook = new Hook((void*)RealEndScene, (void*)*EndScene);
		d3d9Hook->Patch();

		return result;
	}
#endif

	void SetDeviceAvailableCallback(dx9DeviceAvailableType cb)
	{
		callback = cb;
	}

	void SetEndSceneCallback(endSceneCallbackType cb)
	{
		endsceneCallback = cb;
	}

	bool Initialize()
	{
		if (d3d9Hook)
			return true;
		Log::info("Attempting to hook d3d9...");

#if !LIVEHOOK
		if (d3d9CreateDeviceHook)
			return true;
		LPDIRECT3D9 d3d = ::Direct3DCreate9(D3D_SDK_VERSION);
		if (!d3d)
			return false;

		PDWORD		D3DVTable = (PDWORD)*(PDWORD)d3d;
		d3d->Release();

		RealCreateDevice = (RealCreateDeviceFuncType)D3DVTable[CREATEDEVICE_INDEX];
		d3d9CreateDeviceHook = new Hook((void*)RealCreateDevice, (void*)*hkCreateDevice);
		d3d9CreateDeviceHook->Patch();
#endif

#if LIVEHOOK
		LPDIRECT3D9 p = ::Direct3DCreate9(D3D_SDK_VERSION);
		if (!p)
			return false;

		D3DPRESENT_PARAMETERS	presParams;
		ZeroMemory(&presParams, sizeof(presParams));
		presParams.Windowed = TRUE;
		presParams.SwapEffect = D3DSWAPEFFECT_DISCARD;

		IDirect3DDevice9	*pI = NULL;
		HRESULT	hr;
		HWND	hWnd = Utilities::getToplevelWindows();
		hr = p->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_NULLREF, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE, &presParams, &pI);

		if (!pI)
		{
			return false;
		}
		PVOID	*pVTable = (PVOID*)*((DWORD*)pI);

		RealPresent = (RealPresentFuncType)pVTable[PRESENT_INDEX];
		RealEndScene = (RealEndSceneFuncType)pVTable[ENDSCENE_INDEX];

		//Delete dummy device
		pI->Release();
		p->Release();

		//Install hook
		d3d9Hook = new Hook((void*)RealEndScene, (void*)*EndScene);
		d3d9Hook->Patch();
		Log::info("...successfully installed d3d9 hook.");
#endif
		return true;
	}

	long CanvasWidth()
	{
		return xwidth;
	}

	long CanvasHeight()
	{
		return xheight;
	}

	IDirect3DDevice9 *Device()
	{
		return d3d9Device;
	}
}
