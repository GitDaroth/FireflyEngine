set(sources 
	src/SandboxApp.cpp
	include/CameraController.h
	src/CameraController.cpp)

set(shaders 
	assets/shaders/test.glsl)

set(meshes
	assets/meshes/pistol.fbx)

set(textures 
	assets/textures/pistol_albedo.jpg
	assets/textures/test.jpg)

source_group("" FILES ${sources})
source_group("\\assets\\shaders" FILES ${shaders})
source_group("\\assets\\meshes" FILES ${meshes})
source_group("\\assets\\textures" FILES ${textures})

add_executable(Sandbox 
	${sources}
	${shaders}
	${meshes}
	${textures})
 
 target_include_directories(Sandbox PUBLIC "include")
target_link_libraries(Sandbox PRIVATE FireflyEngine)


add_custom_target(symlinkAssets)
set_target_properties(symlinkAssets PROPERTIES FOLDER PostBuild)

FILE(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/assets" source)
FILE(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/assets" destination)

add_custom_command(
	TARGET symlinkAssets POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E create_symlink ${source} ${destination}
	DEPENDS ${destination}
	COMMENT "symbolic link assets folder from ${source} => ${destination}")

add_dependencies(Sandbox symlinkAssets)