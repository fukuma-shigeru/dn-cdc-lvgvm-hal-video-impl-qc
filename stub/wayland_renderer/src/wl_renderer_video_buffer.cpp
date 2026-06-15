/*******************************************************************************
    機能名称    ：  Wayland映像描画バッファクラス
    ファイル名称：  wl_renderer_video_buffer.cpp
*******************************************************************************/
#include "wl_renderer_video_buffer.h"

#include <iostream>

#include "wl_renderer_define.h"
#include "wl_renderer_log.h"
#include "wl_renderer_compliant.h"
#include "wl_renderer_video.h"
#if 0 //Bev3 08/26
#include <linux-dmabuf-unstable-v1-client-protocol.h>
#endif
namespace wlrenderer
{

namespace
{

extern "C" {
static void ReleaseBuffer(void * const p_data, struct wl_buffer * const p_wl_buffer);
}

static constexpr struct wl_buffer_listener g_buffer_listener{
	&ReleaseBuffer
};

/*****************************************************************************
 処理概要：	wl_bufferオブジェクトreleaseイベント登録用関数
 引数    ：	void *p_data					(i)ユーザーデータポインタ
           	struct wl_buffer *p_wl_buffer	(i)wl_bufferオブジェクトポインタ
 戻り値  ：	なし
*****************************************************************************/
static void ReleaseBuffer(void * const p_data, struct wl_buffer * const p_wl_buffer)
{
	CWaylandRendererVideoBuffer * const p_buffer{static_cast<CWaylandRendererVideoBuffer *>(p_data)};

	if (nullptr == p_buffer)
	{
		WLRNDR_LOGE("buffer user data is null");
	}
	else
	{
		p_buffer->WaylandReleaseBuffer(p_wl_buffer);
	}
}

} /* namespace */


/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CWaylandRendererVideoBuffer::CWaylandRendererVideoBuffer(const int32_t index)
	:p_video_(nullptr)
	,index_(index)
	,type_(0)
	,busy_(false)
	,width_(0)
	,height_(0)
	,format_(0)
	,plane_count_(0U)
	,p_release_cb_(nullptr)
	,p_user_data_(nullptr)
{
	WLRNDR_LOGV("CWaylandRendererVideoBuffer[%d] is created. this=%p", index_, this);
	for(uint32_t i{0U}; kPlaneMax > i; i++)
	{
		stride_[i] = 0;
		dmafd_[i]  = -1;
		p_addr_[i] = nullptr;
	}

}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CWaylandRendererVideoBuffer::~CWaylandRendererVideoBuffer(void)
{
	WLRNDR_LOGV("CWaylandRendererVideoBuffer[%d] is deleted. this=%p", index_, this);
	Finalize();
}

/*****************************************************************************
 処理概要：	初期化処理（内部リソースの確保）
 引数    ：	CWaylandRendererVideo *p_video
           	int32_t	type	(i)バッファタイプ
           	int32_t	format	(i)フォーマット
           	int32_t	width	(i)幅
           	int32_t	height	(i)高さ
           	int32_t	stride	(i)ストライド
           	void	*p_addr	(i)メモリアドレス
           	struct wl_shm_pool	*p_wl_pool	(i)wl_poolオブジェクト
 戻り値  ：	処理結果
           		WL_RENDERER_ERR_PARAM		パラメータ不正
           		WL_RENDERER_ERR_WAYLAND		waylandプロトコルエラー
           		WL_RENDERER_SUCCESS			正常終了
*****************************************************************************/
int32_t CWaylandRendererVideoBuffer::Initialize(CWaylandRendererVideo * const p_video, const int32_t type, const int32_t format,
	const int32_t width, const int32_t height, const int32_t stride, void * const p_addr, struct wl_shm_pool * const p_wl_pool, int32_t surface_id)
{
	int32_t		result{WL_RENDERER_SUCCESS};

	if (nullptr == p_video)
	{
		WLRNDR_LOGE("parameter error. p_video=%p", p_video);
		result = WL_RENDERER_ERR_PARAM;
	}
	else
	{
		if ((static_cast<int32_t>(WL_SHM_FORMAT_XRGB8888) != format) && (static_cast<int32_t>(WL_SHM_FORMAT_ARGB8888) != format))
		{
			WLRNDR_LOGW("format[%d] is not supported.", format);
			result = WL_RENDERER_ERR_PARAM;
		}
		else
		{
			WLRNDR_LOGI("shm buffer index=%d (%dx%d)", index_, width, height);

			p_video_   = p_video;
			type_      = type;
			width_     = width;
			height_    = height;
			stride_[0] = stride;
			format_    = format;
			p_addr_[0] = p_addr;
			plane_count_ = 1U;

			struct WlBufferData wl_buf{};
			wl_buf.p_wl_buffer_ = wl_shm_pool_create_buffer(p_wl_pool, MulI32(MulI32(index_, stride), height), width, height, stride, static_cast<uint32_t>(format));
			if (nullptr == wl_buf.p_wl_buffer_)
			{
				WLRNDR_LOGE("wl_shm_pool_create_buffer() error.");
				result = WL_RENDERER_ERR_WAYLAND;
			}
			else
			{
				const int32_t ret{wl_buffer_add_listener(wl_buf.p_wl_buffer_, &g_buffer_listener, this)};
				if (-1 == ret)
				{
					WLRNDR_LOGE("wl_buffer_add_listener() error.");
					result = WL_RENDERER_ERR_WAYLAND;
				}
				else
				{
					(void)wl_buffer_data_.insert(std::make_pair(surface_id, wl_buf));
				}
			}
		}
	}

	return result;
}

/*****************************************************************************
 処理概要：	初期化処理（内部リソースの確保）
 引数    ：	CWaylandRendererVideo *p_video
           	int32_t	type	(i)バッファタイプ
           	int32_t	format	(i)フォーマット
           	int32_t	width	(i)幅
           	int32_t	height	(i)高さ
           	void	*p_addr	(i)メモリアドレス
           	struct zwp_linux_dmabuf_v1 *p_zwp_linux_dmabuf_v1	(i)linux_dmabufオブジェクト
 戻り値  ：	処理結果
           		WL_RENDERER_ERR_PARAM		パラメータ不正
           		WL_RENDERER_ERR_WAYLAND		waylandプロトコルエラー
           		WL_RENDERER_SUCCESS			正常終了
*****************************************************************************/
#if 0 //Bev3 08/26
int32_t CWaylandRendererVideoBuffer::InitializeDma(CWaylandRendererVideo * const p_video, const int32_t type, const int32_t format,
	const int32_t width, const int32_t height, const VideoBufCtlBuffer* const vbcBuf, struct zwp_linux_dmabuf_v1 * const p_zwp_linux_dmabuf_v1, const std::vector<int32_t>& surface_ids)
{
	uint32_t i;

	if (nullptr == p_video)
	{
		WLRNDR_LOGE("parameter error. p_video=%p", p_video);
		return WL_RENDERER_ERR_PARAM;
	}

	WLRNDR_LOGI("linux_dmabuf index=%d (%dx%d)", index_, width, height);

	p_video_     = p_video;
	type_        = type;
	width_       = width;
	height_      = height;
	format_      = format;
	plane_count_ = vbcBuf->numPlane;
	const VideoBufCtlPlane * const pPlane{vbcBuf->plane};

	for (i = 0U; i < vbcBuf->numPlane; i++)
	{
		p_addr_[i] = pPlane[i].virtAddr;
		stride_[i] = UI32ToI32(pPlane[i].stride);
		dmafd_[i]  = pPlane[i].dmabufFd;

		WLRNDR_LOGD("pPlane[%d].dmabufFd=%d, .stride=%d", i, pPlane[i].dmabufFd, pPlane[i].stride);
	}
	for (auto itid = surface_ids.begin(); itid != surface_ids.end(); ++itid)
	{
		struct WlBufferData wl_buf{};
		/* linux_dmabuf. wl_buffer生成 */
		struct zwp_linux_buffer_params_v1 * const p_zwp_linux_buffer_params_v1{zwp_linux_dmabuf_v1_create_params (p_zwp_linux_dmabuf_v1)};
		for (i = 0U; i < vbcBuf->numPlane; i++)
		{
			zwp_linux_buffer_params_v1_add (p_zwp_linux_buffer_params_v1, pPlane[i].dmabufFd, static_cast<uint32_t>(i), pPlane[i].offset, pPlane[i].stride, 0U, 0U);
		}
		wl_buf.p_wl_buffer_ = zwp_linux_buffer_params_v1_create_immed(p_zwp_linux_buffer_params_v1, width, height, I32ToUI32(format), 0U);
		if (nullptr == wl_buf.p_wl_buffer_)
		{
			WLRNDR_LOGE("zwp_linux_buffer_params_v1_create_immed() error.");
			return WL_RENDERER_ERR_WAYLAND;
		}

		/* イベントリスナーの登録 */
		const int32_t ret{wl_buffer_add_listener(wl_buf.p_wl_buffer_, &g_buffer_listener, this)};
		if (-1 == ret)
		{
			WLRNDR_LOGE("wl_buffer_add_listener() error.");
			return WL_RENDERER_ERR_WAYLAND;
		}
		(void)wl_buffer_data_.insert(std::make_pair(*itid, wl_buf));
	}


	return WL_RENDERER_SUCCESS;
}
#endif

/*****************************************************************************
 処理概要：	終了処理（内部リソースの解放）
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CWaylandRendererVideoBuffer::Finalize(void)
{
	for (auto it_buf = wl_buffer_data_.begin(); it_buf != wl_buffer_data_.end(); ++it_buf)
	{
		wl_buffer_destroy(it_buf->second.p_wl_buffer_);
	}
	wl_buffer_data_.clear();

}

/*****************************************************************************
 処理概要：	Wayland Compositorからのバッファ開放
 引数    ：	struct wl_buffer *p_wl_buffer	(i)wl_bufferポインタ
 戻り値  ：	なし
*****************************************************************************/
void CWaylandRendererVideoBuffer::WaylandReleaseBuffer(const struct wl_buffer * const p_wl_buffer)
{
	bool buf_release{false};
	for (auto it_buf = wl_buffer_data_.begin(); it_buf != wl_buffer_data_.end(); ++it_buf)
	{
		if ( it_buf->second.p_wl_buffer_ == p_wl_buffer )
		{
			it_buf->second.use_ = false;
			buf_release = true;
			break;
		}
	}

	bool buf_used{false};
	for (auto it_buf = wl_buffer_data_.begin(); it_buf != wl_buffer_data_.end(); ++it_buf)
	{
		if ( true == it_buf->second.use_ )
		{
			buf_used = true;
			break;
		}
	}

	/* 管理しているバッファがリリースされた AND 全部のバッファを使っていない */
	if ( (true == buf_release) && (false == buf_used) )
	{
		p_video_->ReturnBuffer(this);

		if (nullptr != p_release_cb_)
		{
			p_release_cb_(p_user_data_);
		}
	}
}

/*****************************************************************************
 処理概要：	バッファクリア
 引数    ：	int32_t color	(i)クリアカラー
 戻り値  ：	なし
*****************************************************************************/
void CWaylandRendererVideoBuffer::Clear(const int32_t color) const
{
#if 0 //Bev3 08/26
	/* バッファタイプ */
	if (static_cast<int32_t>(CWaylandRendererVideo::VIDEO_BUFFER_TYPE_DMA) == type_)
	{
		/* フォーマットチェック(対象:4バイト、1プレーンのみ) */
		if ((format_ == static_cast<int32_t>(DRM_FORMAT_XRGB8888)) || (format_ == static_cast<int32_t>(DRM_FORMAT_ARGB8888)))
		{
			WLRNDR_LOGI("clear (%dx%d) %x", width_, height_, color);

			/* XRGB塗りつぶし */
			int32_t *p_mem{static_cast<int32_t *>(p_addr_[0])};
			if (nullptr != p_mem)
			{
				for (int32_t i{(stride_[0] / 4) * height_}; i > 0; i--)
				{
					*p_mem++ = color;
				}
			}
			else
			{
				WLRNDR_LOGE("p_addr_[0] is nullptr.");
			}

			return;
		}
	}
	WLRNDR_LOGW("State mismatch. type=%d, format=%x", type_, format_);
#endif
}

/*****************************************************************************
 処理概要：	リリースコールバック登録
 引数    ：	void (*p_callback)(void *p_data)	(i)コールバック関数
           	void *p_data						(i)ユーザーデータ
 戻り値  ：	なし
*****************************************************************************/
void CWaylandRendererVideoBuffer::SetReleaseCallback(void (*p_callback)(void *p_data), void * const p_data) noexcept
{
	p_release_cb_ = p_callback;
	p_user_data_ = p_data;
}

/*****************************************************************************
 処理概要：	ストライド取得
 引数    ：	uint32_t plane	(i)プレーンNo(0～2)
 戻り値  ：	ストライド
*****************************************************************************/
int32_t CWaylandRendererVideoBuffer::GetStride(const uint32_t plane) const noexcept
{
	if (kPlaneMax <= plane)
	{
		return stride_[0];
	}
	return stride_[plane];
}

/*****************************************************************************
 処理概要：	Dmaバッファファイルディスクリプタ取得
 引数    ：	uint32_t plane	(i)プレーンNo(0～2)
 戻り値  ：	ファイルディスクリプタ
*****************************************************************************/
int32_t CWaylandRendererVideoBuffer::GetDmafd(const uint32_t plane) const noexcept
{
	if (kPlaneMax <= plane)
	{
		return dmafd_[0];
	}
	return dmafd_[plane];
}

/*****************************************************************************
 処理概要：	プレーン数取得
 引数    ：	なし
 戻り値  ：	プレーン数
*****************************************************************************/
uint32_t CWaylandRendererVideoBuffer::GetPlaneCount(void) const noexcept
{
	return plane_count_;
}

/*****************************************************************************
 処理概要：	メモリアドレス取得
 引数    ：	uint32_t plane	(i)プレーンNo(0～2)
 戻り値  ：	メモリアドレス
*****************************************************************************/
void *CWaylandRendererVideoBuffer::GetPlaneAddr(const uint32_t plane) const noexcept
{
	if (kPlaneMax <= plane)
	{
		return nullptr;
	}
	return p_addr_[plane];
}

/*****************************************************************************
 処理概要：	ビジー状態取得
 引数    ：	なし
 戻り値  ：	ビジー状態
*****************************************************************************/
bool CWaylandRendererVideoBuffer::GetBusy(void) const noexcept
{
	return busy_;
}

/*****************************************************************************
 処理概要：	ビジー状態設定
 引数    ：	bool busy	(i)ビジー状態
 戻り値  ：	なし
*****************************************************************************/
void CWaylandRendererVideoBuffer::SetBusy(const bool busy) noexcept
{
	busy_ = busy;
}

/*****************************************************************************
 処理概要：	wl_buffer取得
 引数    ：	OutputTarget target		(i)取得ターゲット
 戻り値  ：	wl_bufferオブジェクト
*****************************************************************************/
struct wl_buffer *CWaylandRendererVideoBuffer::GetWlBuffer(const int32_t surface_id) const
{
	struct wl_buffer* p_buf{nullptr};

	const auto it_buf = wl_buffer_data_.find(surface_id);
	if ( it_buf != wl_buffer_data_.end() )
	{
		p_buf = it_buf->second.p_wl_buffer_;
	}
	else
	{
		WLRNDR_LOGE("invalid surface_id=%d", surface_id);
	}
	return p_buf;
}

/*****************************************************************************
 処理概要：	wl_buffer使用設定
 引数    ：	OutputTarget target		(i)取得ターゲット
 戻り値  ：	なし
*****************************************************************************/
void CWaylandRendererVideoBuffer::SetWlBufferUse(const int32_t surface_id)
{
	const auto it_buf = wl_buffer_data_.find(surface_id);
	if ( it_buf != wl_buffer_data_.end() )
	{
		it_buf->second.use_ = true;
	}
	else
	{
		WLRNDR_LOGE("invalid surface_id=%d", surface_id);
	}
}


} /* namespace wlrenderer */

