cmake_minimum_required(VERSION 3.0)
project("flipclock" C)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")
set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_C_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -std=c11 -Wall -g -D__DEBUG__")
set(CMAKE_C_FLAGS_RELEASE "$ENV{CXXFLAGS} -O2 -std=c11")

find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)

file(
	COPY "${CMAKE_SOURCE_DIR}/flipclock.ttf"
	DESTINATION ${CMAKE_BINARY_DIR}
)
file(
	COPY "${CMAKE_SOURCE_DIR}/flipclock.png"
	DESTINATION ${CMAKE_BINARY_DIR}
)
file(
	COPY "${CMAKE_SOURCE_DIR}/flipclock.desktop"
	DESTINATION ${CMAKE_BINARY_DIR}
)
add_subdirectory("srcs")
