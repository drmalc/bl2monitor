
function hello.initialize()
	log.server("Loading lua components")
	local path = luaModPath()
	log.server("Lua mod path is: "..path)
	
	local a = TestClass(42)
	a:PrintSomething()
end

function hello.cleanup()
	log.server("Cleaning up lua components")
end
