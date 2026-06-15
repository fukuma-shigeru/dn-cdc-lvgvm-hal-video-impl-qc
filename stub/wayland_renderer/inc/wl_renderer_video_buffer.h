/*******************************************************************************
    機能名称    ：  Wayland映像描画バッファクラス
    ファイル名称：  wl_renderer_video_buffer.h
*******************************************************************************/
#ifndef	WL_RENDERER_BUFFER_H
#define	WL_RENDERER_BUFFER_H

#include <vector>
#include <map>
#include <wayland-client.h>
#include "video_buf_ctl_public.h"

struct zwp_linux_dmabuf_v1;

namespace wlrenderer
{
class CWaylandRendererVideo;

/*****************************************************************************
 クラス名称：CWaylandRendererVideoBuffer
 処理概要  ：westonに画面を表示するためのバッファクラス
*****************************************************************************/
class CWaylandRendererVideoBuffer {
public:
	explicit CWaylandRendererVideoBuffer(const int32_t index);
	~CWaylandRendererVideoBuffer(void);

	int32_t Initialize(CWaylandRendererVideo * const p_video, const int32_t type, const int32_t format,
		const int32_t width, const int32_t height, const int32_t stride, void * const p_addr, struct wl_shm_pool * const p_wl_pool, int32_t surface_id);
	int32_t InitializeDma(CWaylandRendererVideo * const p_video, const int32_t type, const int32_t format,
		const int32_t width, const int32_t height, const VideoBufCtlBuffer* const vbcBuf, struct zwp_linux_dmabuf_v1 * const p_zwp_linux_dmabuf_v1, const std::vector<int32_t>& surface_ids);
	void Finalize(void);

	/* バッファクリア */
	void Clear(const int32_t color) const;
	/* Wayland Compositorからのバッファ開放 */
	void WaylandReleaseBuffer(const struct wl_buffer * const p_wl_buffer);
	/* リリースコールバック登録 */
	void SetReleaseCallback(void (*p_callback)(void *p_data), void * const p_data) noexcept;
	/* ストライド取得 */
	int32_t GetStride(const uint32_t plane) const noexcept;
	/* Dmaバッファファイルディスクリプタ取得 */
	int32_t GetDmafd(const uint32_t plane) const noexcept;

	/* プレーン数取得 */
	uint32_t GetPlaneCount(void) const noexcept;
	/* プレーンメモリアドレス取得 */
	void *GetPlaneAddr(const uint32_t plane) const noexcept;
	/* ビジー状態取得 */
	bool GetBusy(void) const noexcept;
	/* ビジー状態設定 */
	void SetBusy(const bool busy) noexcept;
	/* wl_buffer取得 */
	struct wl_buffer *GetWlBuffer(const int32_t surface_id) const;
	/* wl_buffer使用設定 */
	void SetWlBufferUse(const int32_t surface_id);

private:
	/* プレーン最大枚数 */
	static const uint32_t kPlaneMax = 3;

	CWaylandRendererVideo *p_video_;

	int32_t index_;
	int32_t type_;
	bool busy_;

	int32_t width_;
	int32_t height_;
	int32_t format_;
	uint32_t plane_count_;

	int32_t stride_[kPlaneMax];
	int32_t dmafd_[kPlaneMax];;

	void *p_addr_[kPlaneMax];

	struct WlBufferData {
		struct wl_buffer *p_wl_buffer_;
		bool	use_;
		WlBufferData()
			:p_wl_buffer_(nullptr)
			,use_(false)
		{}
	};
	std::map<int32_t, struct WlBufferData> wl_buffer_data_;

	void (*p_release_cb_)(void *);
	void *p_user_data_;
};

} /* namespace wlrenderer */

#endif	/* #ifndef	WL_RENDERER_BUFFER_H */
