local rootCanvas_l
local mainLabel_l
local text_l

function initialize()
	log.server("Loading lua components")
	
	rootCanvas_l = baseCanvas()
	mainLabel_l = Label(rootCanvas_l)
	text_l = TextObject("Hello from LUA")
	mainLabel_l:SetText(text_l)
	mainLabel_l:Dock(Base.Top)
end

function cleanup()
	log.server("Cleaning up lua components")
end
