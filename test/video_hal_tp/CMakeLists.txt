# --------------------------------------------------------------
# Target CMakeLists.txt
# --------------------------------------------------------------
project(video_hal_tp CXX)

# --------------------------------------------------------------
# Dependent OSS package confirmation
# --------------------------------------------------------------
# Pkg-config run
find_package(PkgConfig)
pkg_check_modules(_gtestmain REQUIRED gtest_main)

# --------------------------------------------------------------
# Target definition (executable)
# --------------------------------------------------------------
# Generate an executable file.
add_executable(${PROJECT_NAME}
    src/common/vhal_test_main.cpp
    src/common/vhal_test_common.cpp
    src/api/vhal_api_test_capture.cpp
    src/api/vhal_api_test_hdcp.cpp
    src/api/vhal_api_test_heacon_front_output.cpp
    src/api/vhal_api_test_ivi_layer.cpp
    src/api/vhal_api_test_ivi_property.cpp
    src/api/vhal_api_test_layer_configuration.cpp
    src/api/vhal_api_test_libvideo_hal.cpp
    src/api/vhal_api_test_movie.cpp
    src/api/vhal_api_test_mute.cpp
    src/api/vhal_api_test_opacity.cpp
    src/api/vhal_api_test_setting.cpp
    src/api/vhal_api_test_video_adjustment.cpp
    src/api/vhal_api_test_video_source_id.cpp
    src/api/vhal_api_test_visible.cpp
    src/api/vhal_api_test_vpath_camera_output.cpp
    src/api/vhal_api_test_vpath_camera.cpp
    src/api/vhal_api_test_vpath_front.cpp
    src/api/vhal_api_test_vpath_front_clipping.cpp
    src/api/vhal_api_test_vpath_front_output.cpp
    src/api/vhal_api_test_vpath_ic_clipping.cpp
    src/api/vhal_api_test_vpath_ic_output.cpp
    src/api/vhal_api_test_vpath_ic.cpp
    src/api/vhal_api_test_vpath_rear.cpp
    src/api/vhal_api_test_vpath_rear_output.cpp
    src/api/vhal_api_test_vpath_rse.cpp
    src/api/vhal_api_test_wide_mode.cpp
    src/seq/vhal_seq_test_capture.cpp
    src/seq/vhal_seq_test_hdcp.cpp
    src/seq/vhal_seq_test_heacon_front_output.cpp
    src/seq/vhal_seq_test_ivi_layer.cpp
    src/seq/vhal_seq_test_ivi_property.cpp
    src/seq/vhal_seq_test_layer_configuration.cpp
    src/seq/vhal_seq_test_libvideo_hal.cpp
    src/seq/vhal_seq_test_movie.cpp
    src/seq/vhal_seq_test_mute.cpp
    src/seq/vhal_seq_test_opacity.cpp
    src/seq/vhal_seq_test_setting.cpp
    src/seq/vhal_seq_test_video_adjustment.cpp
    src/seq/vhal_seq_test_video_source_id.cpp
    src/seq/vhal_seq_test_visible.cpp
    src/seq/vhal_seq_test_vpath_camera_output.cpp
    src/seq/vhal_seq_test_vpath_camera.cpp
    src/seq/vhal_seq_test_vpath_front.cpp
    src/seq/vhal_seq_test_vpath_front_clipping.cpp
    src/seq/vhal_seq_test_vpath_front_output.cpp
    src/seq/vhal_seq_test_vpath_ic_clipping.cpp
    src/seq/vhal_seq_test_vpath_ic_output.cpp
    src/seq/vhal_seq_test_vpath_ic.cpp
    src/seq/vhal_seq_test_vpath_rear.cpp
    src/seq/vhal_seq_test_vpath_rear_output.cpp
    src/seq/vhal_seq_test_vpath_rse.cpp
    src/seq/vhal_seq_test_wide_mode.cpp
    src/env/vhal_env_test_vpath_front.cpp
    src/env/vhal_env_test_vpath_rear.cpp
    src/env/vhal_env_test_vpath_ic.cpp
    src/env/vhal_env_test_vpath_camera.cpp
    src/env/vhal_env_test_setting.cpp
    src/etc/vhal_function_test.cpp
)

# Set include path
target_include_directories(${PROJECT_NAME} BEFORE PRIVATE
    ./inc
    ${CMAKE_SOURCE_DIR}/include
)

# Set include path (dependent OSS packages)
target_include_directories(${PROJECT_NAME} BEFORE PRIVATE
    ${_gtestmain_INCLUDE_DIRS}
)

# compile option(define)
# BEVstep3
if(TIER1_BUILD_TARGET MATCHES "sa8255p-dn-cdc")
	# 1S board
	target_compile_options(${PROJECT_NAME} PRIVATE
		-DNOP_CONNECTED_CAMERA_VALUE=false
		-DVHAL_TIER1_TARGET=2
	)
elseif(TIER1_BUILD_TARGET MATCHES "sa8255p-adp-air")
	# ADP board
	target_compile_options(${PROJECT_NAME} PRIVATE
		-DNOP_CONNECTED_CAMERA_VALUE=true
		-DVHAL_TIER1_TARGET=1
	)
else()
	# x86-64
	target_compile_options(${PROJECT_NAME} PRIVATE
		-DNOP_CONNECTED_CAMERA_VALUE=true
		-DVHAL_TIER1_TARGET=0
	)
endif()

# Link test target library
target_link_libraries(${PROJECT_NAME}
    video_hal
    sysdb
    ilmCommon
    ilmControl
    pthread
    dlt
)

# Link library (dependent OSS package)
target_link_libraries(${PROJECT_NAME}
    ${_gtestmain_LIBRARIES}
#    ${OpenCV_LIBS}
)

# Collect VideoHal test module
#add_custom_target(collect-test-module
#    COMMAND echo "collect VideoHal test module."
#    COMMAND mkdir -p ./video_hal_test_module
#    COMMAND cp -p ${PROJECT_BINARY_DIR}/video_hal_tp ./video_hal_test_module
#    COMMAND cp -p ${PROJECT_BINARY_DIR}/../render_image/render_image ./video_hal_test_module
#    COMMAND cp -p ${PROJECT_SOURCE_DIR}/data/*.png ./video_hal_test_module
#    COMMAND cp -p ${PROJECT_SOURCE_DIR}/data/*.mp4 ./vdeo_hal_test_module
#    COMMAND cp -p ${PROJECT_SOURCE_DIR}/script/run-target-video_hal_tp.sh ./video_hal_test_module
#    COMMAND tar cvzf video_hal_test_module.tar.gz ./video_hal_test_module
#)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_VIDEO_HAL_TEST}/video_hal_test_module)
install(PROGRAMS script/run-target-video_hal_tp.sh DESTINATION ${CMAKE_INSTALL_VIDEO_HAL_TEST}/video_hal_test_module)
install(PROGRAMS data/ANDROIDAUTO_1280x720.png     DESTINATION ${CMAKE_INSTALL_VIDEO_HAL_TEST}/video_hal_test_module)
install(PROGRAMS data/CARPLAY_1280x720.png         DESTINATION ${CMAKE_INSTALL_VIDEO_HAL_TEST}/video_hal_test_module)
install(PROGRAMS data/Clipping_800x600.png         DESTINATION ${CMAKE_INSTALL_VIDEO_HAL_TEST}/video_hal_test_module)
install(PROGRAMS data/DRC_1280x720.png             DESTINATION ${CMAKE_INSTALL_VIDEO_HAL_TEST}/video_hal_test_module)
install(PROGRAMS data/DTV_1280x720.png             DESTINATION ${CMAKE_INSTALL_VIDEO_HAL_TEST}/video_hal_test_module)
install(PROGRAMS data/MAP_1280x720.png             DESTINATION ${CMAKE_INSTALL_VIDEO_HAL_TEST}/video_hal_test_module)
install(PROGRAMS data/MEDIAPLAYER_1280x720.png     DESTINATION ${CMAKE_INSTALL_VIDEO_HAL_TEST}/video_hal_test_module)
install(PROGRAMS data/Wide_800x600.png             DESTINATION ${CMAKE_INSTALL_VIDEO_HAL_TEST}/video_hal_test_module)
install(PROGRAMS data/FRAGRANCE_1280x720.png       DESTINATION ${CMAKE_INSTALL_VIDEO_HAL_TEST}/video_hal_test_module)
install(PROGRAMS data/MULTISENSORY_1280x720.png    DESTINATION ${CMAKE_INSTALL_VIDEO_HAL_TEST}/video_hal_test_module)
install(PROGRAMS data/HUD_448x224.png              DESTINATION ${CMAKE_INSTALL_VIDEO_HAL_TEST}/video_hal_test_module)
