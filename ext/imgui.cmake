add_library(imgui STATIC EXCLUDE_FROM_ALL
# core
"ext/imgui/imconfig.h" "ext/imgui/imgui.cpp" "ext/imgui/imgui.h" "ext/imgui/imgui_demo.cpp" "ext/imgui/imgui_draw.cpp" "ext/imgui/imgui_internal.h" "ext/imgui/imgui_tables.cpp" "ext/imgui/imgui_tables.cpp" "ext/imgui/imgui_widgets.cpp" "ext/imgui/imstb_rectpack.h" "ext/imgui/imstb_textedit.h" "ext/imgui/imstb_truetype.h"

# backends
"ext/imgui/backends/imgui_impl_sdl3.cpp" "ext/imgui/backends/imgui_impl_sdl3.h" "ext/imgui/backends/imgui_impl_sdlrenderer3.cpp" "ext/imgui/backends/imgui_impl_sdlrenderer3.h"

# extras
"ext/imgui/misc/cpp/imgui_stdlib.cpp" "ext/imgui/misc/cpp/imgui_stdlib.h"
)
target_include_directories(imgui PUBLIC "ext/imgui/" "ext/imgui/backends/")
target_link_libraries(imgui PUBLIC SDL3::SDL3)

target_compile_definitions(imgui PUBLIC "IMGUI_DISABLE_OBSOLETE_FUNCTIONS")

if (${CMAKE_BUILD_TYPE} STREQUAL "Release" OR ${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
	target_compile_definitions(imgui PUBLIC "IMGUI_DISABLE_DEMO_WINDOWS" "IMGUI_DISABLE_DEBUG_TOOLS")
endif()