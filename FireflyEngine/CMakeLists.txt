set(BUILD_STATIC_LIBS ON)

add_subdirectory(3rdParty/spdlog)
add_subdirectory(3rdParty/GLFW)
add_subdirectory(3rdParty/glad)
add_subdirectory(3rdParty/glm)
add_subdirectory(3rdParty/imgui)

set_target_properties(spdlog glad glm_static imgui PROPERTIES FOLDER 3rdParty)
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

set(renderingFiles
	include/Rendering/RenderingAPI.h
	src/Rendering/RenderingAPI.cpp
	include/Rendering/Renderer.h
	src/Rendering/Renderer.cpp
	include/Rendering/GraphicsContext.h
	include/Rendering/Shader.h
	include/Rendering/VertexBuffer.h
	include/Rendering/IndexBuffer.h)

set(renderingOpenGLFiles
	include/Rendering/OpenGL/OpenGLContext.h
	src/Rendering/OpenGL/OpenGLContext.cpp
	include/Rendering/OpenGL/OpenGLShader.h
	src/Rendering/OpenGL/OpenGLShader.cpp
	include/Rendering/OpenGL/OpenGLVertexBuffer.h
	src/Rendering/OpenGL/OpenGLVertexBuffer.cpp
	include/Rendering/OpenGL/OpenGLIndexBuffer.h
	src/Rendering/OpenGL/OpenGLIndexBuffer.cpp)

set(eventFiles
	include/Event/Event.h
	include/Event/MouseEvent.h
	include/Event/KeyEvent.h
	include/Event/WindowEvent.h)

set(inputFiles
	include/Input/KeyCodes.h
	include/Input/MouseButtonCodes.h
	include/Input/Input.h
	src/Input/Input.cpp)

add_library(FireflyEngine STATIC 
	${coreFiles} 
	${windowFiles} 
	${renderingFiles} 
	${renderingOpenGLFiles} 
	${eventFiles} 
	${inputFiles})

source_group("" FILES ${coreFiles})
source_group("\\Window" FILES ${windowFiles})
source_group("\\Rendering" FILES ${renderingFiles})
source_group("\\Rendering\\OpenGL" FILES ${renderingOpenGLFiles})
source_group("\\Event" FILES ${eventFiles})
source_group("\\Input" FILES ${inputFiles})


target_link_libraries(FireflyEngine 
	PUBLIC
		spdlog
		glm_static
	PRIVATE
		glfw
		glad
		imgui)

target_include_directories(FireflyEngine PUBLIC "include")
target_precompile_headers(FireflyEngine PRIVATE "include/pch.h")
target_compile_definitions(FireflyEngine PRIVATE GLFW_INCLUDE_NONE)