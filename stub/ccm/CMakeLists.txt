# --------------------------------------------------------------
# ターゲットのCMakeLists.txt
# --------------------------------------------------------------
project(ccm_client C)

find_package(PkgConfig)

add_library(${PROJECT_NAME} SHARED ccm_api.c)

target_include_directories(${PROJECT_NAME} PRIVATE
	../common
)

target_compile_options(${PROJECT_NAME} PRIVATE
	-fsigned-char
	-fsigned-bitfields
	-funwind-tables
	-rdynamic
	-Wno-unused-result
)

target_link_libraries(${PROJECT_NAME}
	stub_common
)


install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_VIDEO_STUB})
#install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/ccm_api_public.h DESTINATION include)
#install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/ccm_public.h DESTINATION include)
#install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/ls1_ctrl_api_public.h DESTINATION include)
#install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/tab_ctrl_api_public.h DESTINATION include)
#install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/AvLanMod.h DESTINATION include)
#install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/AvLanPrt.h DESTINATION include)
#install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/TabPower.h DESTINATION include)
