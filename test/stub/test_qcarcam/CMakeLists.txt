# # --------------------------------------------------------------
# # ターゲットのCMakeLists.txt
# # --------------------------------------------------------------
project(test_qcarcam CXX)

# --------------------------------------------------------------
# 依存OSSパッケージ確認
# --------------------------------------------------------------
# pkg-config実行
find_package(PkgConfig)

# --------------------------------------------------------------
# ターゲット定義(実行ファイル)
# --------------------------------------------------------------
set ( SOURCE
	src/qcarcam_test.cpp
)

# 実行ファイルの生成
add_executable(${PROJECT_NAME} ${SOURCE})

# インクルードパスの設定
target_include_directories(${PROJECT_NAME} PRIVATE
	${PKG_CONFIG_SYSROOT_DIR}/usr/include
	${PKG_CONFIG_SYSROOT_DIR}/usr/include/drm
	${CMAKE_SYSROOT}/usr/include/qcx
)

# インクルードパスの設定(依存OSSパッケージ)

# コンパイルオプション
target_compile_options(${PROJECT_NAME} PRIVATE -fsigned-char -Wall)

# コンパイルオプション(デファイン)
#target_compile_definitions(${PROJECT_NAME} PRIVATE -DSIF_DEBUG=2)

# ライブラリのリンク
target_link_libraries(${PROJECT_NAME}
	qcxclient
	qcxosal
)

#実行ファイルのインストール
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_VIDEO_HAL_TEST})
