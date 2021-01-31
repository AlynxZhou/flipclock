cmake_minimum_required(VERSION 3.0)

project(
	"flipclock"
	LANGUAGES C
	VERSION 2.7.4
)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules")

if(NOT MSVC)
	set(
		CMAKE_C_FLAGS_DEBUG
		"$ENV{CXXFLAGS} -O0 -std=c11 -Wall -g -D__DEBUG__"
	)
	set(CMAKE_C_FLAGS_RELEASE "$ENV{CXXFLAGS} -O2 -std=c11")
endif()

find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)

configure_file("srcs/config-cmake.h.in" "config.h" @ONLY)

include_directories(
	${SDL2_INCLUDE_DIRS}
	${SDL2_TTF_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR}
)

aux_source_directory("srcs" FLIPCLOCK_SOURCES)

if(WIN32)
	add_executable(
		"flipclock"
		WIN32
		${FLIPCLOCK_SOURCES}

	)
	set_target_properties(
		"flipclock"
		PROPERTIES
		SUFFIX ".scr"
	)
elseif(APPLE)
	add_executable(
		"flipclock"
		MACOSX_BUNDLE
		${FLIPCLOCK_SOURCES}

	)
else()
	add_executable(
		"flipclock"
		${FLIPCLOCK_SOURCES}
	)
endif()

target_link_libraries(
	"flipclock"
	${SDL2_LIBRARIES}
	${SDL2_TTF_LIBRARIES}
)

if(WIN32)
	install(TARGETS "flipclock" DESTINATION "flipclock")
	install(FILES "${CMAKE_SOURCE_DIR}/LICENSE" DESTINATION "flipclock")
	install(
		FILES "${CMAKE_SOURCE_DIR}/dists/flipclock.ttf"
		DESTINATION "flipclock"
	)
	install(
		FILES "${CMAKE_SOURCE_DIR}/dists/COPYING"
		DESTINATION "flipclock"
	)
	install(
		FILES "${CMAKE_SOURCE_DIR}/dists/请先读我.txt"
		DESTINATION "flipclock"
	)
	install(
		FILES "${SDL2_DIR}/${VC_LIB_PATH_SUFFIX}/SDL2.dll"
		DESTINATION "flipclock"
	)
	install(
		FILES "${SDL2_TTF_DIR}/${VC_LIB_PATH_SUFFIX}/SDL2_ttf.dll"
		DESTINATION "flipclock"
	)
	install(
		FILES "${SDL2_TTF_DIR}/${VC_LIB_PATH_SUFFIX}/zlib1.dll"
		DESTINATION "flipclock"
	)
	install(
		FILES "${SDL2_TTF_DIR}/${VC_LIB_PATH_SUFFIX}/libfreetype-6.dll"
		DESTINATION "flipclock"
	)
else()
	install(TARGETS "flipclock" DESTINATION "bin")
	install(
		FILES "${CMAKE_SOURCE_DIR}/LICENSE"
		DESTINATION "share/licenses/flipclock"
	)
	install(
		FILES "${CMAKE_SOURCE_DIR}/dists/flipclock.ttf"
		DESTINATION "share/fonts"
	)
	install(
		FILES "${CMAKE_SOURCE_DIR}/dists/flipclock.png"
		DESTINATION "share/pixmaps"
	)
	install(
		FILES "${CMAKE_SOURCE_DIR}/dists/flipclock.desktop"
		DESTINATION "share/applications"
	)
endif()
