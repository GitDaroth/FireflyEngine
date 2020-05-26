set(sources 
	src/SandboxGame.cpp)

add_executable(Sandbox ${sources})
target_link_libraries(Sandbox PRIVATE FireflyEngine)