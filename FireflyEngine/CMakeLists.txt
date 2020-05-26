set(headers 
	src/FireflyEngine.h
	src/EntryPoint.h
	src/Core.h
	src/Game.h)

set(sources 
	src/Game.cpp)

add_library(FireflyEngine SHARED ${sources} ${headers})
target_compile_definitions(FireflyEngine PRIVATE FIREFLY_ENGINE)