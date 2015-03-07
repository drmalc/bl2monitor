-- log.server("Loading autorun")
local logger = CEGUI.Logger:getSingleton()
-- logger:logEvent( ">>> Init script says hello" )

local rootWin = CEGUI.System:getSingleton():getDefaultGUIContext():getRootWindow()
local winMgr = CEGUI.WindowManager:getSingleton()
local mainFrame = winMgr:createWindow("TaharezLook/FrameWindow", "myWindow")

rootWin:addChild(mainFrame)
mainFrame:setText("Hello CEGUI from Lua!")
mainFrame:setPosition(CEGUI.UVector2(CEGUI.UDim(0.25, 0.0), CEGUI.UDim(0.25, 0.0)))
mainFrame:setSize(CEGUI.USize(CEGUI.UDim(0.5, 0.0), CEGUI.UDim(0.5, 0.0)))
