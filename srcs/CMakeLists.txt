if(WIN32)
set(GUI_EXTRA WIN32)
endif()

if(APPLE)
set(GUI_EXTRA MACOSX_BUNDLE)
endif()

configure_file("config.h.in" "config.h" @ONLY)

include_directories(
	${SDL2_INCLUDE_DIRS}
	${SDL2_TTF_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR}
)
aux_source_directory("." FLIPCLOCK_SOURCES)
add_executable(
	"flipclock"
	${GUI_EXTRA}
	${FLIPCLOCK_SOURCES}
)
target_link_libraries(
	"flipclock"
	${SDL2_LIBRARIES}
	${SDL2_TTF_LIBRARIES}
)

if(WIN32)
install(TARGETS "flipclock" DESTINATION "flipclock")
install(FILES "${CMAKE_BINARY_DIR}/flipclock.ttf" DESTINATION "flipclock")
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
install(FILES "${CMAKE_BINARY_DIR}/flipclock.ttf" DESTINATION "share/fonts")
install(FILES "${CMAKE_BINARY_DIR}/flipclock.png" DESTINATION "share/pixmaps")
install(
	FILES "${CMAKE_BINARY_DIR}/flipclock.desktop"
	DESTINATION "share/applications"
)
endif()