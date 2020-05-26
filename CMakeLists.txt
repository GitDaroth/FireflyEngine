cmake_minimum_required(VERSION 3.12)

project(Firefly VERSION 0.0.1
				LANGUAGES CXX)

add_compile_definitions(FIREFLY_WINDOWS)

add_subdirectory(FireflyEngine)
add_subdirectory(Sandbox)