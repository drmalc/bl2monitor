
function initialize()
	log.server("Loading lua components")
	local path = luaModPath()
	log.server("Lua mod path is: "..path)
end

function cleanup()
	log.server("Cleaning up lua components")
end
