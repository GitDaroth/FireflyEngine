set(sources 
	src/SandboxGame.cpp)

add_executable(Sandbox ${sources})
target_include_directories(Sandbox PRIVATE "${PROJECT_SOURCE_DIR}/FireflyEngine/src")
target_link_libraries(Sandbox PRIVATE FireflyEngine)