#include "stdafx.h"
#include "CEGUIManager.h"
#include <d3d9.h>
#include "CEGUI/CEGUI.h"
#include "CEGUI\RendererModules\Direct3D9\Renderer.h"
#include "CEGUI\ScriptModules\Lua\ScriptModule.h"
#include "Log.h"
#include "Utilities.h"

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
		LuaScriptModule& script_module = LuaScriptModule::create();

		//Init system
		System::create(myRenderernderer, d_resourceProvider, 0, d_imageCodec, &script_module);

		//Log file
		Logger::getSingleton().setLoggingLevel(CEGUI::Informative);
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

		script_module.createBindings();

		//Create root window
		WindowManager& wmgr = WindowManager::getSingleton();
		rootWindow = wmgr.createWindow("DefaultWindow", "root");
		System::getSingleton().getDefaultGUIContext().setRootWindow(rootWindow);

		//Create GUI
		/*mainFrameWindow = static_cast<FrameWindow*>(wmgr.createWindow("TaharezLook/FrameWindow", "myWindow"));
		rootWindow->addChild(mainFrameWindow);
		// position a quarter of the way in from the top-left of parent.
		mainFrameWindow->setPosition(UVector2(UDim(0.25f, 0.0f), UDim(0.25f, 0.0f)));
		// set size to be half the size of the parent
		mainFrameWindow->setSize(USize(UDim(0.5f, 0.0f), UDim(0.5f, 0.0f)));
		mainFrameWindow->setText("Hello CEGUI!");*/

		Log::info("...CEGUI init done.");
		device = dev;
		return true;
	}

	void Render()
	{
		if (!device)
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
		LuaScriptModule *m = (LuaScriptModule*)System::getSingleton().getScriptingModule();
		m->executeScriptFile(path, ScriptModule::getDefaultResourceGroup());
	}

	void CleanUp()
	{

	}
}

