# 新しいフォルダレイアウト版に対応する場合は、ONにする。
# ON: /opt/dc-ivi-pf
# OFF: /usr
option(TIER1_NEW_DIRLAYOUT "use new directory layout" ON)

include(${CMAKE_SOURCE_DIR}/cmake/Modules/arene.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/Modules/dten.cmake)

set(CMAKE_INSTALL_VIDEO_STUB "/usr/local/share/stub/video_hal_svc" CACHE PATH "video stub install path" FORCE)

# ログレベル設定
add_compile_options(-D__SPF_LOG_LEVEL__=3)
