#pragma once

struct IDirect3DDevice9;
namespace CEGUIManager
{
	bool Initialize(IDirect3DDevice9 *dev);
	void CleanUp();
	void Render();
}

