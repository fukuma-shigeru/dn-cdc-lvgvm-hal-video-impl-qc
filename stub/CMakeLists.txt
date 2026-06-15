# --------------------------------------------------------------
# Required CMake version
# --------------------------------------------------------------
cmake_minimum_required(VERSION 3.14)

# --------------------------------------------------------------
# Project name
# --------------------------------------------------------------
project(sim-stub)

# --------------------------------------------------
# path configuration
# --------------------------------------------------
include(${CMAKE_SOURCE_DIR}/cmake/local.cmake)


# --------------------------------------------------------------------------------------
# add target directories
# --------------------------------------------------------------------------------------
add_subdirectory(common)
#add_subdirectory(gst-launch)
add_subdirectory(ls3ctl)
#add_subdirectory(pfm_lib)
#add_subdirectory(wayland_renderer)
#add_subdirectory(drm)

# BEVstep3
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "core2-64" OR "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
	# x86-64用のみ使用
	add_subdirectory(ccm)
	add_subdirectory(hal)
	add_subdirectory(gbm_lib)
	add_subdirectory(qcarcam)
endif()

# --------------------------------------------------------------------------------------
# CPack configuration
# see https://cmake.org/cmake/help/v3.3/module/CPackDeb.html
# --------------------------------------------------------------------------------------

# CPackを使って、Debianパッケージに変換
set(CPACK_GENERATOR "DEB")
# installのCOMPORNENT毎にパッケージを分けない設定
set(CPACK_DEB_COMPONENT_INSTALL OFF)
# パッケージバージョン
set(CPACK_PACKAGE_VERSION "1.0.0")
# project()で指定した、プロジェクト名 を代入
set(CPACK_PACKAGE_NAME "${CMAKE_PROJECT_NAME}")
# パッケージの提供ベンダー
set(CPACK_PACKAGE_VENDOR "TOYOTA MOTOR CORPORATION")
# メンテナ
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "TOYOTA MOTOR CORPORATION")
# この変数を使用すると、高度なユーザーはcontrol.tar.gzにカスタムスクリプトを追加できます。 
# 典型的な使用法は、conffiles、postinst、postrm、prermのためのものです。
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/postinst;${CMAKE_CURRENT_SOURCE_DIR}/postrm;" )
# debianパッケージのアーキテクチャを変換して設定しなおす。
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "aarch64")
	set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "arm64")
elseif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
	set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
else()
	set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
	set(CPACK_RPM_PACKAGE_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
endif()

set(RELEASE "r0")
# パッケージのファイル名。さらに、install時のCOMPORNENTで指定した名前がサフィックスで付与されます。
set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}-${RELEASE}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE})

include(CPack)

# --------------------------------------------------------------------------------------
# debug information
# --------------------------------------------------------------------------------------
# ソースツリーのトップディレクトリ（トップレベルの CMakeLists.txt があるディレクトリ）
message(STATUS "CMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR}")
# CMAKE_SOURCE_DIR に対応するビルドディレクトリ
message(STATUS "CMAKE_BINARY_DIR=${CMAKE_BINARY_DIR}")
# 現在処理中の CMakeLists.txt があるディレクトリ
message(STATUS "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")
# CMAKE_CURRENT_BINARY_DIR に対応するビルドディレクトリ
message(STATUS "CMAKE_CURRENT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}")
# 現在のプロジェクトのトップディレクトリ（project コマンドを使用した CMakeLists.txt があるディレクトリ）
message(STATUS "PROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}")
# PROJECT_SOURCE_DIR に対応するビルドディレクトリ
message(STATUS "PROJECT_BINARY_DIR=${PROJECT_BINARY_DIR}")
# CMake Cross Compiling variable
message(STATUS "CMAKE_CROSSCOMPILING=${CMAKE_CROSSCOMPILING}")
message(STATUS "CMAKE_SYSTEM_PROCESSOR=${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "CMAKE_HOST_SYSTEM_PROCESSOR=${CMAKE_HOST_SYSTEM_PROCESSOR}")
message(STATUS "CMAKE_C_COMPILER=${CMAKE_C_COMPILER}")
message(STATUS "CMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_INSTALL_SYSCONFDIR=${CMAKE_INSTALL_SYSCONFDIR}")
message(STATUS "CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")

