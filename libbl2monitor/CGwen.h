#pragma once
#include <d3d9.h>

#include "Gwen/Gwen.h"
#include "Gwen/Skins/Simple.h"
#include "Gwen/Skins/TexturedBase.h"
#include "Gwen/UnitTest/UnitTest.h"
#include "Gwen/Input/Windows.h"
#include "Gwen/Renderers/DirectX9.h"

namespace CGwen
{
	bool Initialize(IDirect3DDevice9 *dev);
	void Render();
	Gwen::Controls::Canvas* baseCanvas();
}

