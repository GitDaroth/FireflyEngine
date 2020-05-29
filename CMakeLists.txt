cmake_minimum_required(VERSION 3.17)

project(Firefly 
	VERSION 0.0.1
	LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG_MODE")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(WIN32)
    add_compile_definitions(FIREFLY_WINDOWS)
endif()

add_subdirectory(FireflyEngine)
add_subdirectory(Sandbox)