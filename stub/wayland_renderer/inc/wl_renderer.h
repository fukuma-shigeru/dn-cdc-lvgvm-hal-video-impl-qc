/*******************************************************************************
    機能名称    ：  Wayland描画クラス
    ファイル名称：  wl_renderer.h
*******************************************************************************/
#ifndef	WL_RENDERER_H
#define	WL_RENDERER_H

#include <vector>
#include <list>
#include <string>
#include <thread>
#include <mutex>
#include <semaphore.h>
#include <wayland-client.h>
#include <ilm/ivi-application-client-protocol.h>

struct zwp_linux_dmabuf_v1;

namespace wlrenderer
{

class CWaylandRendererConfig;
class CWaylandRendererVideo;
class CWaylandRendererOutputListener;
class CWaylandRendererOutputNotify;

struct WlRendererOutput {
	int32_t width;
	int32_t height;
	std::string make;
	std::string model;
	struct wl_output *p_wl_output;
	WlRendererOutput()
		:width(0)
		,height(0)
		,p_wl_output(nullptr)
	{}
};

struct WlRendererDisplay {
	struct wl_display *p_wl_display;
	struct wl_registry *p_wl_registry;
	struct wl_compositor *p_wl_compositor;
	struct ivi_application *p_ivi_application;
	struct wl_shm *p_wl_shm;
	struct zwp_linux_dmabuf_v1 *p_zwp_linux_dmabuf_v1;
	std::list<std::unique_ptr<WlRendererOutput>> output_list;			/* 出力先リスト */
	sem_t semDisp;

	WlRendererDisplay()
		:p_wl_display(nullptr)
		,p_wl_registry(nullptr)
		,p_wl_compositor(nullptr)
		,p_ivi_application(nullptr)
		,p_wl_shm(nullptr)
		,p_zwp_linux_dmabuf_v1(nullptr)
	{}
};

/*****************************************************************************
 クラス名称：CWaylandRenderer
 処理概要  ：waylandクライアントとしてwestonに接続する。
*****************************************************************************/
class CWaylandRenderer {
	friend CWaylandRendererOutputNotify;		/* AddOutputアクセス用 */
public:
	CWaylandRenderer(void);
	~CWaylandRenderer(void);

	int32_t Initialize(void);
	void Finalize(void);

	/* Videoインスタンスの作成 */
	CWaylandRendererVideo *CreateRendererVideo(const CWaylandRendererConfig &conf);
	/* Videoインスタンスの除去 */
	void RemoveRendererVideo(CWaylandRendererVideo * const p_video);
	/* WlRendererDisplay情報の取得 */
	friend WlRendererDisplay *GetDisplayInfo(CWaylandRenderer* const p_wl_renderer) noexcept;
	/* 出力デバイス検知リスナー登録 */
	int32_t RegisterOutputListener(CWaylandRendererOutputListener* const p_listener);
	/* 出力デバイス検知リスナー解除 */
	void ClearOutputListener(void) noexcept;

protected:

private:
	int shutdown_fd_;
	bool thread_running_;
	std::thread event_thread_;
	std::mutex mutex_output_list_;		/* display_.output_list用 */

	WlRendererDisplay display_;

	std::vector<std::unique_ptr<CWaylandRendererVideo>> video_clients_;
	bool finalizing_;

	CWaylandRendererOutputListener* p_output_listener_;
	std::vector<std::unique_ptr<CWaylandRendererOutputNotify>> output_notify_list;


	/* Wayland接続開始 */
	int32_t InitWayland(void);
	/* Wayland接続終了 */
	void DeinitWayland(void);
	/* Waylandイベントスレッドメイン関数 */
	void WaylandEventThreadMain(void);
	/* 出力先追加 */
	void AddOutput(const struct WlRendererOutput& output);
};


/*****************************************************************************
 クラス名称：CWaylandRendererOutputListener
 処理概要  ：出力デバイス検知リスナー
*****************************************************************************/
class CWaylandRendererOutputListener {
public:
	CWaylandRendererOutputListener() {
	}
	virtual ~CWaylandRendererOutputListener() {
	}

	virtual void NotifyOutputAdded(const int32_t width, const int32_t height, const std::string& model, const std::string& make) = 0;
};

} /* namespace wlrenderer */

#endif	/* #ifndef	WL_RENDERER_H */
