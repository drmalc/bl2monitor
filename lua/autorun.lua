local logger = CEGUI.Logger:getSingleton()
logger:logEvent( ">>> Lua autorun says hello." )
log.server("Lua autorun init.")

local rootWin = CEGUI.System:getSingleton():getDefaultGUIContext():getRootWindow()
local winMgr = CEGUI.WindowManager:getSingleton()
local mainFrame = winMgr:createWindow("TaharezLook/FrameWindow", "myWindow")
local loaded_modules = {}

rootWin:addChild(mainFrame)
mainFrame:setText("Bl2monitor - F1 to hide/show")
mainFrame:setPosition(CEGUI.UVector2:new_local(CEGUI.UDim:new_local(0.2, 0.0), CEGUI.UDim:new_local(0.6, 0.0)))
mainFrame:setSize(CEGUI.USize:new_local(CEGUI.UDim:new_local(0.3, 0.0), CEGUI.UDim:new_local(0.3, 0.0)))

--cant get layouts to work
--local layout = winMgr:createWindow("VerticalLayoutContainer", "vLayout")
--layout:setPosition(CEGUI.UVector2:new_local(CEGUI.UDim:new_local(0.1, 0.0), CEGUI.UDim:new_local(0.1, 0.0)))
--layout:setSize(CEGUI.USize:new_local(CEGUI.UDim:new_local(0.5, 0.0), CEGUI.UDim:new_local(0.5, 0.0)))
--mainFrame:addChild(layout)

local loadButton = winMgr:createWindow("TaharezLook/Button", "loadButton")
loadButton:setPosition(CEGUI.UVector2:new_local(CEGUI.UDim:new_local(0.1, 0.0), CEGUI.UDim:new_local(0.05, 0.0)))
loadButton:setSize(CEGUI.USize:new_local(CEGUI.UDim:new_local(0.4, 0.0), CEGUI.UDim:new_local(0.12, 0.0)))
loadButton:setText("Load Selected")
loadButton:subscribeEvent("Clicked", "lualoader_clicked")
mainFrame:addChild(loadButton)

local label = winMgr:createWindow("TaharezLook/Label", "infoLabel")
label:setPosition(CEGUI.UVector2:new_local(CEGUI.UDim:new_local(0.5, 0.0), CEGUI.UDim:new_local(0.05, 0.0)))
label:setSize(CEGUI.USize:new_local(CEGUI.UDim:new_local(0.4, 0.0), CEGUI.UDim:new_local(0.12, 0.0)))
label:setText("F2: reset lua")
mainFrame:addChild(label)

local listbox = winMgr:createWindow("TaharezLook/Listbox", "ModList");
listbox:setPosition(CEGUI.UVector2:new_local(CEGUI.UDim:new_local(0.1, 0.0), CEGUI.UDim:new_local(0.3, 0.0)))
listbox:setSize(CEGUI.USize:new_local(CEGUI.UDim:new_local(0.8, 0.0), CEGUI.UDim:new_local(0.65, 0.0)))
mainFrame:addChild(listbox)
listbox = CEGUI.toListbox(listbox)

function toggle_visibility()
	if mainFrame:isVisible() then
		mainFrame:hide()
	else
		mainFrame:show()
	end
end

function populate_list()
	local baseDir = luaModPath().."/"
	lfs.chdir(baseDir)
	
	for file in lfs.dir(".") do
		local fname = baseDir..file.."/info.txt" --relative paths not working ?
		if lfs.attributes(fname, "mode") == "file" then
			--local file = io.open(fname, "r");
			--local arr = {}
			--for line in file:lines() do
			--	table.insert(arr, line);
			--end
			--io.close(file)
			local item = CEGUI.createListboxTextItem(file, 0, nil, false, true)
			local c1 = CEGUI.Colour:new_local(0.97, 0.91, 0.01, 0.5)
			local c2 = CEGUI.Colour:new_local(1.0, 0.64, 0.0, 0.5)
			item:setSelectionColours(c1, c1, c2, c2)
			item:setSelectionBrushImage("TaharezLook/ListboxSelectionBrush")
			listbox:addItem(item)
		end
	end
end

function lualoader_clicked(e)
	local item = listbox:getFirstSelectedItem()
	if item == nil then
		return
	end
	local baseDir = luaModPath().."/"
	lfs.chdir(baseDir)
	
	local name = item:getText()
	for i,v in ipairs(loaded_modules) do
		if name == v then
			return
		end
	end
	log.server("Loading Lua mod: "..name)
	loaded_file = assert(loadfile(name.."/main.lua"))
	loaded_file()
	assert(loadstring(name..".initialize()"))()
	table.insert(loaded_modules, name)
end

function clean_module(index)
	assert(loadstring(loaded_modules[index]..".cleanup()"))()
end

function cleanup()
	log.server("autorun cleanup")
	table.foreach(loaded_modules, clean_module)
	loaded_modules = {}
	removeAllEngineHooks()
	removeAllPrerenderCallbacks()
end

populate_list()
