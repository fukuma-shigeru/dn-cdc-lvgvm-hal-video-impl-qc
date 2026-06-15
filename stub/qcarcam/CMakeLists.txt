# --------------------------------------------------------------
# ターゲットのCMakeLists.txt
# --------------------------------------------------------------
project(qcxclient CXX)

find_package(PkgConfig)

# --------------------------------------------------------------------------------------
# add target directories
# --------------------------------------------------------------------------------------
add_subdirectory(qcxosal)

add_library(${PROJECT_NAME} SHARED qcarcam_stub_api.cpp)

# target_include_directories
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SYSROOT}/usr/include/qcx)

# compile definition setting
target_compile_options(${PROJECT_NAME} PRIVATE
	-fsigned-char
	-fsigned-bitfields
	-funwind-tables
	-rdynamic
	-Wno-unused-result
)

# library setting
target_link_libraries(${PROJECT_NAME}
	FileObserver
	sifutil
	timer
	stub_common
	png
)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_VIDEO_STUB})
#install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/qcarcam.h DESTINATION include)
#install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/qcarcam_types.h DESTINATION include)
