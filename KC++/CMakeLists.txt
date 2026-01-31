# CMakeList.txt : CMake project for KC++, include source and define
# project specific logic here.
#

add_executable (KC++ "KC++.cpp" "KC++.h" "Style.h" "LCDStyle.h" "LCDStyle.cpp" "InputChecker.h" "LCDFont.h" "KCCommon.h" "KCCommon.cpp" "KCBuild.h" "Menu.h" "Menu.cpp" "KC++Save.proto" "LCDStyleSave.proto" "UsernameFinder.h" "Styles.h" "Styles.cpp" "UsernameFinder.cpp" "Save.h" "Save.cpp" "${CMAKE_CURRENT_BINARY_DIR}/binDir/KC++.png.h")

protobuf_generate(TARGET KC++ LANGUAGE cpp PROTOC_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/protoOutput" IMPORT_DIRS "${CMAKE_CURRENT_SOURCE_DIR}")

add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/binDir/KC++.png.h" "${CMAKE_CURRENT_BINARY_DIR}/binDir/KC++.png._h" COMMAND "${CMAKE_COMMAND}" ARGS "-DKCPPResourcesGenBinDir=${CMAKE_CURRENT_BINARY_DIR}" "-P" "${CMAKE_CURRENT_SOURCE_DIR}/KC++PNGResGen.cmake"	DEPENDS "${CMAKE_SOURCE_DIR}/KC++.png" "${CMAKE_CURRENT_SOURCE_DIR}/KC++PNGResGen.cmake" "${CMAKE_SOURCE_DIR}/Bin2C.cmake" VERBATIM WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

target_include_directories(KC++ PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/protoOutput")
target_include_directories(KC++ PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/binDir")

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	target_sources(KC++ PRIVATE "Platform/Win32/Win32Username.cpp")
	target_sources(KC++ PRIVATE "Platform/Win32/Win32Input.cpp")

	# while we're here
	#if (${CMAKE_BUILD_TYPE} STREQUAL "Release" OR ${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
		#set_target_properties(KC++ PROPERTIES WIN32_EXECUTABLE YES)
	#endif()
else()
	target_sources(KC++ PRIVATE "Platform/Default/DefaultUsername.cpp")
	#target_sources(KC++ PRIVATE "Platform/Default/DefaultInput.cpp")
	message(FATAL_ERROR "Unsupported platform")
endif()

set_property(TARGET KC++ PROPERTY CXX_STANDARD 20)

target_link_libraries(KC++ PRIVATE SDL3::SDL3)
target_link_libraries(KC++ PRIVATE protobuf::libprotobuf-lite)
target_include_directories(KC++ PRIVATE "${CMAKE_SOURCE_DIR}/ext/protobuf/third_party/utf8_range")
target_link_libraries(KC++ PRIVATE imgui)

# TODO: Add tests and install targets if needed.
