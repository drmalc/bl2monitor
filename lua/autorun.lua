-- log.server("Loading autorun")
local logger = CEGUI.Logger:getSingleton()
logger:logEvent( ">>> Lua autorun says hello" )

local rootWin = CEGUI.System:getSingleton():getDefaultGUIContext():getRootWindow()
local winMgr = CEGUI.WindowManager:getSingleton()
local mainFrame = winMgr:createWindow("TaharezLook/FrameWindow", "myWindow")

rootWin:addChild(mainFrame)
mainFrame:setText("Hello CEGUI from Lua!")
mainFrame:setPosition(CEGUI.UVector2(CEGUI.UDim(0.25, 0.0), CEGUI.UDim(0.25, 0.0)))
mainFrame:setSize(CEGUI.USize(CEGUI.UDim(0.5, 0.0), CEGUI.UDim(0.5, 0.0)))

local button = winMgr:createWindow("TaharezLook/Button", "myButton")
button:setPosition(CEGUI.UVector2(CEGUI.UDim(0.1, 0.0), CEGUI.UDim(0.1, 0.0)))
button:setSize(CEGUI.USize(CEGUI.UDim(0.1, 0.0), CEGUI.UDim(0.1, 0.0)))
button:setText("Button 1")
mainFrame:addChild(button)
