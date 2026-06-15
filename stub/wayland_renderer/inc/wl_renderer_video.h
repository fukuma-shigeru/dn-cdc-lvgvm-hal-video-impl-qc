/*******************************************************************************
    機能名称    ：  Wayland映像描画クラス
    ファイル名称：  wl_renderer_video.h
*******************************************************************************/
#ifndef	WL_RENDERER_VIDEO_H
#define	WL_RENDERER_VIDEO_H

#include <cstdint>
#include <vector>
#include <map>
#include <mutex>
#include <condition_variable>
#include <pthread.h>
#include <wayland-client.h>
#include <ilm/ivi-application-client-protocol.h>
#include "video_buf_ctl_public.h"
#include "wl_renderer_video_buffer.h"

namespace wlrenderer
{
namespace
{
const uint32_t kWaylandBufferMax = 10;	/* ★バッファ最大枚数。後でカメラ等の必要枚数を考慮して変更する */
}

class CWaylandRenderer;
class CWaylandRendererConfig;
struct WlRendererDisplay;

/*****************************************************************************
 クラス名称：CWaylandRendererVideo
 処理概要  ：waylandクライアントとしてwestonに画像を表示する
*****************************************************************************/
class CWaylandRendererVideo {
public:
	enum VideoBufferType {
		VIDEO_BUFFER_TYPE_SHM,
		VIDEO_BUFFER_TYPE_DMA,
	};

	CWaylandRendererVideo(void);
	~CWaylandRendererVideo(void);

	int32_t Initialize(CWaylandRenderer * const p_renderer, const CWaylandRendererConfig &conf);
	void Finalize(void);

	/* バッファ取得 */
	CWaylandRendererVideoBuffer *GetBuffer(void) noexcept;
	/* バッファ送信 */
	int32_t SendBuffer(CWaylandRendererVideoBuffer * const p_buffer, const std::vector<int32_t>& surface_ids);
	/* バッファ返却 */
	void ReturnBuffer(CWaylandRendererVideoBuffer * const p_buffer);

	/* Wayland Compositorとのsync応答 */
	void WaylandSyncDone(void);

protected:

private:
	struct WlRendererVideoWindow {
		int32_t ivi_id;
		int32_t width;
		int32_t height;
		struct wl_surface* p_wl_surface;
		struct ivi_surface* p_ivi_surface;
		
		WlRendererVideoWindow()
			:ivi_id(0)
			,width(0)
			,height(0)
			,p_wl_surface(nullptr)
			,p_ivi_surface(nullptr)
		{}
	};
	pthread_cond_t syncCond_;
	pthread_mutex_t syncMutex_;

	CWaylandRenderer *p_renderer_;
	std::mutex mtx_buffer_;
	std::condition_variable cond_buffer_;
	uint32_t free_buf_count_;
	std::mutex mtx_sync_;
	bool sync_done_;

	int32_t buf_type_;
	bool loop_buf_;
	bool output_error_;
	std::vector<std::unique_ptr<CWaylandRendererVideoBuffer>> buffers_;
	std::map<int32_t, struct WlRendererVideoWindow> windows_;

	VideoBufCtlObj videoBufCtlObj_;
	std::vector<VideoBufCtlBuffer> ctl_buffers_;

	int8_t *p_shm_;
	std::size_t shm_size_;

	/* バッファ作成（wl_shm） */
	int32_t CreateBuffers(const CWaylandRendererConfig &conf);
	/* バッファ作成（linux_dmabuf） */
	int32_t CreateBuffersDma(const CWaylandRendererConfig &conf);
	/* バッファ削除（wl_shm） */
	void DestroyBuffers(void) noexcept;
	/* バッファ削除（linux_dmabuf） */
	void DestroyBuffersDma(void);
	/* Wayland Compositorとのsync要求 */
	int32_t WaylandSync(void);

	/* waylandサーフェス生成 */
	int32_t CreateSurface(const WlRendererDisplay &display, int32_t surface_id);
	/* waylandサーフェス破棄 */
	void DestroySurface(const int32_t surface_id);
	/* waylandサーフェスコミット */
	void CommitSurface(const WlRendererDisplay &display, CWaylandRendererVideoBuffer * const p_buffer, const int32_t surface_id, const WlRendererVideoWindow &window);

};

} /* namespace wlrenderer */

#endif	/* #ifndef	WL_RENDERER_VIDEO_H */
