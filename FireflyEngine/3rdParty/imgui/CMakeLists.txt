set(files
	imconfig.h
	imgui.h
	imgui.cpp
	imgui_demo.cpp
	imgui_draw.cpp
	imgui_internal.h
	imgui_widgets.cpp
	imstb_rectpack.h
	imstb_textedit.h
	imstb_truetype.h)

add_library(imgui STATIC ${files})
target_include_directories(imgui PUBLIC "")