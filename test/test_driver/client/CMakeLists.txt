# --------------------------------------------------------------
# ターゲットのCMakeLists.txt
# --------------------------------------------------------------
cmake_minimum_required(VERSION 3.16)
project(halcp_video_client C)

# --------------------------------------------------------------
# 依存OSSパッケージ確認
# --------------------------------------------------------------
# pkg-config実行
find_package(PkgConfig)
pkg_check_modules(_wayland_client REQUIRED wayland-client)
pkg_check_modules(_libegl REQUIRED egl)
pkg_check_modules(_libglesv2 REQUIRED glesv2)
pkg_check_modules(_libwayland-egl REQUIRED wayland-egl)
pkg_check_modules(_ivi-application REQUIRED ivi-application)
pkg_check_modules(_libpng REQUIRED libpng)

# --------------------------------------------------------------
# ターゲット定義(共有ライブラリ)
# --------------------------------------------------------------
# 共有ライブラリを生成
add_executable(${PROJECT_NAME}
	${PROJECT_NAME}.c
)

# インクルードパスの設定
target_include_directories(${PROJECT_NAME} PRIVATE
	${CMAKE_SOURCE_DIR}/include
	${_wayland_client_INCLUDE_DIRS}
	${_libegl_INCLUDE_DIRS}
	${_libglesv2_INCLUDE_DIRS}
	${_libwayland-egl_INCLUDE_DIRS}
	${_ivi-application_INCLUDE_DIRS}
	${_libpng_INCLUDE_DIRS}
)

# インクルードパスの設定(依存OSSパッケージ)
target_include_directories(${PROJECT_NAME} BEFORE PRIVATE ${_wayland_INCLUDE_DIRS})

# コンパイルオプション
target_compile_options(${PROJECT_NAME} PRIVATE -fsigned-char -Wno-clobbered)

# コンパイルオプション(デファイン)
#target_compile_definitions(${PROJECT_NAME} PRIVATE -DSIF_DEBUG=2)

# ライブラリのリンク
target_link_libraries(${PROJECT_NAME}
	${_wayland_client_LIBRARIES}
	${_libegl_LIBRARIES}
	${_libglesv2_LIBRARIES}
	${_libwayland-egl_LIBRARIES}
	${_ivi-application_LIBRARIES}
	${_libpng_LIBRARIES}
)

# ライブラリのリンク(依存OSSパッケージ)
target_link_libraries(${PROJECT_NAME} ${_wayland_LIBRARIES})

# Install target
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_VIDEO_HAL_TEST}/certification-program)
