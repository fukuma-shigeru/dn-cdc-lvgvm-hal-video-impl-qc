# pkg-config
project(gst-launch-1.0 C)

# add_executable
set	( SOURCE
	gst_launch.c
)

add_executable(${PROJECT_NAME} ${SOURCE})

# add definitions

# add_library

# target_include_directories

# target_compile_options
target_compile_options(${PROJECT_NAME} PRIVATE
	-fsigned-char
	-fsigned-bitfields
	-funwind-tables
	-rdynamic
)

#target_compile_definitions

# target_link_libraries

# install
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_VIDEO_STUB})

