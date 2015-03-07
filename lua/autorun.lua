local logger = CEGUI.Logger:getSingleton()
logger:logEvent( ">>> Lua autorun says hello" )

local rootWin = CEGUI.System:getSingleton():getDefaultGUIContext():getRootWindow()
local winMgr = CEGUI.WindowManager:getSingleton()
local mainFrame = winMgr:createWindow("TaharezLook/FrameWindow", "myWindow")

rootWin:addChild(mainFrame)
mainFrame:setText("Bl2monitor")
mainFrame:setPosition(CEGUI.UVector2:new_local(CEGUI.UDim:new_local(0.1, 0.0), CEGUI.UDim:new_local(0.8, 0.0)))
mainFrame:setSize(CEGUI.USize:new_local(CEGUI.UDim:new_local(0.1, 0.0), CEGUI.UDim:new_local(0.1, 0.0)))

local button = winMgr:createWindow("TaharezLook/Button", "myButton")
button:setPosition(CEGUI.UVector2:new_local(CEGUI.UDim:new_local(0.1, 0.0), CEGUI.UDim:new_local(0.1, 0.0)))
button:setSize(CEGUI.USize:new_local(CEGUI.UDim:new_local(0.8, 0.0), CEGUI.UDim:new_local(0.8, 0.0)))
button:setText("Lua Loader")
button:subscribeEvent("Clicked", "lualoader_clicked")
mainFrame:addChild(button)

function lualoader_clicked(e)
   log.server("lualoader_clicked")
end
