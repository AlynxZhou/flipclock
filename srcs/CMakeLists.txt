include_directories(
	${SDL2_INCLUDE_DIRS}
	${SDL2_TTF_INCLUDE_DIRS}
)
aux_source_directory("." FLIPCLOCK_SOURCES)
add_executable(
	"flipclock"
	${FLIPCLOCK_SOURCES}
)
target_link_libraries(
	"flipclock"
	${SDL2_LIBRARIES}
	${SDL2_TTF_LIBRARIES}
)
