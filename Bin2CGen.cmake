function(BIN2CGEN intFile fileName outputFile variableName)
	file(WRITE "${intFile}" "include(\"${CMAKE_SOURCE_DIR}/Bin2C.cmake\")

BIN2C(\"${fileName}\" \"${outputFile}\" \"${variableName}\")")
	
	add_custom_command(OUTPUT "${outputFile}" COMMAND "${CMAKE_COMMAND}" ARGS "-P" "${intFile}"	DEPENDS "${fileName}" "${intFile}" "${CMAKE_SOURCE_DIR}/Bin2C.cmake" VERBATIM WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
endfunction()