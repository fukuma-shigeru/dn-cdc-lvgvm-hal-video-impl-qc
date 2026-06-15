/*******************************************************************************
    機能名称    ：  Wayland映像描画クラス
    ファイル名称：  wl_renderer_video.cpp
*******************************************************************************/
#include "wl_renderer_video.h"

#include <algorithm>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <cerrno>
#include <sys/mman.h>

#include "wl_renderer_define.h"
#include "wl_renderer_log.h"
#include "wl_renderer_compliant.h"
#include "wl_renderer.h"
#include "wl_renderer_config.h"
#if 0 //Bev3 08/26
#include <linux-dmabuf-unstable-v1-client-protocol.h>
#endif
#include <climits>

namespace wlrenderer
{

namespace
{
constexpr int32_t kSyncTimeout{1};		/* 秒 */

extern "C" {
static void SyncDone(void * const p_data, struct wl_callback * const p_wl_callback, const uint32_t serial);
static void FrameDone(void * const p_data, struct wl_callback * const p_wl_callback, const uint32_t cb_time);
}

/*****************************************************************************
 処理概要：	wl_callbackオブジェクトdoneイベント登録用関数（wl_display_sync）
 引数    ：	void *p_data						(i)ユーザーデータポインタ
           	struct wl_callback *p_wl_callback	(i)wl_callbackオブジェクトポインタ
           	uint32_t serial						(i)シリアル番号
 戻り値  ：	なし
*****************************************************************************/
static void SyncDone(void * const p_data, struct wl_callback * const p_wl_callback, const uint32_t serial)
{
	CWaylandRendererVideo * const p_video{static_cast<CWaylandRendererVideo *>(p_data)};
	if (nullptr == p_video)
	{
		WLRNDR_LOGW("data is NULL");
	}
	else
	{
		p_video->WaylandSyncDone();
	}
	wl_callback_destroy(p_wl_callback);
}

/*****************************************************************************
 処理概要：	wl_callbackオブジェクトdoneイベント登録用関数（wl_surface_frame）
 引数    ：	void *p_data						(i)ユーザーデータポインタ
           	struct wl_callback *p_wl_callback	(i)wl_callbackオブジェクトポインタ
           	uint32_t cb_time					(i)時刻
 戻り値  ：	なし
*****************************************************************************/
static void FrameDone(void * const p_data, struct wl_callback * const p_wl_callback, const uint32_t cb_time)
{
	/* 特に処理なし */
	wl_callback_destroy(p_wl_callback);
}

} /* namespace */


/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CWaylandRendererVideo::CWaylandRendererVideo(void)
	:p_renderer_(nullptr)
	,free_buf_count_(0U)
	,sync_done_(false)
	,buf_type_(0)
	,loop_buf_(false)
	,output_error_(false)
	,videoBufCtlObj_{}
	,p_shm_{}
	,shm_size_{}
{
	WLRNDR_LOGV("CWaylandRendererVideo is created. this=%p", this);
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CWaylandRendererVideo::~CWaylandRendererVideo(void)
{
	WLRNDR_LOGV("CWaylandRendererVideo is deleted. this=%p", this);
	Finalize();
}

/*****************************************************************************
 処理概要：	初期化処理（内部リソースの確保、weston接続）
 引数    ：	CWaylandRenderer *p_renderer	(i)WaylandRendererインスタンスポインタ
           	CWaylandRendererConfig &conf	(i)Video設定情報
 戻り値  ：	処理結果
           		WL_RENDERER_ERR_PARAM	パラメータ不正
           		WL_RENDERER_ERR_WAYLAND	waylandプロトコルエラー
           		WL_RENDERER_SUCCESS		正常終了
*****************************************************************************/
int32_t CWaylandRendererVideo::Initialize(CWaylandRenderer * const p_renderer, const CWaylandRendererConfig &conf)
{
	if (nullptr == p_renderer)
	{
		WLRNDR_LOGE("parameter error. p_renderer=%p", p_renderer);
		return WL_RENDERER_ERR_PARAM;
	}

	int32_t pret{0};
	pthread_condattr_t attr{};
	/* 同期用変数の初期化 */
	(void)pthread_condattr_init(&attr);
	pret = pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
	if (0 != pret)
	{
		WLRNDR_LOGE("pthread_condattr_setclock error. ret = %d", pret);
		(void)pthread_condattr_destroy(&attr);
		return WL_RENDERER_ERR_PARAM;
	}

	pret = pthread_cond_init(&syncCond_, &attr);
	if (0 != pret)
	{
		WLRNDR_LOGE("pthread_cond_init error. ret = %d", pret);
		(void)pthread_condattr_destroy(&attr);
		return WL_RENDERER_ERR_PARAM;
	}

	(void)pthread_condattr_destroy(&attr);

	pret = pthread_mutex_init(&syncMutex_, NULL);
	if (0 != pret)
	{
		WLRNDR_LOGE("pthread_mutex_init error. ret = %d", pret);
		return WL_RENDERER_ERR_PARAM;
	}

	const uint32_t buf_count{conf.GetBufferCount()};
	if (0U >= buf_count)
	{
		WLRNDR_LOGE("parameter error. buf_count=%d", buf_count);
		return WL_RENDERER_ERR_PARAM;
	}

	if (kWaylandBufferMax < buf_count)
	{
		WLRNDR_LOGW("buf_count[%d] is too large.", buf_count);
	}

	std::vector<int32_t> surface_ids{};
	conf.GetSurfaceIds(surface_ids);
	if (surface_ids.empty())
	{
		WLRNDR_LOGE("parameter error surface_ids is empty");
		return WL_RENDERER_ERR_PARAM;
	}

	int32_t ret{WL_RENDERER_SUCCESS};
	int32_t ret_inner{WL_RENDERER_SUCCESS};
	p_renderer_ = p_renderer;
	buf_type_ = conf.GetBufferType();
	output_error_ = false;

	WlRendererDisplay * const p_display{GetDisplayInfo(p_renderer)};
	for(auto itid = surface_ids.begin() ; itid != surface_ids.end(); ++itid)
	{
		ret_inner = CreateSurface(*p_display, *itid);
		if (ret_inner != WL_RENDERER_SUCCESS)
		{
			WLRNDR_LOGE("CreateSurface error. ret=%d ivi_id=%d", ret_inner, *itid);
			ret = ret_inner;
			break;
		}
	}

	/* バッファの確保 */
	switch (buf_type_)
	{
		case static_cast<int32_t>(VIDEO_BUFFER_TYPE_SHM):
			ret_inner = CreateBuffers(conf);
			if (0 > ret_inner)
			{
				WLRNDR_LOGE("CreateBuffers() error. ret=%d", ret_inner);
				ret = ret_inner;
			}
			break;
		case static_cast<int32_t>(VIDEO_BUFFER_TYPE_DMA):
			ret_inner = CreateBuffersDma(conf);
			if (0 > ret_inner)
			{
				WLRNDR_LOGE("CreateBuffersDma() error. ret=%d", ret_inner);
				ret = ret_inner;
			}
			break;
		default:
			WLRNDR_LOGW("unknown buffer type");
			ret = WL_RENDERER_ERR_PARAM;
			break;
	}

	if (WL_RENDERER_SUCCESS == ret)
	{
		free_buf_count_ = buf_count;
	}

	return ret;
}

/*****************************************************************************
 処理概要：	終了処理（内部リソースの解放、weston切断）
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CWaylandRendererVideo::Finalize(void)
{
	/* 終了時はループバッファ解除 */
	loop_buf_ = false;

	std::vector<int32_t> surface_ids{};
	for(auto itwindow = windows_.begin() ; itwindow != windows_.end(); ++itwindow)
	{
		surface_ids.push_back(itwindow->first);
	}

	/* バッファ取り外し */
	(void)SendBuffer(nullptr, surface_ids);

	const std::lock_guard<std::mutex> lock_buffer{mtx_buffer_};

	/* バッファの開放 */
	switch (buf_type_)
	{
		case static_cast<int32_t>(VIDEO_BUFFER_TYPE_SHM):
			DestroyBuffers();
			break;
		case static_cast<int32_t>(VIDEO_BUFFER_TYPE_DMA):
			DestroyBuffersDma();
			break;
		default:
			WLRNDR_LOGW("unknown buffer type");
			break;
	}

	/* サーフェス破棄 */
	for(auto itid = surface_ids.begin() ; itid != surface_ids.end(); ++itid)
	{
		DestroySurface(*itid);
	}
	WlRendererDisplay * const p_display{GetDisplayInfo(p_renderer_)};
	(void)wl_display_flush(p_display->p_wl_display);

	int32_t pret{0};
	pret = pthread_cond_destroy(&syncCond_);
	if (0 != pret)
	{
		WLRNDR_LOGW("pthread_cond_destroy error %d", pret);
	}
	pret = pthread_mutex_destroy(&syncMutex_);
	if (0 != pret)
	{
		WLRNDR_LOGW("pthread_mutex_destroy error %d", pret);
	}
}

/*****************************************************************************
 処理概要：	バッファ取得
 引数    ：	なし
 戻り値  ：	bufferポインタ
*****************************************************************************/
CWaylandRendererVideoBuffer *CWaylandRendererVideo::GetBuffer(void) noexcept
{
	std::unique_lock<std::mutex> lock_buffer{mtx_buffer_};

	while (0U == free_buf_count_)
	{
		cond_buffer_.wait(lock_buffer);
	}

	if (0U < free_buf_count_ )
	{
		free_buf_count_--;
	}

	CWaylandRendererVideoBuffer *p_available{nullptr};
	for (auto itr_buffer = buffers_.begin(); itr_buffer != buffers_.end(); ++itr_buffer)
	{
		CWaylandRendererVideoBuffer * const p_buffer{itr_buffer->get()};

		const bool busy{p_buffer->GetBusy()};
		if (false == busy)
		{
			p_buffer->SetBusy(true);
			p_available = p_buffer;
			break;
		}
	}

	return p_available;
}

/*****************************************************************************
 処理概要：	バッファ送信
 引数    ：	CWaylandRendererVideoBuffer *p_buffer	(i)bufferポインタ
 戻り値  ：	処理結果
           		WL_RENDERER_ERR_WAYLAND	waylandプロトコルエラー
           		WL_RENDERER_SUCCESS		正常終了
*****************************************************************************/
int32_t CWaylandRendererVideo::SendBuffer(CWaylandRendererVideoBuffer * const p_buffer, const std::vector<int32_t>& surface_ids)
{
	const std::lock_guard<std::mutex> lock_buffer{mtx_buffer_};

	if (nullptr == p_buffer)
	{
		/* nullptrの場合はバッファ取り外しとして動作する。 */
		WLRNDR_LOGI("p_buffer is nullptr. dettach the current buffer.");
	}
	else
	{
		const bool busy{p_buffer->GetBusy()};
		if (false == busy)
		{
			WLRNDR_LOGE("p_buffer[%p] is invalid", p_buffer);
			return WL_RENDERER_ERR_PARAM;
		}
	}

	bool commit{false};
	WlRendererDisplay * const p_display{GetDisplayInfo(p_renderer_)};
	for( auto itid = surface_ids.begin(); itid != surface_ids.end(); ++itid)
	{
		const auto itwindow = windows_.find(*itid);
		if ( itwindow != windows_.end() )
		{
			CommitSurface(*p_display, p_buffer, *itid, itwindow->second);
			commit = true;
		}
	}

	if ( true == commit )
	{
		/* 描画時のみ */
		if (nullptr != p_buffer)
		{
			const int32_t ret{WaylandSync()};
			if (0 > ret)
			{
				WLRNDR_LOGW("WaylandSync() error. ret=%d", ret);
			}
		}
	}

	return WL_RENDERER_SUCCESS;
}

/*****************************************************************************
 処理概要：	バッファ返却
 引数    ：	CWaylandRendererVideoBuffer *p_buffer	(i)bufferポインタ
 戻り値  ：	なし
*****************************************************************************/
void CWaylandRendererVideo::ReturnBuffer(CWaylandRendererVideoBuffer * const p_buffer)
{
	if (loop_buf_)
	{
		return;
	}
	const std::unique_lock<std::mutex> lock_buffer{mtx_buffer_};

	const bool busy{p_buffer->GetBusy()};
	if (true == busy)
	{
		p_buffer->SetBusy(false);

		free_buf_count_++;
		cond_buffer_.notify_one();
	}
	else
	{
		WLRNDR_LOGW("not busy buffer. nothing to do.");
	}
}

/*****************************************************************************
 処理概要：	バッファ作成（wl_shm）
 引数    ：	CWaylandRendererConfig &conf	(i)Video設定情報
 戻り値  ：	処理結果
           		WL_RENDERER_ERR_PARAM	パラメータ不正
           		WL_RENDERER_ERR_WAYLAND	waylandプロトコルエラー
           		WL_RENDERER_ERR_FD		ファイルディスクリプタエラー
           		WL_RENDERER_ERR_MEMORY	メモリ確保エラー
           		WL_RENDERER_SUCCESS		正常終了
*****************************************************************************/
int32_t CWaylandRendererVideo::CreateBuffers(const CWaylandRendererConfig &conf)
{
	int32_t ret;

	char path[]{"/run/arene/share/wlrndr-shared-XXXXXX"};
	const int32_t fd{mkostemp(&path[0], O_CLOEXEC)};
	if (0 > fd)
	{
		WLRNDR_LOGE("mkostemp error, errno=%d", errno);
		return WL_RENDERER_ERR_FD;
	}

	(void)unlink(&path[0]);

	const int32_t  buf_type{conf.GetBufferType()};
	const uint32_t buf_count{conf.GetBufferCount()};
	const uint32_t width{conf.GetWidth()};
	const uint32_t height{conf.GetHeight()};
	const int32_t  format{conf.GetFormat()};
	std::vector<int32_t> surface_ids{};
	conf.GetSurfaceIds(surface_ids);

	if ( surface_ids.empty() )
	{
		WLRNDR_LOGE("surface_ids is empty,");
		return WL_RENDERER_ERR_PARAM;
	}

	if (static_cast<uint64_t>(INT32_MAX) < static_cast<uint64_t>(width) * 4U)
	{
		WLRNDR_LOGE("stride is wrap around.");
		return WL_RENDERER_ERR_PARAM;
	}
	const int32_t stride{UI32ToI32(width * 4U)};

	if (UINT32_MAX < static_cast<uint64_t>(stride) * static_cast<uint64_t>(height))
	{
		WLRNDR_LOGE("buf_size is wrap around.");
		return WL_RENDERER_ERR_PARAM;
	}
	const uint32_t buf_size{static_cast<uint32_t>(stride) * height};

	if (static_cast<uint64_t>(INT32_MAX) < static_cast<uint64_t>(buf_size) * static_cast<uint64_t>(buf_count))
	{
		WLRNDR_LOGE("pool_size is wrap around.");
		return WL_RENDERER_ERR_PARAM;
	}
	const int32_t pool_size{UI32ToI32(static_cast<uint32_t>(buf_size) * buf_count)};

	ret = posix_fallocate(fd, 0, static_cast<int64_t>(pool_size));
	if (0 != ret)
	{
		WLRNDR_LOGE("posix_fallocate error, ret=%d", ret);
		(void)close(fd);
		return WL_RENDERER_ERR_MEMORY;
	}

	int8_t * const p_data{static_cast<int8_t *>(mmap(nullptr, static_cast<size_t>(pool_size), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0))};
	if (MAP_FAILED == p_data) {
		WLRNDR_LOGE("mmap error. errno=%d", errno);
		(void)close(fd);
		return WL_RENDERER_ERR_MEMORY;
	}

	WlRendererDisplay * const p_display{GetDisplayInfo(p_renderer_)};
	struct wl_shm_pool * const p_wl_pool{wl_shm_create_pool(p_display->p_wl_shm, fd, pool_size)};
	if (nullptr == p_wl_pool)
	{
		WLRNDR_LOGE("wl_shm_create_pool() error.");
		(void)close(fd);
		if (nullptr != p_data)
		{
			(void)munmap(p_data, static_cast<size_t>(pool_size));
		}
		return WL_RENDERER_ERR_WAYLAND;
	}

	for (uint32_t i{0U}; i < buf_count; i++)
	{
		std::unique_ptr<CWaylandRendererVideoBuffer> p_buffer{std::make_unique<CWaylandRendererVideoBuffer>(static_cast<int32_t>(i))};
		const uint64_t offset{i * static_cast<uint64_t>(buf_size)};
		(void)p_buffer->Initialize(this, buf_type, format, static_cast<int32_t>(width), static_cast<int32_t>(height), stride, static_cast<void *>(&p_data[offset]), p_wl_pool, surface_ids.at(0U));

		buffers_.push_back(std::move(p_buffer));
	}

	free_buf_count_ = buf_count;
	if (0 < pool_size)
	{
		p_shm_ = p_data;
		shm_size_ = static_cast<std::size_t>(pool_size);
	}

	wl_shm_pool_destroy(p_wl_pool);
	(void)close(fd);

	return WL_RENDERER_SUCCESS;
}

/*****************************************************************************
 処理概要：	バッファ作成（linux_dmabuf）
 引数    ：	CWaylandRendererConfig &conf	(i)Video設定情報
 戻り値  ：	処理結果
           		WL_RENDERER_ERR_MEMORY	メモリ確保エラー
           		WL_RENDERER_SUCCESS		正常終了
*****************************************************************************/
int32_t CWaylandRendererVideo::CreateBuffersDma(const CWaylandRendererConfig &conf)
{
#if 0 //Bev3 08/26
	WlRendererDisplay * const p_display{GetDisplayInfo(p_renderer_)};

	const uint32_t buf_count{conf.GetBufferCount()};
	const uint32_t width{conf.GetWidth()};
	const uint32_t height{conf.GetHeight()};
	const int32_t  format{conf.GetFormat()};
	loop_buf_          = conf.GetLoopBuffer();
	std::vector<int32_t> surface_ids{};
	conf.GetSurfaceIds(surface_ids);

	/* VideoBufCtlバッファ制御オープン */
	int32_t ret{VideoBufCtlOpen(&videoBufCtlObj_, VIDEO_BUF_CTL_DRM_DEVICE)};
	if (VIDEO_BUF_CTL_SUCCESS != ret)
	{
		WLRNDR_LOGE("VideoBufCtlOpen error, ret=%d", ret);
		return WL_RENDERER_ERR_MEMORY;
	}
	for (uint32_t i{0U}; i < buf_count; i++)
	{
		/* VideoBufCtlバッファ確保 */
		VideoBufCtlBuffer ctl_buffer{};
		(void)memset(&ctl_buffer, 0x00, sizeof(ctl_buffer));
		ctl_buffer.width  = UI32ToI32(width);
		ctl_buffer.height = UI32ToI32(height);
		ctl_buffer.format = I32ToUI32(format);
		ret = VideoBufCtlMemAlloc(&videoBufCtlObj_, &ctl_buffer);
		if (VIDEO_BUF_CTL_SUCCESS != ret)
		{
			WLRNDR_LOGE("VideoBufCtlMemAlloc error, ret=%d", ret);
			return WL_RENDERER_ERR_MEMORY;
		}
		ctl_buffers_.push_back(ctl_buffer);

		const int32_t  buf_type{conf.GetBufferType()};
		std::unique_ptr<CWaylandRendererVideoBuffer> p_buffer{std::make_unique<CWaylandRendererVideoBuffer>(static_cast<int32_t>(i))};
		(void)p_buffer->InitializeDma(this, buf_type, format, UI32ToI32(width), UI32ToI32(height), &ctl_buffer, p_display->p_zwp_linux_dmabuf_v1, surface_ids);

		buffers_.push_back(std::move(p_buffer));
	}

	free_buf_count_ = buf_count;
#endif
	return WL_RENDERER_SUCCESS;
}

/*****************************************************************************
 処理概要：	バッファ削除（wl_shm）
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CWaylandRendererVideo::DestroyBuffers(void) noexcept
{
	buffers_.clear();
	if (nullptr != p_shm_)
	{
		(void)munmap(p_shm_, shm_size_);
	}
}

/*****************************************************************************
 処理概要：	バッファ削除（linux_dmabuf）
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CWaylandRendererVideo::DestroyBuffersDma(void)
{
#if 0 //Bev3 08/26
	int32_t ret{0};

	buffers_.clear();

	/* VideoBufCtlバッファ破棄 */
	for (auto itr_buffer = ctl_buffers_.begin(); itr_buffer != ctl_buffers_.end(); ++itr_buffer)
	{
		VideoBufCtlBuffer buffer{*itr_buffer};
		ret = VideoBufCtlMemFree(&videoBufCtlObj_, &buffer);
		if (VIDEO_BUF_CTL_SUCCESS != ret)
		{
			WLRNDR_LOGE("VideoBufCtlMemFree error, ret=%d", ret);
		}
	}
	ctl_buffers_.clear();

	/* VideoBufCtlバッファ制御クローズ */
	ret = VideoBufCtlClose(&videoBufCtlObj_);
	if (VIDEO_BUF_CTL_SUCCESS != ret)
	{
		WLRNDR_LOGE("VideoBufCtlClose error, ret=%d", ret);
	}
#endif
}

/*****************************************************************************
 処理概要：	Wayland Compositorとのsync要求
 引数    ：	なし
 戻り値  ：	処理結果
           		WL_RENDERER_ERR_WAYLAND	waylandプロトコルエラー
           		WL_RENDERER_SUCCESS		正常終了
*****************************************************************************/
int32_t CWaylandRendererVideo::WaylandSync(void)
{
	int32_t ret{0};

	WlRendererDisplay * const p_display{GetDisplayInfo(p_renderer_)};

	/* 排他の手法をsem_wait/pthread_cond_timedwaitに変更 */
	/* 異なるスレッドからwait_forが呼ばれた際、 
	   mutexの再ロック取得が発生し、例外を発生されるため */
	ret = sem_wait(&p_display->semDisp);
	if (0 > ret)
	{
		WLRNDR_LOGW("sem_wait(semDisp) error %d", ret);
	}

	while (0 != wl_display_prepare_read(p_display->p_wl_display))
	{
		ret = wl_display_dispatch_pending(p_display->p_wl_display);
		if (-1 == ret)
		{
			/* エラー出力回数抑止 */
			if ( false == output_error_ )
			{
				WLRNDR_LOGW("wl_display_dispatch_pending() error. ret=%d", ret);
				output_error_ = true;
			}
		}
	}

	struct wl_callback* const p_wl_callback{wl_display_sync(p_display->p_wl_display)};
	static constexpr struct wl_callback_listener g_sync_listener{
		&SyncDone
	};
	ret = wl_callback_add_listener(p_wl_callback, &g_sync_listener, this);
	if (-1 == ret)
	{
		WLRNDR_LOGW("wl_callback_add_listener() error. ret=%d", ret);
	}
	wl_display_cancel_read(p_display->p_wl_display);

	ret = wl_display_flush(p_display->p_wl_display);
	if (-1 == ret)
	{
		WLRNDR_LOGW("wl_display_flush() error. ret=%d", ret);
	}

	ret = sem_post(&p_display->semDisp);
	if (0 > ret)
	{
		WLRNDR_LOGW("sem_post(semDisp) error %d", ret);
	}

	struct timespec ts{};
	/* コールバックが来るまで待機 */
	(void)pthread_mutex_lock(&syncMutex_);

	(void)clock_gettime(CLOCK_MONOTONIC, &ts);
	if (0 <= ts.tv_sec)
	{
		if(kSyncTimeout < (LONG_MAX - ts.tv_sec))
		{
			ts.tv_sec += kSyncTimeout;
		}
		else
		{
			ts.tv_sec = kSyncTimeout - 1;
		}
	}
	else
	{
		WLRNDR_LOGE("ts.tv_sec is a negative value");
	}

	while ((0 == ret) && (false == sync_done_))
	{
		ret = pthread_cond_timedwait(&syncCond_, &syncMutex_, &ts);
		if (0 != ret)
		{
			WLRNDR_LOGE("pthread_cond_timedwait error. ret = %d", ret);
		}
	}
	(void)pthread_mutex_unlock(&syncMutex_);

	if (false == sync_done_)
	{
		/* エラーになってコールバックが返ってこなかったとき */
		WLRNDR_LOGE("callback not called.");
		wl_callback_destroy (p_wl_callback);
	}

	return WL_RENDERER_SUCCESS;
}

/*****************************************************************************
 処理概要：	Wayland Compositorとのsync応答
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CWaylandRendererVideo::WaylandSyncDone(void)
{
	(void)pthread_mutex_lock(&syncMutex_);

	sync_done_ = true;
	(void)pthread_cond_signal(&syncCond_);

	(void)pthread_mutex_unlock(&syncMutex_);
}

/*****************************************************************************
 処理概要：	waylandサーフェス生成
 引数    ：	WlRendererDisplay &display		(i)ディスプレイ情報
           	int32_t surface_id)				(i)サーフェスID
 戻り値  ：	処理結果
           		WL_RENDERER_ERR_WAYLAND	waylandプロトコルエラー
           		WL_RENDERER_SUCCESS		正常終了
*****************************************************************************/
int32_t CWaylandRendererVideo::CreateSurface(const WlRendererDisplay &display, int32_t surface_id)
{
	WlRendererVideoWindow window{};

	/* サーフェス生成  */
	window.ivi_id = surface_id;
	window.p_wl_surface = wl_compositor_create_surface(display.p_wl_compositor);
	if (nullptr == window.p_wl_surface)
	{
		WLRNDR_LOGE("wl_compositor_create_surface() error.");
		return WL_RENDERER_ERR_WAYLAND;
	}
	if (nullptr != display.p_ivi_application)
	{
		/* iviサーフェス生成 */
		window.p_ivi_surface = ivi_application_surface_create(display.p_ivi_application, I32ToUI32(window.ivi_id), window.p_wl_surface);
		if (nullptr == window.p_ivi_surface)
		{
			WLRNDR_LOGE("ivi_application_surface_create() error.");
			return WL_RENDERER_ERR_WAYLAND;
		}
	}

	/* 入力を受け入れない設定（タッチ貫通） */
	struct wl_region* const region{wl_compositor_create_region(display.p_wl_compositor)};
	if (nullptr == region)
	{
		WLRNDR_LOGE("wl_compositor_create_region() error.");
	}
	else
	{
		wl_region_add(region, 0, 0, 0, 0);
		wl_surface_set_input_region(window.p_wl_surface, region);
		wl_region_destroy(region);
	}

	(void)windows_.insert(std::make_pair(surface_id, window));
	const int32_t ret{wl_display_flush(display.p_wl_display)};
	if (-1 == ret)
	{
		WLRNDR_LOGW("wl_display_flush() error. ret=%d", ret);
	}

	return WL_RENDERER_SUCCESS;
}

/*****************************************************************************
 処理概要：	waylandサーフェス破棄
 引数    ：	WlRendererVideoWindow &window	(i)ウィンドウ情報
 戻り値  ：	なし
*****************************************************************************/
void CWaylandRendererVideo::DestroySurface(const int32_t surface_id)
{
	const auto it = windows_.find(surface_id);
	if( it != windows_.end() )
	{
		if (nullptr != it->second.p_ivi_surface)
		{
			ivi_surface_destroy(it->second.p_ivi_surface);
			it->second.p_ivi_surface = nullptr;
		}

		if (nullptr != it->second.p_wl_surface)
		{
			wl_surface_destroy(it->second.p_wl_surface);
			it->second.p_wl_surface = nullptr;
		}
		it->second.ivi_id = 0;
		(void)windows_.erase(surface_id);

		WlRendererDisplay * const p_display{GetDisplayInfo(p_renderer_)};
		const int32_t ret{wl_display_flush(p_display->p_wl_display)};
		if (-1 == ret)
		{
			WLRNDR_LOGW("wl_display_flush() error. ret=%d", ret);
		}
	}
}

/*****************************************************************************
 処理概要：	waylandサーフェスコミット
 引数    ：	WlRendererDisplay &display							(i)ディスプレイ情報
           	CWaylandRendererVideoBuffer *p_buffer				(i)bufferポインタ
           	WlRendererVideoWindow &window						(i)ウィンドウ情報
           	CWaylandRendererVideoBuffer::OutputTarget target	(i)出力先ターゲット
 戻り値  ：	なし
*****************************************************************************/
void CWaylandRendererVideo::CommitSurface(const WlRendererDisplay &display, CWaylandRendererVideoBuffer * const p_buffer, const int32_t surface_id, const WlRendererVideoWindow &window)
{
	struct wl_buffer *p_wl_buffer{nullptr};

	if (nullptr == p_buffer)
	{
		/* nullptrの場合はバッファ取り外しとして動作する。 */
		WLRNDR_LOGI("p_buffer is nullptr. dettach the current buffer.");
	}
	else
	{
		p_wl_buffer = p_buffer->GetWlBuffer(surface_id);
	}

	/* 描画時のみ */
	if (nullptr != p_wl_buffer)
	{
		int32_t ret;
		while (0 != wl_display_prepare_read(display.p_wl_display))
		{
			ret = wl_display_dispatch_pending(display.p_wl_display);
			if (-1 == ret)
			{
				/* エラー出力回数抑止 */
				if ( false == output_error_ )
				{
					WLRNDR_LOGW("wl_display_dispatch_pending() error. ret=%d", ret);
					output_error_ = true;
				}
			}
		}

		struct wl_callback* const p_wl_callback{wl_surface_frame(window.p_wl_surface)};
		static constexpr struct wl_callback_listener g_frame_listener{
			&FrameDone
		};
		ret = wl_callback_add_listener(p_wl_callback, &g_frame_listener, p_buffer);
		if (-1 == ret)
		{
			WLRNDR_LOGW("wl_callback_add_listener() error. ret=%d", ret);
		}
		wl_display_cancel_read(display.p_wl_display);

		p_buffer->SetWlBufferUse(surface_id);
	}
	wl_surface_attach(window.p_wl_surface, p_wl_buffer, 0, 0);
	wl_surface_damage(window.p_wl_surface, 0, 0, window.width, window.height);
	wl_surface_commit(window.p_wl_surface);

	(void)wl_display_flush(display.p_wl_display);

}

} /* namespace wlrenderer */

