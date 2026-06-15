# --------------------------------------------------------------
# ターゲットのCMakeLists.txt
# --------------------------------------------------------------
project(qcxosal CXX)

find_package(PkgConfig)

add_library(${PROJECT_NAME} SHARED qcxosal.cpp)

# compile definition setting
target_compile_options(${PROJECT_NAME} PRIVATE
	-fsigned-char
	-fsigned-bitfields
	-funwind-tables
	-rdynamic
	-Wno-unused-result
)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_VIDEO_STUB})
