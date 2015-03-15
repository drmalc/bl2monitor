local logger = CEGUI.Logger:getSingleton()
logger:logEvent( ">>> Lua autorun says hello." )
log.server("Lua autorun init.")

local rootWin = CEGUI.System:getSingleton():getDefaultGUIContext():getRootWindow()
local winMgr = CEGUI.WindowManager:getSingleton()
local mainFrame = winMgr:createWindow("TaharezLook/FrameWindow", "myWindow")

rootWin:addChild(mainFrame)
mainFrame:setText("Bl2monitor")
mainFrame:setPosition(CEGUI.UVector2:new_local(CEGUI.UDim:new_local(0.2, 0.0), CEGUI.UDim:new_local(0.6, 0.0)))
mainFrame:setSize(CEGUI.USize:new_local(CEGUI.UDim:new_local(0.3, 0.0), CEGUI.UDim:new_local(0.3, 0.0)))

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
label:setText("F1 to reset lua")
mainFrame:addChild(label)

local listbox = winMgr:createWindow("TaharezLook/Listbox", "ModList");
listbox:setPosition(CEGUI.UVector2:new_local(CEGUI.UDim:new_local(0.1, 0.0), CEGUI.UDim:new_local(0.3, 0.0)))
listbox:setSize(CEGUI.USize:new_local(CEGUI.UDim:new_local(0.8, 0.0), CEGUI.UDim:new_local(0.65, 0.0)))
mainFrame:addChild(listbox)

function populate_list()
	local baseDir = luaPath()
	lfs.chdir(baseDir)
	
	for file in lfs.dir("mods") do
		if lfs.attributes(baseDir.."/mods/"..file.."/main.lua", "mode") == "file" then --relative paths not working ?
			local item = CEGUI.createListboxTextItem(file, 0, nil, false, true)
			item:setSelectionColours(CEGUI.Colour:new_local(0.2, 0.2, 1, 1))
			CEGUI.toListbox(listbox):addItem(item)
		end
	end
end

function lualoader_clicked(e)
   log.server("lualoader_clicked")
end

function cleanup()
	log.server("autorun cleanup")
end

populate_list()
