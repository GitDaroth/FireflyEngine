add_subdirectory(3rdParty/spdlog)

set(headers 
	include/FireflyEngine.h
	include/EntryPoint.h
	include/Core.h
	include/Game.h
	include/Logger.h
	include/pch.h)

set(sources 
	src/Game.cpp
	src/Logger.cpp
	src/pch.cpp)

add_library(FireflyEngine SHARED ${sources} ${headers})
target_link_libraries(FireflyEngine PUBLIC spdlog::spdlog)
target_include_directories(FireflyEngine PUBLIC "include")
target_compile_definitions(FireflyEngine PRIVATE FIREFLY_ENGINE)
target_precompile_headers(FireflyEngine PRIVATE "include/pch.h")