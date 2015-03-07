#include "stdafx.h"
#include "CEGUIManager.h"
#include <d3d9.h>
#include "CEGUI/CEGUI.h"
#include "CEGUI\RendererModules\Direct3D9\Renderer.h"
#include "CEGUI\ScriptModules\Lua\ScriptModule.h"
#include "Log.h"
#include "Utilities.h"
#include "GameHooks.h"
#include "WillowGame_classes.h"
#include "UEventsConst.h"

#pragma comment(lib, "CEGUIBase-0.lib")
#pragma comment(lib, "CEGUIExpatParser.lib")
#pragma comment(lib, "CEGUICoreWindowRendererSet.lib")
#pragma comment(lib, "CEGUIDirect3D9Renderer-0.lib")
#pragma comment(lib, "CEGUILuaScriptModule-0.lib")

namespace CEGUIManager
{
	static IDirect3DDevice9 *device = NULL;

	using namespace CEGUI;
	static Window *rootWindow = NULL;
	static FrameWindow *mainFrameWindow = NULL;
	static ImageCodec* d_imageCodec = NULL;
	static ResourceProvider* d_resourceProvider = NULL;

	static bool InputKey(UObject* caller, UFunction* function, void* parms, void* result)
	{
		if (!rootWindow->isVisible())
			return true;

		UWillowGameViewportClient_execInputKey_Parms* realParms = reinterpret_cast<UWillowGameViewportClient_execInputKey_Parms*>(parms);

		if (realParms->EventType == (int)BUTTON_STATE_Pressed)
		{
			const char* name = realParms->Key.GetName();
			//Log::info("InputKey %d: %s", realParms->EventType, name);

			if (!strcmp(name, KEYEVENT_MOUSELEFT))
			{
				bool b = !CEGUI::System::getSingleton().getDefaultGUIContext().injectMouseButtonDown(CEGUI::LeftButton);
				//Log::info("MouseDown %d", (int)b);
				//always returns false.. why ?
				return b;
			}
			else if (!strcmp(name, KEYEVENT_MOUSERIGHT))
			{
				return !CEGUI::System::getSingleton().getDefaultGUIContext().injectMouseButtonDown(CEGUI::RightButton);
			}
			else if (name[1] == 0)
			{
				//ASCII char
				return !CEGUI::System::getSingleton().getDefaultGUIContext().injectChar(*name);
			}
		}
		else if (realParms->EventType == (int)BUTTON_STATE_Released)
		{
			const char* name = realParms->Key.GetName();

			if (!strcmp(name, KEYEVENT_MOUSELEFT))
			{
				return !CEGUI::System::getSingleton().getDefaultGUIContext().injectMouseButtonUp(CEGUI::LeftButton);
			}
			else if (!strcmp(name, KEYEVENT_MOUSERIGHT))
			{
				return !CEGUI::System::getSingleton().getDefaultGUIContext().injectMouseButtonUp(CEGUI::RightButton);
			}
		}
		/*
		if (bButtonDown)
		{
			CEGUI::System::getSingleton().getDefaultGUIContext().injectKeyDown((CEGUI::Key::Scan)inKey.key);
			CEGUI::System::getSingleton().getDefaultGUIContext().injectChar(inKey.text);
		}
		else
		{
			CEGUI::System::getSingleton().getDefaultGUIContext().injectKeyUp((CEGUI::Key::Scan)inKey.key);
		}
		*/
		return true;
	}

	static bool InputAxis(UObject* caller, UFunction* function, void* parms, void* result)
	{
		if (!rootWindow->isVisible())
			return true;

		POINT p;
		if (::GetCursorPos(&p))
		{
			if (::ScreenToClient(Utilities::getToplevelWindows(), &p))
			{
				CEGUI::System::getSingleton().getDefaultGUIContext().injectMousePosition((float)p.x, (float)p.y);
			}
		}
		return true;
	}

	bool Initialize(IDirect3DDevice9 *dev, lua_State* luaState)
	{
		if (device)
			return true;

		//Get resources path
		const char *basePath1 = Utilities::LayoutPath();
		char basePath[MAX_PATH] = { 0 };
		char buffer[MAX_PATH] = { 0 };
		ZeroMemory(basePath, MAX_PATH*sizeof(char));
		ZeroMemory(buffer, MAX_PATH*sizeof(char));
		const char *c = basePath1;
		char *c2 = basePath;
		//Convert \ to /
		while (*c)
		{
			*c2 = *c == '\\' ? '/' : *c;
			c2++;
			c++;
		}
		Log::info("CEGUI init (Path is %s)", basePath);

		//Create renderer
		Direct3D9Renderer &myRenderernderer = CEGUI::Direct3D9Renderer::create(dev);

		//Create lua module
		LuaScriptModule& script_module = LuaScriptModule::create(luaState);

		//Init system
		System::create(myRenderernderer, d_resourceProvider, 0, d_imageCodec, &script_module);

		//Log file
		Logger::getSingleton().setLoggingLevel(CEGUI::Warnings);
		sprintf_s(buffer, "%s/../log/cegui.log", basePath);
		CEGUI::Logger::getSingleton().setLogFilename(buffer);

		//Setup default paths
		DefaultResourceProvider* rp = static_cast<DefaultResourceProvider*>(System::getSingleton().getResourceProvider());
		sprintf_s(buffer, "%s/%s/", basePath, "imagesets");
		rp->setResourceGroupDirectory("imagesets", buffer);
		sprintf_s(buffer, "%s/%s/", basePath, "fonts");
		rp->setResourceGroupDirectory("fonts", buffer);
		sprintf_s(buffer, "%s/%s/", basePath, "layouts");
		rp->setResourceGroupDirectory("layouts", buffer);
		sprintf_s(buffer, "%s/%s/", basePath, "looknfeel");
		rp->setResourceGroupDirectory("looknfeel", buffer);
		sprintf_s(buffer, "%s/%s/", basePath, "schemes");
		rp->setResourceGroupDirectory("schemes", buffer);
		sprintf_s(buffer, "%s/%s/", basePath, "xml_schemas");
		rp->setResourceGroupDirectory("xml_schemas", buffer);
		sprintf_s(buffer, "%s/../%s/", basePath, "lua");
		rp->setResourceGroupDirectory("lua_scripts", buffer);

		//Setup default groups
		ImageManager::setImagesetDefaultResourceGroup("imagesets");
		Font::setDefaultResourceGroup("fonts");
		Scheme::setDefaultResourceGroup("schemes");
		WidgetLookManager::setDefaultResourceGroup("looknfeel");
		WindowManager::setDefaultResourceGroup("layouts");
		ScriptModule::setDefaultResourceGroup("lua_scripts");
		AnimationManager::setDefaultResourceGroup("animations");

		//Setup default group for validation schemas
		CEGUI::XMLParser* parser = CEGUI::System::getSingleton().getXMLParser();
		if (parser->isPropertyPresent("SchemaDefaultResourceGroup"))
			parser->setProperty("SchemaDefaultResourceGroup", "schemas");

		//Register resources
		SchemeManager::getSingleton().createFromFile("TaharezLook.scheme");
		FontManager::getSingleton().createFromFile("DejaVuSans-10.font");

		//Setup some defaults
		System::getSingleton().getDefaultGUIContext().getMouseCursor().setDefaultImage("TaharezLook/MouseArrow");
		System::getSingleton().getDefaultGUIContext().setDefaultTooltipType("TaharezLook/Tooltip");
		System::getSingleton().getDefaultGUIContext().getMouseCursor().hide();

		script_module.createBindings();

		//Create root window
		WindowManager& wmgr = WindowManager::getSingleton();
		rootWindow = wmgr.createWindow("DefaultWindow", "root");
		System::getSingleton().getDefaultGUIContext().setRootWindow(rootWindow);

		//Setup IOs
		GameHooks::EngineHookManager->Register("Function WillowGame.WillowGameViewportClient:InputKey", "CEGUIInputKey", &InputKey);
		GameHooks::EngineHookManager->Register("Function WillowGame.WillowGameViewportClient:InputAxis", "CEGUIInputAxis", &InputAxis);

		Log::info("...CEGUI init done.");
		device = dev;
		return true;
	}

	void Render()
	{
		if (!device)
			return;
		if (!rootWindow->isVisible())
			return;
		static bool isInit = false;
		if (!isInit){
			Log::info("CEGUI is rendering for the first time...");
		}
		CEGUI::System::getSingleton().renderAllGUIContexts();

		if (!isInit){
			Log::info("...CEGUI is done rendering for the first time.");
			isInit = true;
		}
	}

	void RunLua(const char*path)
	{
		CEGUI::System::getSingleton().executeScriptFile(path, ScriptModule::getDefaultResourceGroup());
	}

	void RunLua(const std::string &path)
	{
		CEGUI::System::getSingleton().executeScriptFile(path, ScriptModule::getDefaultResourceGroup());
	}

	void CleanUp()
	{

	}
}

