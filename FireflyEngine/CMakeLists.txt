add_subdirectory(3rdParty/spdlog)

set(headers 
	include/FireflyEngine.h
	include/EntryPoint.h
	include/Core.h
	include/Game.h
	include/Logger.h)

set(sources 
	src/Game.cpp
	src/Logger.cpp)

add_library(FireflyEngine SHARED ${sources} ${headers})
target_link_libraries(FireflyEngine PUBLIC spdlog::spdlog)
target_include_directories(FireflyEngine PUBLIC "include")
target_compile_definitions(FireflyEngine PRIVATE FIREFLY_ENGINE)