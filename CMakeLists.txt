cmake_minimum_required(VERSION 3.0)

project(
	"flipclock"
	LANGUAGES C
	VERSION 2.4.0
)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")
set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")

if(NOT MSVC)
	set(
		CMAKE_C_FLAGS_DEBUG
		"$ENV{CXXFLAGS} -O0 -std=c11 -Wall -g -D__DEBUG__"
	)
	set(CMAKE_C_FLAGS_RELEASE "$ENV{CXXFLAGS} -O2 -std=c11")
endif()

add_subdirectory("srcs")
