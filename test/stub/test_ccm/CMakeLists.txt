cmake_minimum_required(VERSION 3.14)
project(test_ccm_stub)

# Pkg-config run
find_package(PkgConfig)
pkg_check_modules(_gtestmain REQUIRED gtest_main)
pkg_check_modules(_dlt REQUIRED automotive-dlt)

set(EXECUTABLE_OUTPUT_PATH test)

add_executable(${PROJECT_NAME}
    test_main.cpp
#    test_ccm_01.cpp
    test_ccm_02.cpp
)

target_compile_options(${PROJECT_NAME} PRIVATE
	-pthread
	-fsigned-char
	-fsigned-bitfields
	-funwind-tables
	-rdynamic
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${_dlt_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PRIVATE ../../client/ccm)


target_link_libraries(${PROJECT_NAME} ${_gtestmain_LIBRARIES})
target_link_libraries(${PROJECT_NAME} FileObserver)
target_link_libraries(${PROJECT_NAME} ccm_client)
target_link_libraries(${PROJECT_NAME} stub_common)
target_link_libraries(${PROJECT_NAME} ${_dlt_LIBRARIES})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_VIDEO_HAL_TEST})
