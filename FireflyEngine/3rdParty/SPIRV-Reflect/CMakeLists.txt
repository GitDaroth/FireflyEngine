find_package(Vulkan REQUIRED)

set(files
	spirv_reflect.h
	spirv_reflect.c)

add_library(spirv_reflect STATIC ${files})
target_link_libraries(spirv_reflect PRIVATE Vulkan::Vulkan)
target_include_directories(spirv_reflect PUBLIC "")