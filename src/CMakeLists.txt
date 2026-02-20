set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmodules -O0 -g -Wall -Wextra -Wpedantic")

add_library(wordmeter)
target_sources(wordmeter PUBLIC FILE_SET CXX_MODULES FILES
	wordmeter.cppm
	distance.cppm
	database.cppm
)
target_sources(wordmeter PRIVATE database.cpp)

add_executable(parseText parseText.cpp)
target_link_libraries(parseText PRIVATE wordmeter)
# Usage: parseText <inputFile.txt> <outputFile.db>

add_executable(calcStat calcStat.cpp)
target_link_libraries(calcStat PRIVATE wordmeter)
# Usage: calcStat [options] <inputFile.db> <wordPairFile.txt>
# Usage: calcStat [options] <inputFile.txt> <wordPairFile.txt>

# Options:
# --dist prints the distances between each occurrence of wordA and wordB.
# --minDist prints the smallest distance between wordA and wordB.
# --maxDist prints the largest distance between wordA and wordB.
# --avgDist prints the average distance between wordA and wordB.
# --sqVariance prints the square variance of wordA and wordB.
# --all applies all options.

add_executable(catDb catDb.cpp)
target_link_libraries(catDb PRIVATE wordmeter)
# Usage: catDb <inputFile.db> [<maxPosColSz>]
