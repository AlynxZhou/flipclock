if(WIN32)
set(GUI_EXTRA WIN32)
endif()

if(APPLE)
set(GUI_EXTRA MACOSX_BUNDLE)
endif()

include_directories(
	${SDL2_INCLUDE_DIRS}
	${SDL2_TTF_INCLUDE_DIRS}
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
