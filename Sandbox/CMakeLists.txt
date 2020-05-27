set(sources 
	src/SandboxGame.cpp)

add_executable(Sandbox ${sources})
target_link_libraries(Sandbox PRIVATE FireflyEngine)

add_custom_command(TARGET Sandbox POST_BUILD        
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        "${PROJECT_SOURCE_DIR}/build/FireflyEngine/$<CONFIGURATION>/FireflyEngine.dll" 
        $<TARGET_FILE_DIR:Sandbox>)