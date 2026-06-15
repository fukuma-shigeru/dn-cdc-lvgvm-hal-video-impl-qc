/*******************************************************************************
    機能名称    ：  Wayland描画設定クラス
    ファイル名称：  wl_renderer_config.cpp
*******************************************************************************/
#include "wl_renderer_config.h"

#include "wl_renderer_define.h"
#include "wl_renderer_log.h"

namespace wlrenderer
{

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CWaylandRendererConfig::CWaylandRendererConfig(void)
	:buf_type_(0)
	,buf_count_(0U)
	,width_(0U)
	,height_(0U)
	,format_(0)
	,loop_buf_(false)
{
	WLRNDR_LOGV("CWaylandRendererConfig is created. this=%p", this);
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CWaylandRendererConfig::~CWaylandRendererConfig(void)
{
	WLRNDR_LOGV("CWaylandRendererConfig is deleted. this=%p", this);
	ivi_ids_.clear();
}


void CWaylandRendererConfig::SetBufferType(const int32_t type) noexcept
{
	buf_type_ = type;
}

int32_t CWaylandRendererConfig::GetBufferType(void) const noexcept
{
	return buf_type_;
}

void CWaylandRendererConfig::SetBufferCount(const uint32_t count) noexcept
{
	buf_count_ = count;
}

uint32_t CWaylandRendererConfig::GetBufferCount(void) const noexcept
{
	return buf_count_;
}

void CWaylandRendererConfig::SetWidth(const uint32_t width) noexcept
{
	width_ = width;
}

uint32_t CWaylandRendererConfig::GetWidth(void) const noexcept
{
	return width_;
}

void CWaylandRendererConfig::SetHeight(const uint32_t height) noexcept
{
	height_ = height;
}

uint32_t CWaylandRendererConfig::GetHeight(void) const noexcept
{
	return height_;
}

void CWaylandRendererConfig::SetFormat(const int32_t format) noexcept
{
	format_ = format;
}

int32_t CWaylandRendererConfig::GetFormat(void) const noexcept
{
	return format_;
}

void CWaylandRendererConfig::AddSurfaceId(const int32_t iviid)
{
	ivi_ids_.push_back(iviid);
}

void CWaylandRendererConfig::ClearSurfaceId(void) noexcept
{
	ivi_ids_.clear();
}

void CWaylandRendererConfig::GetSurfaceIds(std::vector<int32_t>& ids) const
{
	ids = ivi_ids_;
}

void CWaylandRendererConfig::SetLoopBuffer(const bool loop) noexcept
{
	loop_buf_ = loop;
}

bool CWaylandRendererConfig::GetLoopBuffer(void) const noexcept
{
	return loop_buf_;
}

} /* namespace wlrenderer */

