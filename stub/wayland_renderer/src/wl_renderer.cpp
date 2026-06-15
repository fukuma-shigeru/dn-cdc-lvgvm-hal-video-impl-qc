/*******************************************************************************
    機能名称    ：  Wayland描画クラス
    ファイル名称：  wl_renderer.cpp
*******************************************************************************/
#include "wl_renderer.h"

#include <algorithm>
#include <iostream>
#include <poll.h>
#include <sys/eventfd.h>
#include <cstdint>

#include "wl_renderer_define.h"
#include "wl_renderer_log.h"
#include "wl_renderer_config.h"
#include "wl_renderer_video.h"

#if 0 //Bev3 8/26
#include <linux-dmabuf-unstable-v1-client-protocol.h>
#endif

namespace wlrenderer
{

namespace
{

constexpr int32_t kWaylandInitRetryCount{600};			/* wayland接続のリトライ回数 */
constexpr uint32_t kWaylandInitRetryWait{50U};			/* wayland接続のリトライ待ち時間(ms) */
constexpr int32_t kWaylandEventThreadErrorMax{20};		/* waylandイベントスレッドで連続エラー最大数。これを超えたらスレッド終了 */

} /* namespace */


/*****************************************************************************
 クラス名称：CWaylandRendererOutputNotify
 処理概要  ：出力デバイス検知通知
*****************************************************************************/
class CWaylandRendererOutputNotify {
public:
	CWaylandRendererOutputNotify(CWaylandRenderer* const p_wl_renderer, struct wl_output* const p_wl_output)
		:p_wl_renderer_(p_wl_renderer)
		,geometry_(false)
		,mode_(false)
	{
		output_data_.p_wl_output = p_wl_output;
	}
	~CWaylandRendererOutputNotify(void) = default;
  	CWaylandRendererOutputNotify(const CWaylandRendererOutputNotify& src) = delete;
	CWaylandRendererOutputNotify& operator=(const CWaylandRendererOutputNotify& src) & = delete;
	CWaylandRendererOutputNotify(CWaylandRendererOutputNotify&& src) = delete;
	CWaylandRendererOutputNotify& operator=(CWaylandRendererOutputNotify&& src) & = delete;

	void SetOutputGeometry(const std::string &make, const std::string &model)
	{
		output_data_.make  = make;
		output_data_.model = model;
		geometry_ = true;
		/* SetOutputModeが既にコールされていれば通知する */
		if ( true == mode_ )
		{
			p_wl_renderer_->AddOutput(output_data_);
		}
	}
	void SetOutputMode(const int32_t width, const int32_t height)
	{
		output_data_.width  = width;
		output_data_.height = height;
		mode_   = true;

		/* SetOutputGeometryが既にコールされていれば通知する */
		if ( true == geometry_ )
		{
			p_wl_renderer_->AddOutput(output_data_);
		}
	}

private:
	CWaylandRenderer* p_wl_renderer_;
	struct WlRendererOutput output_data_;
	bool geometry_;
	bool mode_;
};


namespace
{

extern "C" {
static void RegistryHandleGlobal(void * const p_data, struct wl_registry * const p_wl_registry, const uint32_t name, const char * const p_interface, const uint32_t version);
static void RegistryHandleGlobalRemove(void * const p_data, struct wl_registry * const p_wl_registry, const uint32_t name);
static void OutputHandleGeometry(void * const p_data, struct wl_output * const p_wl_output, const int32_t x, const int32_t y,
	const int32_t physical_width, const int32_t physical_height, const int32_t subpixel, const char * const make, const char * const model, const int32_t transform);
static void OutputHandleMode(void * const p_data, struct wl_output * const p_wl_output,
	const uint32_t flags, const int32_t width, const int32_t height, const int32_t refresh);
static void OutputHandleDone(void * const p_data, struct wl_output * const p_wl_output);
static void OutputHandleScale(void * const p_data, struct wl_output * const p_wl_output, const int32_t scale);
}


/*****************************************************************************
 処理概要：	wl_registryオブジェクトglobalイベント登録用関数
 引数    ：	void *p_data						(i)ユーザーデータポインタ
           	struct wl_registry *p_wl_registry	(i)wl_registryオブジェクトポインタ
           	uint32_t name						(i)名前(ID)
           	const char *p_interface				(i)インターフェース名
           	uint32_t version					(i)バージョン
 戻り値  ：	なし
*****************************************************************************/
static void RegistryHandleGlobal(void * const p_data, struct wl_registry * const p_wl_registry, const uint32_t name, const char * const p_interface, const uint32_t version)
{
	CWaylandRenderer* const p_wl_renderer{static_cast<CWaylandRenderer*>(p_data)};
	WlRendererDisplay* const p_disp{GetDisplayInfo(p_wl_renderer)};
	constexpr std::size_t interface_max_len{32U};

	WLRNDR_LOGI("interface = [%s]", p_interface);

	if ((strnlen(p_interface, interface_max_len) == strnlen("wl_compositor", interface_max_len)) &&
		(0 == strncmp(p_interface, "wl_compositor", sizeof("wl_compositor")))) {
		p_disp->p_wl_compositor = static_cast<struct wl_compositor *>(wl_registry_bind(p_wl_registry, name, &wl_compositor_interface, version));
	} else if ((strnlen(p_interface, interface_max_len) == strnlen("wl_output", interface_max_len)) &&
		(0 == strncmp(p_interface, "wl_output", sizeof("wl_output")))) {
		struct wl_output* const p_wl_output{static_cast<struct wl_output *>(wl_registry_bind (p_wl_registry, name, &wl_output_interface, version))};
		std::unique_ptr<CWaylandRendererOutputNotify> p_output_notify{std::make_unique<CWaylandRendererOutputNotify>(p_wl_renderer, p_wl_output)};
		static constexpr struct wl_output_listener g_output_listener{
			&OutputHandleGeometry,
			&OutputHandleMode,
			&OutputHandleDone,
			&OutputHandleScale
		};		
		const int32_t ret{wl_output_add_listener(p_wl_output, &g_output_listener, p_output_notify.release())};
		if (-1 == ret)
		{
			WLRNDR_LOGW("wl_output_add_listener[%lu] error.", p_disp->output_list.size());
		}
	} else if ((strnlen(p_interface, interface_max_len) == strnlen("ivi_application", interface_max_len)) &&
		(0 == strncmp(p_interface, "ivi_application", sizeof("ivi_application")))) {
		p_disp->p_ivi_application = static_cast<struct ivi_application *>(wl_registry_bind(p_wl_registry, name, &ivi_application_interface, version));
	} else if ((strnlen(p_interface, interface_max_len) == strnlen("wl_shm", interface_max_len)) &&
		(0 == strncmp(p_interface, "wl_shm", sizeof("wl_shm")))) {
		p_disp->p_wl_shm = static_cast<struct wl_shm *>(wl_registry_bind(p_wl_registry, name, &wl_shm_interface, version));

	}
#if 0 //Bev3 8/26 
	else if ((strnlen(p_interface, interface_max_len) == strnlen("zwp_linux_dmabuf_v1", interface_max_len)) &&
		(0 == strncmp(p_interface, "zwp_linux_dmabuf_v1", sizeof("zwp_linux_dmabuf_v1")))) {
		p_disp->p_zwp_linux_dmabuf_v1 = static_cast<struct zwp_linux_dmabuf_v1 *>(wl_registry_bind(p_wl_registry, name, &zwp_linux_dmabuf_v1_interface, version));
	}
#endif
	else
	{
		/* 処理なし */
	}
}

/*****************************************************************************
 処理概要：	wl_registryオブジェクトglobal_removeイベント登録用関数
 引数    ：	void *p_data						(i)ユーザーデータポインタ
           	struct wl_registry *p_wl_registry	(i)wl_registryオブジェクトポインタ
           	uint32_t name						(i)名前(ID)
 戻り値  ：	なし
*****************************************************************************/
static void RegistryHandleGlobalRemove(void * const p_data, struct wl_registry * const p_wl_registry, const uint32_t name)
{
	WLRNDR_LOGI("RegistryHandleGlobalRemove is not implemented.");
}

/*****************************************************************************
 処理概要：	wl_outputオブジェクトgeometryイベント登録用関数
 引数    ：	void *p_data					(i)ユーザーデータポインタ
           	struct wl_output *p_wl_output	(i)wl_outputオブジェクトポインタ
           	int x							(i)x座標
           	int y							(i)y座標
           	int physical_width				(i)実ハードの幅
           	int physical_height				(i)実ハードの高さ
           	int subpixel					(i)サブピクセル
           	const char *make				(i)製造者
           	const char *model				(i)モデル
           	int transform					(i)変換
 戻り値  ：	なし
*****************************************************************************/
static void OutputHandleGeometry(void * const p_data, struct wl_output * const p_wl_output, const int32_t x, const int32_t y,
	const int32_t physical_width, const int32_t physical_height, const int32_t subpixel, const char * const make, const char * const model, const int32_t transform)
{
	WLRNDR_LOGI("OutputHandleGeometry x=%d, y=%d, phys(%dx%d), pix=%d, make[%s], model[%s], trans=%d", 
		x, y, physical_width, physical_height, subpixel, make, model, transform);

	CWaylandRendererOutputNotify* const p_output_notify{static_cast<CWaylandRendererOutputNotify*>(p_data)};
	p_output_notify->SetOutputGeometry(make, model);
}

/*****************************************************************************
 処理概要：	wl_outputオブジェクトmodeイベント登録用関数
 引数    ：	void *p_data					(i)ユーザーデータポインタ
           	struct wl_output *p_wl_output	(i)wl_outputオブジェクトポインタ
           	uint32_t flags					(i)フラグ
           	int width						(i)幅
           	int height						(i)高さ
          	int refresh						(i)リフレッシュレート
 戻り値  ：	なし
*****************************************************************************/
static void OutputHandleMode(void * const p_data, struct wl_output * const p_wl_output,
	const uint32_t flags, const int32_t width, const int32_t height, const int32_t refresh)
{
	WLRNDR_LOGI("OutputHandleMode flags=%u, (%dx%d), refresh=%d", flags, width, height, refresh);

	if (0U != (flags & static_cast<uint32_t>(WL_OUTPUT_MODE_CURRENT)))
	{
		CWaylandRendererOutputNotify* const p_output_notify{static_cast<CWaylandRendererOutputNotify*>(p_data)};
		p_output_notify->SetOutputMode(width, height);
	}
}

/*****************************************************************************
 処理概要：	wl_outputオブジェクトdoneイベント登録用関数
 引数    ：	void *p_data					(i)ユーザーデータポインタ
           	struct wl_output *p_wl_output	(i)wl_outputオブジェクトポインタ
 戻り値  ：	なし
*****************************************************************************/
static void OutputHandleDone(void * const p_data, struct wl_output * const p_wl_output)
{
	WLRNDR_LOGI("OutputHandleDone is not implemented.");
}

/*****************************************************************************
 処理概要：	wl_outputオブジェクトscaleイベント登録用関数
 引数    ：	void *p_data					(i)ユーザーデータポインタ
           	struct wl_output *p_wl_output	(i)wl_outputオブジェクトポインタ
           	int32_t scale					(i)スケール
 戻り値  ：	なし
*****************************************************************************/
static void OutputHandleScale(void * const p_data, struct wl_output * const p_wl_output, const int32_t scale)
{
	WLRNDR_LOGI("OutputHandleScale is not implemented.");
}

} /* namespace */





/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CWaylandRenderer::CWaylandRenderer(void)
	:shutdown_fd_(-1)
	,thread_running_(false)
	,finalizing_(false)
	,p_output_listener_(nullptr)
{
	WLRNDR_LOGV("CWaylandRenderer is created. this=%p", this);
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CWaylandRenderer::~CWaylandRenderer(void)
{
	WLRNDR_LOGV("CWaylandRenderer is deleted. this=%p", this);
	Finalize();
}

/*****************************************************************************
 処理概要：	初期化処理（内部リソースの確保、weston接続）
 引数    ：	なし
 戻り値  ：	処理結果
           		WL_RENDERER_ERR_THREAD			スレッドエラー
           		WL_RENDERER_ERR_WAYLAND_CLIENT	waylandクライアント処理エラー
           		WL_RENDERER_ERR_FD				ファイルディスクリプタエラー
           		WL_RENDERER_SUCCESS				正常終了
*****************************************************************************/
int32_t CWaylandRenderer::Initialize(void)
{
	int32_t ret;

	if (true == thread_running_)
	{
		WLRNDR_LOGE("wayland client is already initialized.");
		return WL_RENDERER_ERR_WAYLAND_CLIENT;
	}

	ret = InitWayland();
	if (0 > ret)
	{
		WLRNDR_LOGE("InitWayland error. ret=%d", ret);
		return ret;
	}

	shutdown_fd_ = eventfd(0U, EFD_CLOEXEC | EFD_NONBLOCK);
	if (-1 == shutdown_fd_)
	{
		WLRNDR_LOGE("eventfd() error.");
		return WL_RENDERER_ERR_FD;
	}

	/* waylandイベントスレッド開始 */
	try
	{
		event_thread_ = std::thread(&CWaylandRenderer::WaylandEventThreadMain, this);
	}
	catch (std::exception &ex)
	{
		WLRNDR_LOGE("std:thread exception received. [%s]", ex.what());
		return WL_RENDERER_ERR_THREAD;
	}

	thread_running_ = true;
	return WL_RENDERER_SUCCESS;
}

/*****************************************************************************
 処理概要：	終了処理（内部リソースの解放、weston切断）
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CWaylandRenderer::Finalize(void)
{
	p_output_listener_ = nullptr;

	/* このWaylandClientをもとに作ったCWaylandRendererVideoが残っていたらすべて削除 */
	finalizing_ = true;
	video_clients_.clear();


	/* waylandイベントスレッド終了 */
	if (true == thread_running_)
	{
		if (shutdown_fd_ > -1)
		{
			ssize_t size;
		    constexpr uint64_t buf{1U};
			do
			{
				size = write(shutdown_fd_, &buf, sizeof buf);
			}
		    while ((size == -1) && (errno == EINTR));

			event_thread_.join();
		}
	}

	if (shutdown_fd_ > -1)
	{
		(void)close(shutdown_fd_);
		shutdown_fd_ = -1;
	}

	DeinitWayland();

	output_notify_list.clear();

	thread_running_ = false;
	finalizing_ = false;
}

/*****************************************************************************
 処理概要：	Videoインスタンスの作成
 引数    ：	CWaylandRendererConfig &conf	(i)Video設定情報
 戻り値  ：	Videoインスタンスポインタ
*****************************************************************************/
CWaylandRendererVideo *CWaylandRenderer::CreateRendererVideo(const CWaylandRendererConfig &conf)
{
	int32_t ret;

//	WLRNDR_LOGI("CreateRendererVideo type=%d, %dx(%dx%d) format=%x id=(%d, %d)",
//		conf.buf_type, conf.buf_count, conf.width, conf.height, conf.format, conf.ivi_id_front, conf.ivi_id_rear);

	std::unique_ptr<CWaylandRendererVideo> p_video{std::make_unique<CWaylandRendererVideo>()};

	ret = p_video->Initialize(this, conf);
	if (0 > ret)
	{
		p_video = nullptr;
		return nullptr;
	}

	CWaylandRendererVideo *const p_video_tmp{p_video.get()};
	video_clients_.push_back(std::move(p_video));
	return p_video_tmp;
}

/*****************************************************************************
 処理概要：	Videoインスタンスの除去
 引数    ：	CWaylandRendererVideo *p_video	(i)Videoインスタンス
 戻り値  ：	なし
*****************************************************************************/
void CWaylandRenderer::RemoveRendererVideo(CWaylandRendererVideo * const p_video)
{
	if (true == finalizing_)
	{
		return;
	}

	const auto itr_video = std::find_if(video_clients_.begin(), video_clients_.end(),
		[p_video](const std::unique_ptr<CWaylandRendererVideo> &video_client) noexcept { return video_client.get() == p_video; });
	if (itr_video == video_clients_.end())
	{
		WLRNDR_LOGW("CWaylandRendererVideo is not found.");
	}
	else
	{
		(void)video_clients_.erase(itr_video);
	}
}

/*****************************************************************************
 処理概要：	WlRendererDisplay情報の取得
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
WlRendererDisplay *GetDisplayInfo(CWaylandRenderer* const p_wl_renderer) noexcept
{
	return &p_wl_renderer->display_;
}

/*****************************************************************************
 処理概要：	出力デバイス検知リスナー登録
 引数    ：	CWaylandRendererOutputListener* p_listener		(i)リスナーポインタ
 戻り値  ：	処理結果
           		WL_RENDERER_ERR				異常終了
           		WL_RENDERER_ERR_PARAM		パラメータ不正
           		WL_RENDERER_SUCCESS			正常終了
*****************************************************************************/
int32_t CWaylandRenderer::RegisterOutputListener(CWaylandRendererOutputListener* const p_listener)
{
	if (nullptr == p_listener)
	{
		WLRNDR_LOGE("p_listener is null");
		return WL_RENDERER_ERR_PARAM;
	}

	int32_t ret{WL_RENDERER_SUCCESS};

	if (nullptr == p_output_listener_)
	{
		/* すでに存在する出力先の通知 */
		const std::lock_guard<std::mutex> lock_output{mutex_output_list_};
		for (auto itr_output = display_.output_list.begin(); itr_output != display_.output_list.end(); ++itr_output)
		{
//			if ( true == itr_output->initialized )
//			{
				p_listener->NotifyOutputAdded((*itr_output)->width, (*itr_output)->height, (*itr_output)->model, (*itr_output)->make);
//			}
		}
		p_output_listener_ = p_listener;
	}
	else
	{
		WLRNDR_LOGW("output listener already registerd");
		ret = WL_RENDERER_ERR;
	}
	return ret;
}

/*****************************************************************************
 処理概要：	出力デバイス検知リスナー解除
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CWaylandRenderer::ClearOutputListener(void) noexcept
{
	p_output_listener_ = nullptr;
}

/*****************************************************************************
 処理概要：	Wayland接続開始
 引数    ：	なし
 戻り値  ：	処理結果
           		WL_RENDERER_ERR_WAYLAND		waylandプロトコルエラー
           		WL_RENDERER_SUCCESS			正常終了
*****************************************************************************/
int32_t CWaylandRenderer::InitWayland(void)
{
	int32_t ret;
	int32_t count;

	count = 0;
	while (true)
	{
		display_.p_wl_display = wl_display_connect(nullptr);
		if (nullptr != display_.p_wl_display)
		{
			WLRNDR_LOGI("wl_display_connect() success. retry count(%d)", count);
			break;
		}

		count ++;
		if (count >= kWaylandInitRetryCount)
		{
			WLRNDR_LOGE("wl_display_connect() retry count(%d) over.", count);
			return WL_RENDERER_ERR_WAYLAND;
		}

		WLRNDR_LOGI("wl_display_connect() error. retry (%d)", count);
		(void)usleep(kWaylandInitRetryWait * 1000U);
	}

	display_.p_wl_registry = wl_display_get_registry(display_.p_wl_display);
	if (nullptr == display_.p_wl_registry)
	{
		WLRNDR_LOGE("wl_display_get_registry() error.");
		return WL_RENDERER_ERR_WAYLAND;
	}

	static constexpr struct wl_registry_listener g_registry_listener{
		&RegistryHandleGlobal,
		&RegistryHandleGlobalRemove
	};
	ret = wl_registry_add_listener(display_.p_wl_registry, &g_registry_listener, this);
	if (-1 == ret)
	{
		WLRNDR_LOGE("wl_display_get_registry() error.");
		return WL_RENDERER_ERR_WAYLAND;
	}

	/* wl_registryイベントのため */
	ret = wl_display_roundtrip(display_.p_wl_display);
	if (-1 == ret)
	{
		WLRNDR_LOGE("wl_display_get_registry() error.");
		return WL_RENDERER_ERR_WAYLAND;
	}

	/* wl_outputイベントのため */
	ret = wl_display_roundtrip(display_.p_wl_display);
	if (-1 == ret)
	{
		WLRNDR_LOGE("wl_display_get_registry() error.");
		return WL_RENDERER_ERR_WAYLAND;
	}

	ret = sem_init(&display_.semDisp, 0, 1U);
	if (0 > ret)
	{
		WLRNDR_LOGE("sem_init(semDisp) error %d", ret);
		return WL_RENDERER_ERR_WAYLAND;
	}

	return WL_RENDERER_SUCCESS;
}

/*****************************************************************************
 処理概要：	Wayland接続終了
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CWaylandRenderer::DeinitWayland(void)
{
	auto it_output     = display_.output_list.begin();
	const auto it_output_end = display_.output_list.end();
	for ( ; it_output != it_output_end; ++it_output)
	{
		WlRendererOutput * const p_output{it_output->get()};
		wl_output_destroy(p_output->p_wl_output);
	}
	display_.output_list.clear();

	int32_t ret{0};
	ret = sem_destroy(&display_.semDisp);
	if (0 > ret)
	{
		WLRNDR_LOGW("sem_destroy(semDisp) error ret %d", ret);
	}

	if (nullptr != display_.p_wl_shm)
	{
		wl_shm_destroy(display_.p_wl_shm);
		display_.p_wl_shm = nullptr;
	}

	if (nullptr != display_.p_ivi_application)
	{
		ivi_application_destroy(display_.p_ivi_application);
		display_.p_ivi_application = nullptr;
	}

	if (nullptr != display_.p_wl_compositor)
	{
		wl_compositor_destroy(display_.p_wl_compositor);
		display_.p_wl_compositor = nullptr;
	}

	if (nullptr != display_.p_wl_registry)
	{
		wl_registry_destroy(display_.p_wl_registry);
		display_.p_wl_registry = nullptr;
	}

	if (nullptr != display_.p_wl_display)
	{
		ret = wl_display_flush(display_.p_wl_display);
		if (-1 == ret)
		{
			WLRNDR_LOGW("wl_display_flush() error. ret=%d", ret);
		}
		wl_display_disconnect(display_.p_wl_display);
		display_.p_wl_display = nullptr;
	}
}

/*****************************************************************************
 処理概要：	Waylandイベントスレッドメイン関数
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CWaylandRenderer::WaylandEventThreadMain(void)
{
	int32_t error_count{0};
	struct pollfd fds[2];

	WLRNDR_LOGV("[IN ]");

	fds[0].fd = wl_display_get_fd(display_.p_wl_display);
	fds[0].events = POLLIN;
	fds[0].revents = 0;

	fds[1].fd = shutdown_fd_;
	fds[1].events = POLLIN;
	fds[1].revents = 0;

	bool done{false};
	while (!done)
	{
		int32_t ret;
		while (0 != wl_display_prepare_read(display_.p_wl_display))
		{
			ret = wl_display_dispatch_pending(display_.p_wl_display);
			if (-1 == ret)
			{
				WLRNDR_LOGW("wl_display_dispatch_pending() error. ret=%d", ret);
			}
		}

		ret = wl_display_flush(display_.p_wl_display);
		if (-1 == ret)
		{
			WLRNDR_LOGW("wl_display_flush() error. ret=%d", ret);
			error_count ++;
			if (kWaylandEventThreadErrorMax <= error_count)
			{
				WLRNDR_LOGE("too much error occured. count=%d. break thread loop.", error_count);
				done = true;
			}
		}
		else
		{
			/* カウントリセット */
			error_count = 0;
		}

		if (!done)
		{
			ret = poll(&fds[0], 2U, -1);
			if (-1 == ret)
			{
				wl_display_cancel_read(display_.p_wl_display);
				if (EAGAIN == errno)
				{
					WLRNDR_LOGD("poll() error. errno=EAGAIN. continue");
				}
				else
				{
					WLRNDR_LOGE("poll() error. errno=%d. break thread loop.", errno);
					done = true;
				}
			}
			else
			{
				if (0 != (fds[0].revents & POLLIN))
				{
					(void)wl_display_read_events(display_.p_wl_display);

					ret = wl_display_dispatch_pending(display_.p_wl_display);
					if (-1 == ret)
					{
						if (EPIPE == errno)
						{
							WLRNDR_LOGE("wl_display_dispatch_pending() error. errno=EPIPE. asuume wl_display is disconnected. break thread loop.");
							done = true;
						}
						else
						{
							WLRNDR_LOGW("wl_display_dispatch_pending() error. errno=%d", errno);
						}
					}
				}
				else
				{
					wl_display_cancel_read(display_.p_wl_display);
				}

				if ((!done) && (0 != (fds[1].revents & POLLIN)))
				{
					/* shutdown_fd_なのでreadableになったらスレッド終了 */
					WLRNDR_LOGI("received shutdown request. break thread loop");
					done = true;
				}
			}
		}
	}

	WLRNDR_LOGV("[OUT]");
}


/*****************************************************************************
 処理概要：	出力先追加
 引数    ：	const struct WlRendererOutput& output		(i)出力先情報
 戻り値  ：	なし
*****************************************************************************/
void CWaylandRenderer::AddOutput(const struct WlRendererOutput& output)
{
	if (true == finalizing_)
	{
		return;
	}

	if (nullptr != p_output_listener_)
	{
		p_output_listener_->NotifyOutputAdded(output.width, output.height, output.model, output.make);
	}

	const std::lock_guard<std::mutex> lock_output{mutex_output_list_};
	std::unique_ptr<WlRendererOutput> p_output{std::make_unique<WlRendererOutput>()};
	*p_output = output;
	display_.output_list.push_back(std::move(p_output));

}


} /* namespace wlrenderer */

