# --------------------------------------------------------------
# ターゲットのCMakeLists.txt
# --------------------------------------------------------------
project(video_hal_control CXX)

set	( SOURCE
	./src/vhal_control.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE})

target_link_libraries(${PROJECT_NAME}
	video_hal
	halcomm
	pthread
	dlt
)

#実行ファイルのインストール
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
