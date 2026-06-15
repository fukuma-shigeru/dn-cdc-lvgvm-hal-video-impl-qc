# --------------------------------------------------------------
# ターゲットのCMakeLists.txt
# --------------------------------------------------------------
project(pfm C)

find_package(PkgConfig)

add_library(${PROJECT_NAME} SHARED pfm_core.c)

target_compile_options(${PROJECT_NAME} PRIVATE
	-fsigned-char
	-fsigned-bitfields
	-funwind-tables
	-rdynamic
)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_VIDEO_STUB})

