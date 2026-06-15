# --------------------------------------------------------------
# 繋ぎ役のCMakeLists.txt
# --------------------------------------------------------------
set(CMAKE_INSTALL_VIDEO_HAL_TEST "/opt/dc-ivi-pf/tests/dn-cdc-lvgvm-hal-video-impl-qc" CACHE PATH "video hal test install path" FORCE)

#add_subdirectory(video_hal_tp)
add_subdirectory(render_image)
add_subdirectory(test_driver)
#add_subdirectory(stub)
add_subdirectory(qcarcam_seq)
