local P = {}
local modname = "bosstimer"
bosstimer = P

local startTime = nil

function P.BossbarStatusUpdate()
	if startTime == nil then
		startTime = os.clock()
	else
		startTime = nil
	end
end

function P.PrerenderCallback()
	if startTime == nil then
		return
	end
	local now = os.clock()-startTime
	local minutes = math.floor(now/60.0)
	local seconds = now - (minutes*60.0)
	local secs = math.floor(seconds)
	P.label:setText(string.format("[font='DejaVuSans-28']%02d:%02d.%02d", minutes, secs, (seconds-secs)*100))
end

function P.initialize()
	log.server(modname.." is loading")
	local rootWin = CEGUI.System:getSingleton():getDefaultGUIContext():getRootWindow()
	local winMgr = CEGUI.WindowManager:getSingleton()
	
	P.label = winMgr:createWindow("TaharezLook/Label", "timerLabel")
	P.label:setPosition(CEGUI.UVector2:new_local(CEGUI.UDim:new_local(0.0, 0.0), CEGUI.UDim:new_local(0.0, 0.0)))
	P.label:setSize(CEGUI.USize:new_local(CEGUI.UDim:new_local(0.15, 0.0), CEGUI.UDim:new_local(0.1, 0.0)))
	P.label:setText("[font='DejaVuSans-28']00:00.00")
	rootWin:addChild(P.label)
	
	addEngineHook("Function WillowGame.WillowSeqAct_BossBar:Activated", modname..".BossbarStatusUpdate")
	addPrerenderCallback(modname..".PrerenderCallback")
end

function P.cleanup()
	log.server(modname.." cleanup")
	removeEngineHook(modname..".BossbarStatusUpdate")
	removePrerenderCallback(modname..".PrerenderCallback")
end

return P