set(files
	stb_image.h
	stb_image.cpp)

add_library(stb_image STATIC ${files})
target_include_directories(stb_image PUBLIC "")