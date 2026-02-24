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

add_executable(calcStat calcStat.cpp)
target_link_libraries(calcStat PRIVATE wordmeter)

add_executable(catDb catDb.cpp)
target_link_libraries(catDb PRIVATE wordmeter)
