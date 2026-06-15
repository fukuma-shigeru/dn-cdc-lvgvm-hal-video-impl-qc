cmake_minimum_required(VERSION 3.14)
project(test_driver_video_hal)

# Pkg-config run
find_package(PkgConfig)
pkg_check_modules(_gtestmain REQUIRED gtest_main)
pkg_check_modules(_dlt REQUIRED automotive-dlt)

set(EXECUTABLE_OUTPUT_PATH test)

add_executable(${PROJECT_NAME}
    src/test_main.cpp
    src/test_util.cpp
    src/halcp_lib.cpp
    src/video_hal_test_01_video_hal_api.cpp
    src/video_hal_test_02_layer_configuration.cpp
    src/video_hal_test_03_front_vpath.cpp
    src/video_hal_test_04_hdmi.cpp
    src/video_hal_test_05_vsrc_and_color_management.cpp
    src/video_hal_test_06_camera.cpp
    src/video_hal_test_07_illumination.cpp
    src/video_hal_test_08_rear_vpath.cpp
    src/video_hal_test_09_combination.cpp
    src/video_hal_test_10_setting.cpp
    src/video_hal_test_11_movie_play.cpp
    src/video_hal_test_12_ivi_property.cpp
    src/video_hal_test_13_ivi_order.cpp
    src/video_hal_test_14_ivi_screen_capture.cpp
    src/video_hal_test_15_instrument_cluster.cpp
    src/video_hal_test_16_mute.cpp
    src/video_hal_test_17_img_adj.cpp

)

# completely shutdown variable assignment tracking.
set_source_files_properties(src/video_hal_test_03_front_vpath.cpp PROPERTIES COMPILE_FLAGS -fno-var-tracking-assignments)
set_source_files_properties(src/video_hal_test_08_rear_vpath.cpp PROPERTIES COMPILE_FLAGS -fno-var-tracking-assignments)
set_source_files_properties(src/video_hal_test_15_instrument_cluster.cpp PROPERTIES COMPILE_FLAGS -fno-var-tracking-assignments)
set_source_files_properties(src/video_hal_test_17_img_adj.cpp PROPERTIES COMPILE_FLAGS -fno-var-tracking-assignments)

target_compile_options(${PROJECT_NAME} PRIVATE
	-pthread
	-fsigned-char
	-fsigned-bitfields
	-funwind-tables
	-rdynamic
	-Wno-unused-result
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ./inc
    ${CMAKE_SOURCE_DIR}/include
    ${_dlt_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} ${_gtestmain_LIBRARIES})
target_link_libraries(${PROJECT_NAME} video_hal)
target_link_libraries(${PROJECT_NAME} ilmCommon)
target_link_libraries(${PROJECT_NAME} ilmControl)
target_link_libraries(${PROJECT_NAME} stub_common)
target_link_libraries(${PROJECT_NAME} sifutil)
target_link_libraries(${PROJECT_NAME} FileObserver)
target_link_libraries(${PROJECT_NAME} ${_dlt_LIBRARIES})


install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_VIDEO_HAL_TEST})
