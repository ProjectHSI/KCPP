cmake_minimum_required (VERSION 3.12)

function(BIN2C fileName outputFile variableName)
	file(READ ${fileName} fileBinary HEX)
	string(LENGTH ${fileBinary} fileLength)
	math(EXPR fileLengthFinal "${fileLength} / 2")
	string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," fileBinary "${fileBinary}")
	
	file(WRITE "${outputFile}" "#include <cstdint>\n#include <array>\nconstexpr std::array < uint8_t, ${fileLength} > ${variableName} { ${fileBinary} };")
endfunction()