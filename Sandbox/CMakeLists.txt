set(sources 
	src/main.cpp
	include/SandboxApp.h
	src/SandboxApp.cpp
	include/CameraController.h
	src/CameraController.cpp)

set(openGLShaders 
	assets/shaders/OpenGL/pbr.vert
	assets/shaders/OpenGL/pbr.frag
	assets/shaders/OpenGL/hdrImageToCubeMap.vert
	assets/shaders/OpenGL/hdrImageToCubeMap.frag
	assets/shaders/OpenGL/irradianceCubeMap.vert
	assets/shaders/OpenGL/irradianceCubeMap.frag
	assets/shaders/OpenGL/environmentCubeMap.vert
	assets/shaders/OpenGL/environmentCubeMap.frag
	assets/shaders/OpenGL/prefilterCubeMap.vert
	assets/shaders/OpenGL/prefilterCubeMap.frag
	assets/shaders/OpenGL/brdfLUT.vert
	assets/shaders/OpenGL/brdfLUT.frag
	assets/shaders/OpenGL/drawNormals.vert
	assets/shaders/OpenGL/drawNormals.frag
	assets/shaders/OpenGL/drawNormals.geom)

set(vulkanShaders 
	assets/shaders/Vulkan/pbr.vert
	assets/shaders/Vulkan/pbr.frag
	assets/shaders/Vulkan/drawNormals.vert
	assets/shaders/Vulkan/drawNormals.frag
	assets/shaders/Vulkan/drawNormals.geom)

set(meshes
	assets/meshes/armchair.fbx
	assets/meshes/globe.fbx
	assets/meshes/pistol.fbx)

set(textures 
	assets/textures/armchair/albedo.png
	assets/textures/armchair/normal.png
	assets/textures/armchair/roughness.png
	assets/textures/armchair/occlusion.png
	assets/textures/globe/albedo.png
	assets/textures/globe/metalness.png
	assets/textures/globe/roughness.png
	assets/textures/globe/occlusion.png
	assets/textures/floor/1_albedo.jpg
	assets/textures/floor/1_normal.jpg
	assets/textures/floor/1_roughness.jpg
	assets/textures/floor/1_height.jpg
	assets/textures/floor/1_occlusion.jpg
	assets/textures/floor/2_albedo.jpg
	assets/textures/floor/2_normal.jpg
	assets/textures/floor/2_roughness.jpg
	assets/textures/floor/2_height.jpg
	assets/textures/pistol/albedo.jpg
	assets/textures/pistol/normal.jpg
	assets/textures/pistol/roughness.jpg
	assets/textures/pistol/metalness.jpg
	assets/textures/pistol/occlusion.jpg
	assets/textures/environment/FactoryCatwalk.hdr
	assets/textures/environment/HamarikyuBridge.hdr
	assets/textures/environment/MonValley.hdr
	assets/textures/environment/TopangaForest.hdr
	assets/textures/environment/TropicalBeach.hdr
	assets/textures/environment/WinterForest.hdr)

source_group("" FILES ${sources})
source_group("\\assets\\shaders\\OpenGL" FILES ${openGLShaders})
source_group("\\assets\\shaders\\Vulkan" FILES ${vulkanShaders})
source_group("\\assets\\meshes" FILES ${meshes})
source_group("\\assets\\textures" FILES ${textures})

add_executable(Sandbox 
	${sources}
	${openGLShaders}
	${vulkanShaders}
	${meshes}
	${textures})
 
target_include_directories(Sandbox PUBLIC "include")
target_link_libraries(Sandbox PRIVATE FireflyEngine)

# GLSL -> SPIRV
if (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "AMD64")
  set(glslValidator "$ENV{VULKAN_SDK}/Bin/glslangValidator.exe")
else()
  set(glslValidator "$ENV{VULKAN_SDK}/Bin32/glslangValidator.exe")
endif()

foreach(shader ${vulkanShaders})
	get_filename_component(fileName ${shader} NAME)
	set(spirv "${CMAKE_CURRENT_BINARY_DIR}/assets/shaders/Vulkan/${fileName}.spv")
	add_custom_command(
		OUTPUT ${spirv}
		COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/assets/shaders/"
		COMMAND ${glslValidator} -V ${shader} -o ${spirv}
		DEPENDS ${shader})
	list(APPEND vulkanSpirvBinaryFiles ${spirv})
endforeach(shader)
add_custom_target(CompileGlslToSpirv DEPENDS ${vulkanSpirvBinaryFiles})
add_dependencies(Sandbox CompileGlslToSpirv)
set_target_properties(CompileGlslToSpirv PROPERTIES FOLDER PostBuild)

# Copy assets to binary
add_custom_target(CopyAssets)
add_dependencies(CompileGlslToSpirv CopyAssets)
set_target_properties(CopyAssets PROPERTIES FOLDER PostBuild)
FILE(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/assets" source)
FILE(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/assets" destination)
add_custom_command(
	TARGET CopyAssets POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${source} ${destination}
	DEPENDS ${destination}
	COMMENT "copy assets folder from ${source} to ${destination}")
