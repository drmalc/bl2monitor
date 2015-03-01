#include "stdafx.h"
#include "CGwen.h"
#include "Log.h"
#include "Utilities.h"
#include "DX9Hook.h"
#pragma comment( lib, "gwen" )

#define CANVAS_W 300
#define CANVAS_H 200

namespace CGwen
{
	static IDirect3DDevice9 *device = NULL;
	static Gwen::Controls::Canvas* pCanvas = NULL;

	Gwen::Controls::Canvas* baseCanvas()
	{
		return pCanvas;
	}

	bool Initialize(IDirect3DDevice9 *dev)
	{
		if (device)
			return true;

		// Get skin path
		const char *path = Utilities::ImagesPath();
		char skinPath[MAX_PATH];
		strncpy_s(skinPath, path, sizeof(skinPath));
		strncat_s(skinPath, "DefaultSkin.png", sizeof(skinPath));

		// Create a GWEN DirectX renderer
		Gwen::Renderer::DirectX9* pRenderer = new Gwen::Renderer::DirectX9(dev);
		// Create a GWEN skin
		Gwen::Skin::TexturedBase* pSkin = new Gwen::Skin::TexturedBase(pRenderer);
		pSkin->Init(skinPath);

		// Create a Canvas (it's root, on which all other GWEN panels are created)
		pCanvas = new Gwen::Controls::Canvas(pSkin);
		pCanvas->SetSize(CANVAS_W, CANVAS_H);
		pCanvas->SetDrawBackground(true);
		pCanvas->SetBackgroundColor(Gwen::Color(150, 170, 170, 255));

		// Create a Windows Control helper
		// (Processes Windows MSG's and fires input at GWEN)
		Gwen::Input::Windows GwenInput;
		GwenInput.Initialize(pCanvas);

		// Create our unittest control (which is a Window with controls in it)
		/*using namespace Gwen;
		Controls::Label*	myControl;
		myControl = new Controls::Label(pCanvas);
		myControl->SetText("Hello Gwen");
		myControl->Dock(Pos::Bottom);*/

		device = dev;
		return true;
	}

	void Render()
	{
		if (!pCanvas)
			return;

		pCanvas->RenderCanvas();
	}
}

