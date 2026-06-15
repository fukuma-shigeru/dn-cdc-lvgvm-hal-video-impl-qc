/*******************************************************************************
    機能名称    ：  Wayland描画設定クラス
    ファイル名称：  wl_renderer_config.h
*******************************************************************************/
#ifndef	WL_RENDERER_CONFIG_H
#define	WL_RENDERER_CONFIG_H

#include <cstdint>
#include <vector>

namespace wlrenderer
{

/*****************************************************************************
 クラス名称：CWaylandRendererConfig
 処理概要  ：westonに画面を表示するための描画設定値
*****************************************************************************/
class CWaylandRendererConfig {
public:
	CWaylandRendererConfig(void);
	~CWaylandRendererConfig(void);

	void SetBufferType(const int32_t type) noexcept;
	int32_t GetBufferType(void) const noexcept;

	void SetBufferCount(const uint32_t count) noexcept;
	uint32_t GetBufferCount(void) const noexcept;

	void SetWidth(const uint32_t width) noexcept;
	uint32_t GetWidth(void) const noexcept;

	void SetHeight(const uint32_t height) noexcept;
	uint32_t GetHeight(void) const noexcept;

	void SetFormat(const int32_t format) noexcept;
	int32_t GetFormat(void) const noexcept;

	void AddSurfaceId(const int32_t iviid);
	void ClearSurfaceId(void) noexcept;
	void GetSurfaceIds(std::vector<int32_t>& ids) const;

	void SetLoopBuffer(const bool loop) noexcept;
	bool GetLoopBuffer(void) const noexcept;

private:
	int32_t buf_type_;
	uint32_t buf_count_;
	uint32_t width_;
	uint32_t height_;
	int32_t format_;
	std::vector<int32_t> ivi_ids_;
	bool loop_buf_;
};

} /* namespace wlrenderer */

#endif	/* #ifndef	WL_RENDERER_CONFIG_H */
