# --------------------------------------------------------------
# ターゲットのCMakeLists.txt
# --------------------------------------------------------------
project(video_hal_svc CXX C)

# --------------------------------------------------------------
# 依存OSSパッケージ確認
# --------------------------------------------------------------
# pkg-config実行
find_package(PkgConfig)
pkg_check_modules(_JSONCPP REQUIRED jsoncpp)
pkg_check_modules(_libkms QUIET libkms)
pkg_check_modules(_ilmCommon REQUIRED ilmCommon)
pkg_check_modules(_ilmControl REQUIRED ilmControl)
pkg_check_modules(_dlt REQUIRED automotive-dlt)
pkg_check_modules(_GStreamer REQUIRED gstreamer-1.0)

# --------------------------------------------------------------
# ターゲット定義（共有ライブラリまたは実行ファイル切替）
# --------------------------------------------------------------
set ( SOURCE
	src/system/vhal_main.cpp
	src/system/vhal_log.cpp
	src/system/vhal_main_control.cpp
	src/system/vhal_server.cpp
	src/system/vhal_event_item_base.cpp
	src/system/vhal_event_route.cpp
	src/system/vhal_observer_control.cpp
	src/system/vhal_event_item_power_state.cpp
	src/system/vhal_timer.cpp
	src/system/vhal_net_sequence_manager.cpp
	src/system/vhal_str_mng.cpp
	src/system/vhal_debug_system.cpp
	src/layout/vhal_event_item_surface_configure.cpp
	src/layout/vhal_event_item_ivi_event.cpp
	src/layout/vhal_ivi_controller.cpp
	src/layout/vhal_layout_mng.cpp
	src/layout/vhal_layout_file.cpp
	src/layout/vhal_layout_video_setting.cpp
	src/layout/vhal_screen_shot.cpp
	src/layout/vhal_event_item_screen_shot.cpp
	src/layout/vhal_screen_shot_micon.cpp
	src/layout/vhal_event_item_screen_shot_micon.cpp
	src/layout/vhal_event_item_hud_screen.cpp
	src/layout/vhal_hud_screen_controller.cpp
	src/property/vhal_property_client.cpp
	src/property/vhal_property_control.cpp
	src/property/vhal_event_item_add_output.cpp
	src/movie/vhal_event_item_movie_wait.cpp
	src/movie/vhal_movie_control.cpp
	src/movie/vhal_gst_player.c
	src/db/vhal_sysdb_event_item.cpp
	src/db/vhal_sysdb_control.cpp
	src/db/vhal_fileobserver_control.cpp
	src/db/vhal_fileobserver_receive_event_item.cpp
	src/db/vhal_backup_control.cpp
	src/device/vhal_gpio_control.cpp
	src/device/vhal_environment_mng.cpp
	src/mute/vhal_mute.cpp
	src/mute/vhal_event_item_camera_changed.cpp
	src/capture/vhal_capture_camera.cpp
	src/capture/vhal_capture_hdmi.cpp
	src/capture/vhal_capture_control.cpp
	src/capture/vhal_capture_micon_control.cpp
	src/capture/vhal_event_item_capture_reset.cpp
	src/micon/vhal_micon_comm_control.cpp
	src/micon/vhal_micon_misc_control.cpp
	src/micon/vhal_micon_most_control.cpp
	src/micon/vhal_micon_most_video_info.cpp
	src/color/vhal_color_mng.cpp
	src/color/vhal_color_recv_item_quality.cpp
	src/color/vhal_color_send_item_quality.cpp
	src/color/vhal_color_db.cpp
	src/color/vhal_color_record.cpp
	src/color/vhal_color_record_daynight.cpp
	src/color/vhal_color_record_theme.cpp
	src/hdcp/vhal_hdcp_auth_control.cpp
	src/hdcp/vhal_hdcp_auth_mng.cpp
	src/hdcp/vhal_event_item_hdcp_auth.cpp
)

if(PROCESS_TO_THREAD)
	# 共有ライブラリの生成
	add_library(${PROJECT_NAME} SHARED ${SOURCE})
else()
	# 実行ファイルの生成
	add_executable(${PROJECT_NAME} ${SOURCE} src/system/vhal_entry_point.cpp)
endif()

# インクルードパスの設定
target_include_directories(${PROJECT_NAME} PRIVATE
	./inc
	./inc/system
	./inc/layout
	./inc/property
	./inc/db
	./inc/device
	./inc/mute
	./inc/capture
	./inc/micon
	./inc/color
	./inc/movie
	./inc/hdcp
)

# インクルードパスの設定(依存OSSパッケージ)
target_include_directories(${PROJECT_NAME} BEFORE PRIVATE
	${_JSONCPP_INCLUDE_DIRS}
	${_libkms_INCLUDE_DIRS}
	${_ilmCommon_INCLUDE_DIRS}
	${_ilmControl_INCLUDE_DIRS}
	${_dlt_INCLUDE_DIRS}
	${_GStreamer_INCLUDE_DIRS}
	${CMAKE_SYSROOT}/usr/include/qcx
)

# コンパイルオプション
target_compile_options(${PROJECT_NAME} PRIVATE
	-fsigned-char
	-fsigned-bitfields
	-funwind-tables
	-rdynamic
)

# コンパイルオプション(デファイン)
# BEVstep3
if(TIER1_BUILD_TARGET MATCHES "sa8255p-dn-cdc")
	# 1Sボード
	target_compile_options(${PROJECT_NAME} PRIVATE
		-DRENDERER_SHM=false
		-DVHAL_TIER1_TARGET=2
	)
elseif(TIER1_BUILD_TARGET MATCHES "sa8255p-adp-air")
	# ADPボード
	target_compile_options(${PROJECT_NAME} PRIVATE
		-DRENDERER_SHM=false
		-DVHAL_TIER1_TARGET=1
	)
else()
	# x86-64
	target_compile_options(${PROJECT_NAME} PRIVATE
		-DRENDERER_SHM=true
		-DVHAL_TIER1_TARGET=0
	)
endif()

# ライブラリのリンク
target_link_libraries(${PROJECT_NAME}
	pthread
	sysdb
	sysevmng_m_api
	utlpkgcom
	hal
	halcomm
	wayland-client
	ivi-application
	qcxclient
	qcxosal
	wlrenderer
	libccm_client.so.1
	timer
	sifutil
	FileObserver
	sysbkupmng
	pfmtier1
	ls3ctl
	gbm
)

# ライブラリのリンク(依存OSSパッケージ)
target_link_libraries(${PROJECT_NAME}
	${_JSONCPP_LIBRARIES}
	${_ilmCommon_LIBRARIES}
	${_ilmControl_LIBRARIES}
	${_dlt_LIBRARIES}
	${_GStreamer_LIBRARIES}
)

#実行ファイルのインストール
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
