add_subdirectory(3rdParty/spdlog)
add_subdirectory(3rdParty/GLFW)
add_subdirectory(3rdParty/glad)
add_subdirectory(3rdParty/imgui)

set_target_properties(spdlog glad imgui PROPERTIES FOLDER 3rdParty)
set_target_properties(glfw uninstall PROPERTIES FOLDER 3rdParty/GLFW)

set(coreFiles
	include/FireflyEngine.h
	include/EntryPoint.h
	include/Core.h
	include/ImGuiLayer.h
	src/ImGuiLayer.cpp
	include/Layer.h
	src/Layer.cpp
	include/Application.h
	src/Application.cpp
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

set(inputFiles
	include/Input/Input.h
	include/Input/KeyCodes.h
	include/Input/MouseButtonCodes.h
	src/Input/Input.cpp)

add_library(FireflyEngine SHARED ${coreFiles} ${windowFiles} ${eventFiles} ${inputFiles})

source_group("" FILES ${coreFiles})
source_group("\\Window" FILES ${windowFiles})
source_group("\\Event" FILES ${eventFiles})
source_group("\\Input" FILES ${inputFiles})


target_link_libraries(FireflyEngine 
	PUBLIC
		spdlog
		glfw
		glad
		imgui)

target_include_directories(FireflyEngine PUBLIC "include")
target_precompile_headers(FireflyEngine PRIVATE "include/pch.h")

target_compile_definitions(FireflyEngine 
	PRIVATE 
		FIREFLY_ENGINE 
		GLFW_INCLUDE_NONE)