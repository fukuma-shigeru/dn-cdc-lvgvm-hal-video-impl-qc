project(wlrenderer)

find_package(PkgConfig)
pkg_check_modules(_ilmCommon REQUIRED ilmCommon)
pkg_check_modules(_ilmControl REQUIRED ilmControl)
pkg_check_modules(_dlt REQUIRED automotive-dlt)
pkg_check_modules(_libdrm REQUIRED libdrm)

add_library(${PROJECT_NAME} SHARED
	src/wl_renderer.cpp
	src/wl_renderer_log.cpp
	src/wl_renderer_config.cpp
	src/wl_renderer_video_buffer.cpp
	src/wl_renderer_video.cpp
	linux-dmabuf-unstable-v1-client-protocol.h
	linux-dmabuf-unstable-v1-protocol.c
)

# インクルードパスの設定
target_include_directories(${PROJECT_NAME} PRIVATE
	${_libdrm_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR}
	../video_buf_ctl/include
	./inc
)

# インクルードパスの設定(依存OSSパッケージ)
target_include_directories(${PROJECT_NAME} BEFORE PRIVATE
	${_ilmCommon_INCLUDE_DIRS}
	${_ilmControl_INCLUDE_DIRS}
	${_dlt_INCLUDE_DIRS}
)

# ライブラリのリンク
target_link_libraries(${PROJECT_NAME}
	pthread
	wayland-client
	ivi-application
	video_buf_ctl
)

# ライブラリのリンク(依存OSSパッケージ)
target_link_libraries(${PROJECT_NAME}
	${_ilmCommon_LIBRARIES}
	${_ilmControl_LIBRARIES}
	${_dlt_LIBRARIES}
#	${_libdrm_LIBRARIES}
)

# ライブラリのインストール
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_VIDEO_STUB})


# 公開ヘッダファイルのインストール
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/inc/wl_renderer.h DESTINATION include)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/inc/wl_renderer_video.h DESTINATION include)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/inc/wl_renderer_config.h DESTINATION include)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/inc/wl_renderer_video_buffer.h DESTINATION include)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/inc/wl_renderer_define.h DESTINATION include)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include FILES_MATCHING PATTERN "*.h")

# プロトコル変換コマンドの設定
find_program(WAYLAND_SCANNER_EXECUTABLE NAMES wayland-scanner)

set(WAYLAND_PROTOCOLS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/)

# プロトコルのXMLファイルを検索するパスの設定
#find_path(WAYLAND_PROTOCOLS_DIR
#	NAMES linux-dmabuf-unstable-v1.xml
#	PATHS /opt/arene/share/wayland-protocols/unstable/linux-dmabuf/
#	ONLY_CMAKE_FIND_ROOT_PATH
#)

# プロトコルのXMLファイルから必要なヘッダファイルとCソースファイルを生成するカスタムコマンド
add_custom_command(
    OUTPUT  linux-dmabuf-unstable-v1-client-protocol.h
    COMMAND ${WAYLAND_SCANNER_EXECUTABLE} client-header
            < ${WAYLAND_PROTOCOLS_DIR}/linux-dmabuf-unstable-v1.xml
            > ${CMAKE_CURRENT_BINARY_DIR}/linux-dmabuf-unstable-v1-client-protocol.h
    DEPENDS ${WAYLAND_PROTOCOLS_DIR}/linux-dmabuf-unstable-v1.xml
)

add_custom_command(
    OUTPUT  linux-dmabuf-unstable-v1-protocol.c
    COMMAND ${WAYLAND_SCANNER_EXECUTABLE} code
            < ${WAYLAND_PROTOCOLS_DIR}/linux-dmabuf-unstable-v1.xml
            > ${CMAKE_CURRENT_BINARY_DIR}/linux-dmabuf-unstable-v1-protocol.c
    DEPENDS ${WAYLAND_PROTOCOLS_DIR}/linux-dmabuf-unstable-v1.xml
)
