add_subdirectory(3rdParty/spdlog)
add_subdirectory(3rdParty/GLFW)

set(coreFiles
	include/FireflyEngine.h
	include/EntryPoint.h
	include/Core.h
	include/Game.h
	src/Game.cpp
	include/Logger.h
	src/Logger.cpp
	include/pch.h
	src/pch.cpp)

set(windowFiles
	include/Window/Window.h
	src/Window/Window.cpp
	include/Window/WindowsWindow.h
	src/Window/WindowsWindow.cpp)

set(eventFiles
	include/Event/Event.h
	include/Event/MouseEvent.h
	include/Event/KeyEvent.h
	include/Event/WindowEvent.h)

add_library(FireflyEngine SHARED ${coreFiles} ${windowFiles} ${eventFiles})

source_group("" FILES ${coreFiles})
source_group("\\Window" FILES ${windowFiles})
source_group("\\Event" FILES ${eventFiles})

target_link_libraries(FireflyEngine 
	PUBLIC
		spdlog::spdlog
		glfw)

target_include_directories(FireflyEngine PUBLIC "include")
target_compile_definitions(FireflyEngine PRIVATE FIREFLY_ENGINE)
target_precompile_headers(FireflyEngine PRIVATE "include/pch.h")