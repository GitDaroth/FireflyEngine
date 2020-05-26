add_subdirectory(3rdParty/spdlog)

set(headers 
	src/FireflyEngine.h
	src/EntryPoint.h
	src/Core.h
	src/Game.h
	src/Logger.h)

set(sources 
	src/Game.cpp
	src/Logger.cpp)

add_library(FireflyEngine SHARED ${sources} ${headers})
target_link_libraries(FireflyEngine PUBLIC spdlog::spdlog)
target_include_directories(FireflyEngine PUBLIC "src")
target_compile_definitions(FireflyEngine PRIVATE FIREFLY_ENGINE)