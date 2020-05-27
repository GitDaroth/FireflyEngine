add_subdirectory(3rdParty/spdlog)
add_subdirectory(3rdParty/GLFW)

set(headers 
	include/Window/Window.h
	include/Window/WindowsWindow.h
	include/FireflyEngine.h
	include/EntryPoint.h
	include/Core.h
	include/Game.h
	include/Logger.h
	include/pch.h)

set(sources 
	src/Window/Window.cpp
	src/Window/WindowsWindow.cpp
	src/Game.cpp
	src/Logger.cpp
	src/pch.cpp)

add_library(FireflyEngine SHARED ${sources} ${headers})

target_link_libraries(FireflyEngine 
	PUBLIC
		spdlog::spdlog
		glfw)

target_include_directories(FireflyEngine PUBLIC "include")
target_compile_definitions(FireflyEngine PRIVATE FIREFLY_ENGINE)
target_precompile_headers(FireflyEngine PRIVATE "include/pch.h")