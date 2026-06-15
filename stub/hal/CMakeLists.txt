# pkg-config
project(hal C)

find_package(PkgConfig)

# add_executable

# add definitions

# add_library
add_library(${PROJECT_NAME} SHARED
           hal_api.c
)

# target_include_directories
include_directories(.)
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
#target_link_libraries(${PROJECT_NAME} utlrpcclient)


# install
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_VIDEO_STUB})
#install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/hal_public.h DESTINATION include)
#install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/hal_gpio_public.h DESTINATION include)

