set(files
	include/glad/glad.h
	include/KHR/khrplatform.h
	src/glad.c)

add_library(glad STATIC ${files})
target_include_directories(glad PUBLIC "include")