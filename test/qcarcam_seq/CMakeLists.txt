# # --------------------------------------------------------------
# # ターゲットのCMakeLists.txt
# # --------------------------------------------------------------
project(qcarcam_seq CXX)

# --------------------------------------------------------------
# 依存OSSパッケージ確認
# --------------------------------------------------------------
# pkg-config実行
find_package(PkgConfig)
pkg_check_modules(_dlt REQUIRED automotive-dlt)
pkg_check_modules(_ilmCommon REQUIRED ilmCommon)
pkg_check_modules(_ilmControl REQUIRED ilmControl)
pkg_check_modules(_libdrm REQUIRED libdrm)

# --------------------------------------------------------------
# ターゲット定義(実行ファイル)
# --------------------------------------------------------------
set ( SOURCE
	qcarcam_seq.cpp
	qcarcam_seq_main.cpp
)

# 実行ファイルの生成
add_executable(${PROJECT_NAME} ${SOURCE})

# インクルードパスの設定
target_include_directories(${PROJECT_NAME} PRIVATE
	${_wayland_INCLUDE_DIRS}
	${_libdrm_INCLUDE_DIRS}
	${CMAKE_SYSROOT}/usr/include/qcx
)

# インクルードパスの設定(依存OSSパッケージ)

# コンパイルオプション
target_compile_options(${PROJECT_NAME} PRIVATE 
	-pthread
	-fsigned-char
	-fsigned-bitfields
	-funwind-tables
	-rdynamic
	-Wno-unused-result
)

# コンパイルオプション(デファイン)
#target_compile_definitions(${PROJECT_NAME} PRIVATE -DSIF_DEBUG=2)

# ライブラリのリンク
target_link_libraries(${PROJECT_NAME}
	FileObserver
	${_dlt_LIBRARIES}
	${_ilmCommon_LIBRARIES}
	${_ilmControl_LIBRARIES}
	wlrenderer
	qcxclient
	qcxosal
	gbm
)

# コンパイルオプション(デファイン)
# BEVstep3
if(TIER1_BUILD_TARGET MATCHES "sa8255p-dn-cdc")
	# 1Sボード
	target_compile_options(${PROJECT_NAME} PRIVATE
		-DVHAL_TIER1_TARGET=2
	)
elseif(TIER1_BUILD_TARGET MATCHES "sa8255p-adp-air")
	# ADPボード
	target_compile_options(${PROJECT_NAME} PRIVATE
		-DVHAL_TIER1_TARGET=1
	)
else()
	# x86-64
	target_compile_options(${PROJECT_NAME} PRIVATE
		-DVHAL_TIER1_TARGET=0
	)
endif()


#実行ファイルのインストール
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_VIDEO_HAL_TEST})
