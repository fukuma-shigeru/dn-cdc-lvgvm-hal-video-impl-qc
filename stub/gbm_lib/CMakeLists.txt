# pkg-config
project(gbm C)

find_package(PkgConfig)

# add_executable

# add definitions

# add_library
add_library(${PROJECT_NAME} SHARED
	gbm_stub.c
)

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
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_VIDEO_STUB})

