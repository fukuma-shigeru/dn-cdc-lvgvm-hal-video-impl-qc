# # --------------------------------------------------------------
# # ターゲットのCMakeLists.txt
# # --------------------------------------------------------------
project(render_image CXX)

# --------------------------------------------------------------
# 依存OSSパッケージ確認
# --------------------------------------------------------------
# pkg-config実行
find_package(PkgConfig)
pkg_check_modules(_wayland REQUIRED wayland-client)
pkg_check_modules(_libdrm REQUIRED libdrm)
pkg_check_modules(_libgbm REQUIRED gbm)
pkg_check_modules(_libegl REQUIRED egl)
pkg_check_modules(_libglesv2 REQUIRED glesv2)
pkg_check_modules(_libwayland-egl REQUIRED wayland-egl)
pkg_check_modules(_libpng REQUIRED libpng)

# --------------------------------------------------------------
# ターゲット定義(実行ファイル)
# --------------------------------------------------------------
set ( SOURCE
	src/render_image.cpp
)

# 実行ファイルの生成
add_executable(${PROJECT_NAME} ${SOURCE})

# インクルードパスの設定
target_include_directories(${PROJECT_NAME} PRIVATE
	${CMAKE_CURRENT_BINARY_DIR}
	${_wayland_INCLUDE_DIRS}
	${_libdrm_INCLUDE_DIRS}
	${_libgbm_INCLUDE_DIRS}
	${_libegl_INCLUDE_DIRS}
	${_libglesv2_INCLUDE_DIRS}
	${_libwayland-egl_INCLUDE_DIRS}
	${_libpng_INCLUDE_DIRS}
	${CMAKE_SOURCE_DIR}/util_wl/include
)

# インクルードパスの設定(依存OSSパッケージ)
target_include_directories(${PROJECT_NAME} BEFORE PRIVATE ${_wayland_INCLUDE_DIRS})

# コンパイルオプション
target_compile_options(${PROJECT_NAME} PRIVATE -fsigned-char -Wall)

# コンパイルオプション(デファイン)
#target_compile_definitions(${PROJECT_NAME} PRIVATE -DSIF_DEBUG=2)
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "core2-64" OR "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
	# x86-64用
	target_compile_options(${PROJECT_NAME} PRIVATE -DRENDERER_SHM_ONLY=true)
else()
	# 実機用 (cortexa78cなど)
	target_compile_options(${PROJECT_NAME} PRIVATE -DRENDERER_SHM_ONLY=false)
endif()

# ライブラリのリンク
target_link_libraries(${PROJECT_NAME}
	${_wayland_LIBRARIES}
	${_libdrm_LIBRARIES}
#	${_libgbm_LIBRARIES}
	${_libegl_LIBRARIES}
	${_libglesv2_LIBRARIES}
	${_libwayland-egl_LIBRARIES}
	${_libpng_LIBRARIES}
	wlrenderer
	gbm
)

#実行ファイルのインストール
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
