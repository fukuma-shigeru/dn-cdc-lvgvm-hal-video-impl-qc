# --------------------------------------------------------------
# ターゲットのCMakeLists.txt
# --------------------------------------------------------------
project(stub_common C)

find_package(PkgConfig)

add_library(${PROJECT_NAME} SHARED stub_common.cpp)

target_compile_options(${PROJECT_NAME} PRIVATE
	-fsigned-char
	-fsigned-bitfields
	-funwind-tables
	-rdynamic
)

target_link_libraries(${PROJECT_NAME}
	FileObserver
)


install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_VIDEO_STUB})
#install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/stub_common.h DESTINATION include)
