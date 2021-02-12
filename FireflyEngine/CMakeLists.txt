set(BUILD_STATIC_LIBS ON)

add_subdirectory(3rdParty/spdlog)
add_subdirectory(3rdParty/GLFW)
add_subdirectory(3rdParty/glad)
add_subdirectory(3rdParty/glm)
add_subdirectory(3rdParty/imgui)
add_subdirectory(3rdParty/stb_image)
add_subdirectory(3rdParty/assimp)
add_subdirectory(3rdParty/EnTT)
add_subdirectory(3rdParty/SPIRV-Reflect)
find_package(Vulkan REQUIRED)

set_target_properties(spdlog glad glm_static imgui stb_image spirv_reflect PROPERTIES FOLDER 3rdParty)
set_target_properties(glfw uninstall PROPERTIES FOLDER 3rdParty/GLFW)
set_target_properties(assimp assimp_cmd IrrXML UpdateAssimpLibsDebugSymbolsAndDLLs zlib zlibstatic PROPERTIES FOLDER 3rdParty/assimp)

set(miscFiles
    include/Firefly/FireflyEngine.h
    include/Firefly/pch.h
    src/pch.cpp)

set(coreFiles
    include/Firefly/Core/Engine.h
    src/Core/Engine.cpp
    include/Firefly/Core/EntryPoint.h
    src/Core/EntryPoint.cpp
    include/Firefly/Core/Core.h
    include/Firefly/Core/Application.h
    src/Core/Application.cpp
    include/Firefly/Core/Logger.h
    src/Core/Logger.cpp
    include/Firefly/Core/ResourceRegistry.h)

set(windowFiles
    include/Firefly/Window/Window.h
    src/Window/Window.cpp
    include/Firefly/Window/WindowsWindow.h
    src/Window/WindowsWindow.cpp)

set(renderingFiles
    include/Firefly/Rendering/Material.h
    src/Rendering/Material.cpp
    include/Firefly/Rendering/Mesh.h
    src/Rendering/Mesh.cpp
    include/Firefly/Rendering/MeshGenerator.h
    src/Rendering/MeshGenerator.cpp
    include/Firefly/Rendering/RenderingAPI.h
    src/Rendering/RenderingAPI.cpp
    include/Firefly/Rendering/Renderer.h
    include/Firefly/Rendering/GraphicsContext.h
    src/Rendering/GraphicsContext.cpp
    include/Firefly/Rendering/Shader.h
    src/Rendering/Shader.cpp
    include/Firefly/Rendering/Texture.h
    src/Rendering/Texture.cpp
    include/Firefly/Rendering/FrameBuffer.h
    src/Rendering/FrameBuffer.cpp
    include/Firefly/Rendering/RenderPass.h
    src/Rendering/RenderPass.cpp)

set(renderingOpenGLFiles
    include/Firefly/Rendering/OpenGL/OpenGLContext.h
    src/Rendering/OpenGL/OpenGLContext.cpp
    include/Firefly/Rendering/OpenGL/OpenGLShader.h
    src/Rendering/OpenGL/OpenGLShader.cpp
    include/Firefly/Rendering/OpenGL/OpenGLTexture.h
    src/Rendering/OpenGL/OpenGLTexture.cpp
    include/Firefly/Rendering/OpenGL/OpenGLMesh.h
    src/Rendering/OpenGL/OpenGLMesh.cpp
    include/Firefly/Rendering/OpenGL/OpenGLMaterial.h
    src/Rendering/OpenGL/OpenGLMaterial.cpp
    include/Firefly/Rendering/OpenGL/OpenGLRenderer.h
    src/Rendering/OpenGL/OpenGLRenderer.cpp
    include/Firefly/Rendering/OpenGL/OpenGLFrameBuffer.h
    src/Rendering/OpenGL/OpenGLFrameBuffer.cpp
    include/Firefly/Rendering/OpenGL/OpenGLRenderPass.h
    src/Rendering/OpenGL/OpenGLRenderPass.cpp)

set(renderingVulkanFiles
    include/Firefly/Rendering/Vulkan/VulkanContext.h
    src/Rendering/Vulkan/VulkanContext.cpp
    include/Firefly/Rendering/Vulkan/VulkanDevice.h
    src/Rendering/Vulkan/VulkanDevice.cpp
    include/Firefly/Rendering/Vulkan/VulkanSwapchain.h
    src/Rendering/Vulkan/VulkanSwapchain.cpp
    include/Firefly/Rendering/Vulkan/VulkanMesh.h
    src/Rendering/Vulkan/VulkanMesh.cpp
    include/Firefly/Rendering/Vulkan/VulkanShader.h
    src/Rendering/Vulkan/VulkanShader.cpp
    include/Firefly/Rendering/Vulkan/VulkanMaterial.h
    src/Rendering/Vulkan/VulkanMaterial.cpp
    include/Firefly/Rendering/Vulkan/VulkanTexture.h
    src/Rendering/Vulkan/VulkanTexture.cpp
    include/Firefly/Rendering/Vulkan/VulkanUtils.h
    src/Rendering/Vulkan/VulkanUtils.cpp
    include/Firefly/Rendering/Vulkan/VulkanRenderer.h
    src/Rendering/Vulkan/VulkanRenderer.cpp
    include/Firefly/Rendering/Vulkan/VulkanFrameBuffer.h
    src/Rendering/Vulkan/VulkanFrameBuffer.cpp
    include/Firefly/Rendering/Vulkan/VulkanRenderPass.h
    src/Rendering/Vulkan/VulkanRenderPass.cpp)

set(sceneFiles
    include/Firefly/Scene/Scene.h
    src/Scene/Scene.cpp
    include/Firefly/Scene/Entity.h
    src/Scene/Entity.cpp
    include/Firefly/Scene/Camera.h
    src/Scene/Camera.cpp)

set(entityComponentFiles
    include/Firefly/Scene/Components/Component.h
    include/Firefly/Scene/Components/TransformComponent.h
    include/Firefly/Scene/Components/MeshComponent.h
    include/Firefly/Scene/Components/MaterialComponent.h
    include/Firefly/Scene/Components/TagComponent.h)

set(eventFiles
    include/Firefly/Event/Event.h
    include/Firefly/Event/MouseEvent.h
    include/Firefly/Event/KeyEvent.h
    include/Firefly/Event/GamepadEvent.h
    include/Firefly/Event/WindowEvent.h)

set(inputFiles
    include/Firefly/Input/KeyCodes.h
    include/Firefly/Input/MouseButtonCodes.h
    include/Firefly/Input/GamepadButtonCodes.h
    include/Firefly/Input/Gamepad.h
    src/Input/Gamepad.cpp
    include/Firefly/Input/Input.h
    src/Input/Input.cpp)

add_library(FireflyEngine STATIC 
    ${miscFiles}
    ${coreFiles} 
    ${windowFiles} 
    ${renderingFiles} 
    ${renderingOpenGLFiles} 
    ${renderingVulkanFiles}
    ${sceneFiles} 
    ${entityComponentFiles}
    ${eventFiles} 
    ${inputFiles})

source_group("" FILES ${miscFiles})
source_group("\\Core" FILES ${coreFiles})
source_group("\\Window" FILES ${windowFiles})
source_group("\\Rendering" FILES ${renderingFiles})
source_group("\\Rendering\\OpenGL" FILES ${renderingOpenGLFiles})
source_group("\\Rendering\\Vulkan" FILES ${renderingVulkanFiles})
source_group("\\Scene" FILES ${sceneFiles})
source_group("\\Scene\\Components" FILES ${entityComponentFiles})
source_group("\\Event" FILES ${eventFiles})
source_group("\\Input" FILES ${inputFiles})

target_link_libraries(FireflyEngine 
    PUBLIC
        spdlog
        glm_static
        entt
    PRIVATE
        glfw
        glad
        Vulkan::Vulkan
        imgui
        stb_image
        assimp
        spirv_reflect)

target_include_directories(FireflyEngine PUBLIC "include/Firefly" "${CMAKE_CURRENT_BINARY_DIR}" INTERFACE "include")
target_precompile_headers(FireflyEngine PRIVATE "include/Firefly/pch.h")

target_compile_features(FireflyEngine PUBLIC cxx_std_17)
target_compile_definitions(FireflyEngine PUBLIC GLFW_INCLUDE_NONE "$<$<CONFIG:DEBUG>:DEBUG>")

set(FIREFLY_ENGINE_NAME "Firefly Engine")

if(WIN32)
    set(FIREFLY_OS_WINDOWS ON)
endif()

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/EngineConfig.h.template"
    "${CMAKE_CURRENT_BINARY_DIR}/EngineConfig.h"
    @ONLY
)