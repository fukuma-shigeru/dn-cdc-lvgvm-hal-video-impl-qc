# pkg-config
project(drm C)

find_package(PkgConfig)
pkg_check_modules(_libdrm REQUIRED libdrm)

# add_executable

# add definitions

# add_library
add_library(${PROJECT_NAME} SHARED
           xf86drm.c
)

# target_include_directories
include_directories(${_libdrm_INCLUDE_DIRS})
include_directories(../common)

# target_compile_options
target_compile_options(${PROJECT_NAME} PRIVATE
	-fsigned-char
	-fsigned-bitfields
	-funwind-tables
	-rdynamic
)


#target_compile_definitions

# target_link_libraries
#target_link_libraries(${PROJECT_NAME} pthread)


# install
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_VIDEO_STUB})

