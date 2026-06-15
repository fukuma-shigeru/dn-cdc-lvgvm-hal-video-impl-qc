# --------------------------------------------------------------
# Required CMake version
# --------------------------------------------------------------
cmake_minimum_required(VERSION 3.14)

# --------------------------------------------------------------
# Project name
# --------------------------------------------------------------
project(dn-cdc-lvgvm-hal-video-impl-qc)

# --------------------------------------------------
# path configuration
# --------------------------------------------------
include(${CMAKE_SOURCE_DIR}/cmake/common.cmake)

include_directories(BEFORE ${CMAKE_SOURCE_DIR}/stub/common)
if(TIER1_BUILD_TARGET MATCHES "x86-ubuntu")
	include_directories(${CMAKE_SOURCE_DIR}/stub/qcarcam)
endif()
include_directories(${CMAKE_SOURCE_DIR}/stub/ccm)
include_directories(${CMAKE_SOURCE_DIR}/stub/hal)
include_directories(${CMAKE_SOURCE_DIR}/stub/ls3ctl/api/include)

link_directories(${CMAKE_BINARY_DIR}/stub/hal)
link_directories(${CMAKE_BINARY_DIR}/stub/ccm)
link_directories(${CMAKE_BINARY_DIR}/stub/gbm_lib)

# --------------------------------------------------------------------------------------
# add target directories
# --------------------------------------------------------------------------------------
add_subdirectory(stub)
add_subdirectory(video_hal)
add_subdirectory(video_hal_control)

if(NOT CMAKE_DISABLE_TESTCODE_BUILD)
  add_subdirectory(test)
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
# Set the architecture of the Debian package.
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
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
message(STATUS "CMAKE_INSTALL_BINDIR=${CMAKE_INSTALL_BINDIR}")
message(STATUS "CMAKE_INSTALL_LIBDIR=${CMAKE_INSTALL_LIBDIR}")
message(STATUS "CMAKE_INSTALL_VIDEO_STUB=${CMAKE_INSTALL_VIDEO_STUB}")

