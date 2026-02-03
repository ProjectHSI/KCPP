# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.12)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

set(BUILD_SHARED_LIBS NO)

project ("KC++")

set (CMAKE_CXX_STANDARD 23)

SET(SDL_AUDIO OFF)
#define_sdl_subsystem(Video)
SET(SDL_GPU OFF)
#define_sdl_subsystem(Render DEPS SDL_VIDEO)
SET(SDL_CAMERA OFF)
#define_sdl_subsystem(Joystick)
SET(SDL_HAPTIC OFF)
SET(SDL_HIDAPI OFF)
SET(SDL_POWER OFF)
SET(SDL_SENSOR OFF)
SET(SDL_DIALOG OFF)
SET(SDL_TRAY ON)
set(SDL_HIDAPI_LIBUSB OFF)
set(SDL_TEST_LIBRARY YES)
set(SDL_TESTS YES)

add_subdirectory ("ext/sdl" EXCLUDE_FROM_ALL)

set (ABSL_BUILD_TESTING NO)
set (ABSL_USE_SYSTEM_INCLUDES YES)
set (ABSL_PROPAGATE_CXX_STD NO)

add_subdirectory ("ext/abseil" EXCLUDE_FROM_ALL)

set (protobuf_VERBOSE NO)
set (protobuf_INSTALL NO)
set (protobuf_BUILD_TESTS NO)
set (protobuf_BUILD_LIBUPB YES)

add_subdirectory ("ext/protobuf" EXCLUDE_FROM_ALL)
include ("ext/protobuf/cmake/protobuf-generate.cmake")

include("ext/imgui.cmake")

if (MSVC)
	add_compile_options("/Wall")
	# Disable some useless warnings.
	# Split onto seperate lines for better commenting.
	add_compile_options("/wd4514") # Unreferenced Inline Function
	add_compile_options("/wd4464") # Relative Include Path containing ".."
	add_compile_options("/wd4820") # Padding
	add_compile_options("/wd5045") # Spectre Mitigation Notification
	add_compile_options("/wd5039") # 'extern "C"' function used with -EHc
	add_compile_options("/wd4061") # Switch does not explictly handle enum value
	add_compile_options("/permissive-")
else()
	add_compile_options("-Wpedantic")
	add_compile_options("-Wno-c++98-compat") # C++98 Compat
	add_compile_options("-Wno-c++98-compat-pedantic") # More C++98 Compat
	string(APPEND CMAKE_CXX_FLAGS_RELEASE " -O3")
endif()

# Include sub-projects.
add_subdirectory ("KC++")
