/*******************************************************************************
    機能名称    ：  キャプチャ制御モジュール
    ファイル名称：  vhal_capture_control.cpp
*******************************************************************************/
#include "vhal_capture_control.h"

#include "vhal_define.h"
#include "vhal_log.h"
#include "vhal_round_cast.h"
#include "vhal_layout_mng.h"
#include "vhal_capture_camera.h"
#include "vhal_capture_hdmi.h"
#include "vhal_observer_control.h"
#include "vhal_hdcp_auth_mng.h"
#include "vhal_main_control.h"
#include "vhal_event_item_capture_reset.h"
#include "vhal_debug_system.h"
#include "vendor_ext_properties.h"

extern "C"
{
#include "sif_util.h"
}

/*****************************************************************************
*	define																	 *
*****************************************************************************/
// MISRA C++-2008 Rule 16-2-1 #define VHAL_SUPPORT_SETTING_INITIALIZE
//#define VHAL_SUPPORT_RESET_CAPTURE

/*****************************************************************************
*	globals																	 *
*****************************************************************************/

/*****************************************************************************
*	statics																	 *
*****************************************************************************/

namespace videohal
{

std::unordered_map<captureInputType, VhalCaptureDeviceEntry> CVhalCaptureControl::device_entries_
{
	{captureInputType::VHAL_CAPTURE_INPUT_CAMERA,		{"INPUT_1_0",	DRM_FORMAT_UYVY,	5,	WL_RENDERER_COLOR_SPACE_ITU_R_601_FR}},
	{captureInputType::VHAL_CAPTURE_INPUT_HDMI,			{"INPUT_0_0",	DRM_FORMAT_UYVY,	5,	WL_RENDERER_COLOR_SPACE_ITU_R_709}},
	/* SUPPORT_DUMMY_CAMERA */
	{captureInputType::VHAL_CAPTURE_INPUT_CAMERA_DUMMY,	{"dummy",	DRM_FORMAT_UYVY,	5,	WL_RENDERER_COLOR_SPACE_ITU_R_601_FR}},
};

namespace {

CVhalCaptureControl *p_capture_control_{nullptr};
constexpr uint32_t kVhalQcarcamRetryCount{50U};	/* qcarcamのリトライ回数 */
constexpr uint32_t kVhalQcarcamRetryWait{200U};	/* qcarcamのリトライ待ち時間(ms) */

/* バッファリストID */
constexpr uint32_t	kVHalBufferlistId{9U};

/* BEVstep3 24/10/17 描画種別：実機=false / シミュレータ=true */
constexpr bool kVhalRendererShm{RENDERER_SHM};

/*****************************************************************************
 処理概要：	フレームキャプチャコールバック処理
 引数    ：	QCarCamHndl_t				hndl		(i)ハンドル
           	uint32_t					event_id	(i)イベントID
           	QCarCamEventPayload_t*		p_payload	(i)ペイロード
           	void*						pPrivateData(io)汎用データ(未使用)
 戻り値  ：	処理結果
           		QCARCAM_RET_BADPARAM	パラメータ不正
           		QCARCAM_RET_BADSTATE	状態異常
           		QCARCAM_RET_OK			正常終了
 フェールセーフNo：	F-VHAL-R-079
*****************************************************************************/
QCarCamRet_e CVhalCaptureEventCallBack(const QCarCamHndl_t hndl, const uint32_t event_id, const QCarCamEventPayload_t* p_payload, void* pPrivateData)
{
	QCarCamRet_e	qret{QCARCAM_RET_OK};
//	VHAL_LOGV_IN();

	VHAL_LOGV("event_id is %d", event_id);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	int32_t fail_ret{0};
//	bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-R-079",fail_ret)};
//	if(false == fail)
//	{
//		p_capture_control_->NotifyInterruptHeartbeat(hndl);
//	}
//#else
	p_capture_control_->NotifyInterruptHeartbeat(hndl);
//#endif

	if(QCARCAM_EVENT_FRAME_READY == event_id)
	{
		if (nullptr != p_payload)
		{
			const uint32_t	buf_index{p_payload->frameInfo.bufferIndex};
			VHAL_LOGV("bufferIndex=%u", buf_index);
			p_capture_control_->NotifyCaptureEvent(hndl, static_cast<const int32_t>(buf_index));
			p_capture_control_->SettingSubmitRequestPrev(hndl, buf_index, qret);
		}
	}
	else if (QCARCAM_EVENT_ERROR == event_id)
	{
		VHAL_LOGW("event_id is Error. event_id=%d", event_id);

		/* Check payload:QCARCAM_FATAL_ERROR[0] */
		if (nullptr != p_payload)
		{
			const QCarCamErrorInfo_t* p_error_info = reinterpret_cast<const QCarCamErrorInfo_t*>(p_payload);
			VHAL_LOGW("errorId is %d.", p_error_info->errorId);
			if (QCARCAM_ERROR_FATAL == p_error_info->errorId)
			{
				/* HDMI-Rxデバイス異常判定 */
				p_capture_control_->CheckHdmiReceiverError(hndl);
			}
		}
	}
	else
	{
		VHAL_LOGW("event_id is invalid. event_id=%d", event_id);
	}

//	VHAL_LOGV_OUT();
	return qret;
}

}	// namespace

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalCaptureDraw::CVhalCaptureDraw(void)
	:running_(false)
	,epoll_fd_(0)
	,pair_fd_{0,0}
	,p_draw_thread_(nullptr)
{
	VHAL_LOGV("CVhalCaptureDraw is created. this=%p", this);
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalCaptureDraw::~CVhalCaptureDraw(void)
{
	VHAL_LOGV("CVhalCaptureDraw is deleted. this=%p", this);
	Finalize();
}

/*****************************************************************************
 処理概要：	初期化処理（内部リソースの確保）
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_ERR_****		エラー
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalCaptureDraw::Initialize(void)
{
	int32_t	result{VHAL_SUCCESS};

	do
	{
		/* イベントループ用pollの初期化 */
		epoll_fd_ = epoll_create1(0);

		/* イベント検知対象FDを作成 */
		int32_t	ret{0};
		ret = socketpair(AF_UNIX, static_cast<int>(SOCK_STREAM), 0, pair_fd_);
		if (0 > ret)
		{
			VHAL_LOGE("sockerpair error. ret=%d", ret);
			result = VHAL_ERR_SOCKET;
			break;
		}

		struct epoll_event	event{};
		event.events  = static_cast<uint32_t>(EPOLLIN);
		event.data.fd = pair_fd_[kSocketPairRead];
		ret = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, pair_fd_[kSocketPairRead], &event);
		if (-1 == ret)
		{
			VHAL_LOGE("epoll_ctl(EPOLL_CTL_ADD) error.");
			result = VHAL_ERR_EPOLL;
			break;
		}

		/* スレッド起動 */
		p_draw_thread_ = std::make_unique<std::thread>(&CVhalCaptureDraw::DrawThreadMain, this);
	} while (false);

	return result;
}

/*****************************************************************************
 処理概要：	終了処理（内部リソースの解放）
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalCaptureDraw::Finalize(void)
{
	if (nullptr != p_draw_thread_)
	{
		VHAL_LOGI("requested to exit event loop.");

		/* イベントループの待ち解除（EPOLLHUPを発生させる） */
		(void)shutdown(pair_fd_[kSocketPairRead], SHUT_RDWR);
		p_draw_thread_->join();
		p_draw_thread_ = nullptr;
	}

	if (0 != pair_fd_[kSocketPairRead])
	{
		int32_t	ret{0};
		ret = epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, pair_fd_[kSocketPairRead], nullptr);
		if (-1 == ret)
		{
			VHAL_LOGW("epoll_ctl(EPOLL_CTL_DEL) error. errno=%d", errno);
		}
		(void)close(pair_fd_[kSocketPairRead]);
		pair_fd_[kSocketPairRead] = 0;
	}

	if (0 != pair_fd_[kSocketPairWrite])
	{
		(void)close(pair_fd_[kSocketPairWrite]);
		pair_fd_[kSocketPairWrite] = 0;
	}

	if (0 != epoll_fd_)
	{
		(void)close(epoll_fd_);
		epoll_fd_ = 0;
	}
}

/*****************************************************************************
 処理概要：	イベントの書き込み
 引数    ：	(const event_param_t	event_param		(i)イベントパラメータ
 戻り値  ：	処理結果
           		VHAL_ERR_****		エラー
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalCaptureDraw::WriteEvent(const event_param_t event_param) const
{
	int32_t	result{VHAL_SUCCESS};

//	VHAL_LOGV_IN();

	ssize_t	ret{0};
	ret = write(pair_fd_[kSocketPairWrite], &event_param, sizeof(event_param));
	if (0 > ret)
	{
		VHAL_LOGE("write(%ld) error. ret=%ld", sizeof(event_param), ret);
		result = VHAL_ERR_SOCKET;
	}

//	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	イベントの読み込みと実行
 引数    ：	uint32_t	source_event	(i)イベントフラグ
 戻り値  ：	処理結果
           		VHAL_ERR_****		エラー
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalCaptureDraw::ExecEvent(const uint32_t source_event)
{
	int32_t	result{VHAL_SUCCESS};

//	VHAL_LOGV_IN();

	do
	{
		if (0U != (source_event & (kSourceEventHungup | kSourceEventError)))
		{
			if (0U != (source_event & kSourceEventError))
			{
				/* 内部イベント待ち受けソケットでエラー発生。 */
				VHAL_LOGE("received fd error event. source_event=%d", source_event);
			}
			result = VHAL_ERR_SOCKET;
			break;
		}

		if (0U != (source_event & kSourceEventReadable))
		{
			event_param_t	event_param{};
			ssize_t			read_size{0};
			constexpr uint32_t kVhalEventRetryCount{15U};	/* イベントリトライ回数 */
			for (uint32_t i{0U}; i<kVhalEventRetryCount; ++i)
			{
				bool loop_end{false};
				errno = 0;
				read_size = read(pair_fd_[kSocketPairRead], &event_param, sizeof(event_param));
				if (0 > read_size)
				{
					if ((EINTR == errno) || (EIO == errno))
					{
						/* リトライ */
						if (kVhalEventRetryCount <= (i+1U))
						{
							VHAL_LOGE("read(%ld) error errno=%d, retry over(%d)",sizeof(event_param), errno, i+1U);
							loop_end = true;
						}
						else
						{
							constexpr uint32_t kVhalEventRetryWait{100U};	/* イベントリトライ待ち時間(ms) */
							sif_mdelay(kVhalEventRetryWait);
						}
					}
					else
					{
						/* 失敗 */
						VHAL_LOGE("read(%ld) error. errno=%d, read_size=%ld", sizeof(event_param), errno, read_size);
						loop_end = true;
					}
				}
				else
				{
					loop_end = true;
				}

				if (true == loop_end)
				{
					break;
				}
			}

			if (static_cast<size_t>(read_size) != sizeof(event_param))
			{
				VHAL_LOGE("read size mismatch. read=%ld, expected=%ld", read_size, sizeof(event_param));
				result = VHAL_ERR_SOCKET;
				break;
			}

			/* イベントごとの処理 */
			int32_t	ret{VHAL_SUCCESS};
			switch (event_param.event_type)
			{
				case captureEventType::VHAL_CAPTURE_EVENT_FRAME_CAPTURE:
					ret = p_capture_control_->SendFrame(event_param.input_type, event_param.get_buf_indx);
					if (VHAL_SUCCESS != ret)
					{
						VHAL_LOGW("SendFrame error. ret=%d", ret);
					}
					break;
				default:
					VHAL_LOGW("event_type is invalid. event_type=%d", event_param.event_type);
					break;
			}
		}
	} while (false);

//	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	フレームキャプチャスレッド処理
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalCaptureDraw::DrawThreadMain(void)
{
	VHAL_LOGV_IN();

	int32_t				ret{VHAL_SUCCESS};
	struct epoll_event	events[VHAL_EPOLL_SIZE_MAX]{};

	VHAL_LOGV_IN();

	running_ = true;
	VHAL_LOGI("start event loop in CaptureEventControl");

	bool done{false};
	while ((!done) && running_)
	{
		int32_t	count{0};
		count = epoll_wait(epoll_fd_, &events[0], VHAL_EPOLL_SIZE_MAX, -1);
		if (-1 == count)
		{
			VHAL_LOGE("epoll_wait error(%d).", errno);
		}
		else
		{
			if (1 != count)
			{
				/* count has to be 1. Just in case there is unexpected failure. */
				VHAL_LOGW("count error. count=%d", count);
			}

			if (events[0].data.fd == pair_fd_[kSocketPairRead])
			{
				uint32_t	source_event{0U};

				if (0U != (events[0].events & static_cast<uint32_t>(EPOLLIN)))
				{
					source_event |= kSourceEventReadable;
				}
				if (0U != (events[0].events & static_cast<uint32_t>(EPOLLOUT)))
				{
					source_event |= kSourceEventWritable;
				}
				if (0U != (events[0].events & static_cast<uint32_t>(EPOLLHUP)))
				{
					VHAL_LOGD("received EPOLLHUP");
					source_event |= kSourceEventHungup;
				}
				if (0U != (events[0].events & static_cast<uint32_t>(EPOLLERR)))
				{
					VHAL_LOGD("received EPOLLERR");
					source_event |= kSourceEventError;
				}

				ret = ExecEvent(source_event);
				if (VHAL_SUCCESS != ret)
				{
					/* イベント処理でエラー発生。フレームキャプチャ終了へ。 */
					VHAL_LOGW("ExecEvent in CaptureEventControl.");
					running_ = false;
					done = true;
				}
			}
		}
	}

	VHAL_LOGI("exited event loop in CaptureEventControl. exit code=%d", ret);

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
 フェールセーフNo：	F-VHAL-R-078
                	F-VHAL-R-079
                	F-VHAL-R-080
*****************************************************************************/
CVhalCaptureControl::CVhalCaptureControl(void)
{
	VHAL_LOGV("CVhalCaptureControl is created. this=%p", this);
	p_capture_control_ = this;
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalCaptureControl::~CVhalCaptureControl(void)
{
	VHAL_LOGV("CVhalCaptureControl is deleted. this=%p", this);
	Finalize();
}

/*****************************************************************************
 処理概要：	初期化処理（内部リソースの確保）
 引数    ：	wlrenderer::CWaylandRenderer *p_wayland_renderer	(i)WaylandRendererインスタンスポインタ
           	CVhalMainContro *p_main_ctl							(i)メイン制御インスタンスポインタ
           	CVhalPropertyControl *p_prop_ctl					(i)プロパティ制御インスタンスポインタ
           	CVhalObserverControl *p_observer_ctl				(i)状態監視制御インスタンスポインタ
 戻り値  ：	処理結果
           		VHAL_ERR_****		エラー
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalCaptureControl::Initialize(wlrenderer::CWaylandRenderer * const p_wayland_renderer,
										CVhalMainControl * const p_main_ctl,
										CVhalPropertyControl * const p_prop_ctl,
										CVhalObserverControl * const p_observer_ctl)
{
	int32_t	result{VHAL_SUCCESS};

	if ((nullptr == p_wayland_renderer) || (nullptr == p_main_ctl) || (nullptr == p_prop_ctl) || (nullptr == p_observer_ctl))
	{
		VHAL_LOGE("parameter error. p_renderer=%p p_main=%p p_main=%p p_observer_control=%p", p_wayland_renderer, p_main_ctl, p_prop_ctl, p_observer_ctl);
		result = VHAL_ERR_PARAM;
	}
	else
	{
		if (true == initialized_)
		{
			VHAL_LOGE("CVhalCaptureControl is already initialized");
			result = VHAL_ERR_PARAM;
		}
		else
		{
			p_renderer_ = p_wayland_renderer;
			p_main_ = p_main_ctl;
			p_prop_ = p_prop_ctl;
			p_observer_control_ = p_observer_ctl;

			p_route_ = std::make_unique<CVhalEventRoute>();
			int32_t ret{p_route_->Initialize()};
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("EventRoute Initialize error. ret=%d", ret);
				result = ret;
			}
			else
			{
				ret = p_main_->RegisterEventSource(p_route_.get());
				if (VHAL_SUCCESS != ret)
				{
					VHAL_LOGE("RegisterEventSource error. ret=%d", ret);
					result = ret;
				}
				else
				{
// MISRA C++-2008 Rule 16-2-1 #ifndef VHAL_SUPPORT_SETTING_INITIALIZE
					/* QCarCam 初期化 */
					// result = InitializeQcarcam();
					// if (VHAL_SUCCESS != result)
					// {
					// 	VHAL_LOGE("InitializeQcarcam() error. ret=%d", result);
					// }
// MISRA C++-2008 Rule 16-2-1 #endif
					/* BEVstep3 24/10/17 */
					if (true == kVhalRendererShm)
					{
						/* シミュレータ環境ではフォーマットをWL_SHM_FORMAT_XRGB8888とする */
						for ( auto& itr : device_entries_ )
						{
							itr.second.format = WL_SHM_FORMAT_XRGB8888;
						}
					}
					initialized_ = true;
				}
			}
		}
	}

	/* 失敗時のクリア */
	if (VHAL_SUCCESS != result)
	{
		Finalize();
	}

	return result;
}

/*****************************************************************************
 処理概要：	終了処理（内部リソースの解放）
 引数    ：	なし
 戻り値  ：	なし
 フェールセーフNo ：	F-VHAL-C-068
*****************************************************************************/
void CVhalCaptureControl::Finalize(void)
{
	/* QCarCam終了 */
	FinalizeQcarcam();

	if (nullptr != p_main_)
	{
		if (nullptr != p_route_)
		{
			p_main_->ClearEventSource(p_route_.get());
		}
		p_main_ = nullptr;
	}

	if (nullptr != p_route_)
	{
		p_route_ = nullptr;
	}

	p_prop_ = nullptr;
	p_observer_control_ = nullptr;

	initialized_ = false;
}

/*****************************************************************************
 処理概要：	キャプチャ設定
 引数    ：	CVhalLayoutManager*	p_layout_mng	(i)レイアウト制御インスタンスポインタ
           	captureInputType	input_type		(i)デバイス種別
           	uint32_t 			res_width		(i)解像度幅  （指定なしであれば'0'）
           	uint32_t 			res_height		(i)解像度高さ（指定なしであれば'0'）
 戻り値  ：	処理結果
           		VHAL_ERR_****				未初期化エラー
           		VHAL_SUCCESS				正常終了
 フェールセーフNo ：	F-VHAL-R-071
                    	F-VHAL-N-076
*****************************************************************************/
int32_t CVhalCaptureControl::OpenCapture(CVhalLayoutManager * const p_layout_mng, const captureInputType input_type, const uint32_t res_width, const uint32_t res_height)
{
	int32_t					result{VHAL_SUCCESS};
	std::unique_ptr<capturecontrol_info_t>	p_capture_ctrl_info{nullptr};

	VHAL_LOGV_IN();

	VHAL_LOGI("input_type[%d] res_width[%d] res_height[%d]", input_type, res_width, res_height);

	do
	{
		if (nullptr == p_layout_mng)
		{
			VHAL_LOGE("parameter error. p_layout_mng=%p", p_layout_mng);
			result = VHAL_ERR_PARAM;
			break;
		}

// MISRA C++-2008 Rule 16-2-1 #ifdef VHAL_SUPPORT_SETTING_INITIALIZE
		/* QCarCam 初期化 */
		int32_t ret{InitializeQcarcam()};
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("InitializeProcess() error. ret=%d", ret);
			result = ret;
			break;
		}
// MISRA C++-2008 Rule 16-2-1 #endif

		/* 対象デバイスの検索 */
		const auto itr_map = device_entries_.find(input_type);
		if (itr_map == device_entries_.end())
		{
			VHAL_LOGE("device [%d] is not supported.", input_type);
			result = VHAL_ERR_PARAM;
			break;
		}
		struct VhalCaptureDeviceEntry &entry{itr_map->second};

		/* query取得 */
		QCarCamInput_t	query_input{};
		QCarCamInputSrc_t inputSource {};
		uint32_t uiModeIndex {0U};
		ret = SettingQueryInput(entry.inputName, &query_input, inputSource, uiModeIndex, res_width, res_height);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("SettingQueryInput() error. ret=%d", ret);
			result = ret;
			break;
		}

		VHAL_LOGI("Set input: inputId=%d, res=%dx%d fmt=0x%08x fps=%.2f flags=0x%x",
			query_input.inputId,
			inputSource.width,
			inputSource.height,
			inputSource.colorFmt,
			inputSource.fps,
			query_input.flags);

		/* キャプチャコントロール管理構造体の生成 */
		p_capture_ctrl_info = std::make_unique<capturecontrol_info_t>(input_type);
		if (nullptr == p_capture_ctrl_info)
		{
			VHAL_LOGE("capturecontrol_info nullptr.");
			result = VHAL_ERR_PARAM;
			break;
		}

		/* ハンドル取得 */
		QCarCamHndl_t	hndl{QCARCAM_HNDL_INVALID};
		QCarCamOpen_t	openParams {};
		openParams.opMode = QCARCAM_OPMODE_ISP;		// (=1)
		openParams.numInputs = 1U;
		openParams.inputs[0].inputId = query_input.inputId;
		openParams.inputs[0].srcId = inputSource.srcId;
		openParams.inputs[0].inputMode = uiModeIndex;
		openParams.flags = QCARCAM_OPEN_FLAGS_REQUEST_MODE;
		uint32_t count{0U};
		QCarCamRet_e qret {QCARCAM_RET_OK};
		while (true)
		{
			qret = QCarCamOpen(&openParams, &hndl);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//			int32_t fail_ret{0};
//			bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem2("F-VHAL-R-071",count, fail_ret)};
//			if(true == fail)
//			{
//				qret = static_cast<QCarCamRet_e>(fail_ret);
//			}
//#endif
			if (QCARCAM_RET_OK == qret)
			{
				break;
			}
			count++;
			if (kVhalQcarcamRetryCount <= count)
			{
				break;
			}
			sif_mdelay(kVhalQcarcamRetryWait);
		}
		if (QCARCAM_RET_OK != qret)
		{
			VHAL_LOGE("QCarCamOpen failed retry count=%d", count);
			result = VHAL_ERR_QCARCAM_API;
			break;
		}
		p_capture_ctrl_info->SetHndl(hndl);
		VHAL_LOGI("QCarCamOpen hndl=0x%lXp, retry count=%d", hndl, count);

		/* 入力デバイスの解像度調整 */
		const uint32_t width {inputSource.width};
		const uint32_t height{inputSource.height};

		/* Wayland描画設定処理 */
		ret = SettingRendererConfig(&entry, p_capture_ctrl_info.get(), width, height);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("SettingRendererConfig() error. ret=%d", ret);
			result = ret;
			break;
		}
		ret = SettingIviId(p_layout_mng, p_capture_ctrl_info.get(), &entry);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("SettingIviId() error. ret=%d", ret);
			result = ret;
			break;
		}

		/* パラメータを設定(コールバック) */
		/* パラメータを設定(イベントmask設定) */
		ret = SettingParamEventCallBack(hndl);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("SettingEventCallBack(QCARCAM_PARAM_EVENT_CB/QCARCAM_PARAM_EVENT_MASK) failed. ret = %d", ret);
			result = VHAL_ERR_QCARCAM_API;
			break;
		}

		/* 描画バッファ設定処理 */
		ret = SettingBuffer(p_capture_ctrl_info.get(), inputSource.colorFmt, entry.buf_count, width, height);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("SettingBuffer() error. ret=%d", ret);
			result = ret;
			break;
		}
	} while (false);

	if (VHAL_SUCCESS == result)
	{
		/* 描画クラス生成(各デバイスごと) */
		p_capture_ctrl_info->AllocateCaptureDraw();
		CVhalCaptureDraw* const	p_capture_draw{p_capture_ctrl_info->GetCaptureDraw()};
// MISRA C++-2008 Rule 16-2-1 #ifdef VHAL_SUPPORT_FRAME_CNT
		// p_capture_ctrl_info->bef_time  = 0;
		// p_capture_ctrl_info->frame_cnt = 0;
// MISRA C++-2008 Rule 16-2-1 #endif
		capturecontrol_info_t *const p_capture_raw_ptr{p_capture_ctrl_info.get()};
		p_capture_ctrl_infos_.push_back(std::move(p_capture_ctrl_info));

		/* イベントスレッド起動 */
		int32_t ret{p_capture_draw->Initialize()};
		if (VHAL_SUCCESS != ret)
		{
			/* イベントスレッド起動失敗:呼び元からRelease処理を実行 */
			result = ret;
		}
		else
		{
			if (captureInputType::VHAL_CAPTURE_INPUT_HDMI == input_type)
			{
				/* キャプチャ開始 */
				result = StartCapture(input_type);
			}
			else
			{
				/* カメラキャプチャ動作通知 */
				p_captureRecv_listener_->NotifyReceiveCameraControl();
			}
			/* 黒画バッファをSend */
			ret = SendFrameBlackBuffer(p_capture_raw_ptr);
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGW("SendFrameBlackBuffer() error. ret=%d", ret);
			}
		}
	}
	else
	{
		if (nullptr != p_capture_ctrl_info)
		{
			/* スレッド関連以外のリソースを解放 */
			ReleaseProcess(p_capture_ctrl_info.get());
			p_capture_ctrl_info = nullptr;
		}
	}

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	キャプチャ設定解除
 引数    ：	captureInputType	input_type	(i)デバイス種別
 戻り値  ：	なし
*****************************************************************************/
void CVhalCaptureControl::CloseCapture(const captureInputType input_type)
{

	VHAL_LOGV_IN();

	VHAL_LOGI("input_type[%d]", input_type);

	do
	{
		const auto it_info = std::find_if(p_capture_ctrl_infos_.begin(), p_capture_ctrl_infos_.end(),
			[input_type](const std::unique_ptr<capturecontrol_info_t> &p_capture_ctrl_info) noexcept { return p_capture_ctrl_info->GetInputType() == input_type; });
		if (it_info == p_capture_ctrl_infos_.end())
		{
			VHAL_LOGW("capture_ctrl_info is not found.");
			break;
		}

		capturecontrol_info_t*	p_capture_ctrl_info{it_info->get()};
		if (nullptr == p_capture_ctrl_info)
		{
			VHAL_LOGW("parameter error. p_capture_ctrl_info=%p", p_capture_ctrl_info);
			break;
		}

		CVhalCaptureDraw*	p_capture_draw{nullptr};
		p_capture_draw = p_capture_ctrl_info->GetCaptureDraw();

		/* キャプチャ停止 */
		const int32_t	ret{StopCapture(input_type)};
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("StopCapture failed. ret = %d", ret);
		}

		/* バッファ領域解放前に先行してバッファの切り離しを実施 */
		wlrenderer::CWaylandRendererVideo*	p_renderer_video{nullptr};
		p_renderer_video  = p_capture_ctrl_info->GetRendererVideo();
		if (nullptr != p_renderer_video)
		{
			wlrenderer::CWaylandRendererConfig*	p_renderer_config{nullptr};
			p_renderer_config = p_capture_ctrl_info->GetRendererConfig();
			if (nullptr != p_renderer_config)
			{
				int32_t					wl_ret{WL_RENDERER_SUCCESS};
				std::vector<int32_t>	surface_ids{};
				p_renderer_config->GetSurfaceIds(surface_ids);
				wl_ret = p_renderer_video->SendBuffer(nullptr, surface_ids);
				if (WL_RENDERER_SUCCESS != wl_ret)
				{
					VHAL_LOGE("SendBuffer[nullptr] failed. ret = %d", wl_ret);
				}
			}
		}

		/* スレッド停止 */
		if (nullptr != p_capture_draw)
		{
			p_capture_ctrl_info->FreeCaptureDraw();
		}

		/* 映像キャプチャ準備状態通知2:Lo */
		p_captureRecv_listener_->NotifyReceiveCaptureStandby2(p_capture_ctrl_info->GetInputType(), false);

		/* スレッド関連以外のリソースを解放 */
		ReleaseProcess(p_capture_ctrl_info);

		/* キャプチャ管理の解放 */
		(void)p_capture_ctrl_infos_.erase(it_info);
		p_capture_ctrl_info = nullptr;
	} while (false);

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	キャプチャ開始
 引数    ：	const captureInputType	input_type		(i)デバイス種別
 戻り値  ：	処理結果
           		VHAL_ERR_****				エラー
           		VHAL_SUCCESS				正常終了
 フェールセーフNo ：	F-VHAL-R-076
*****************************************************************************/
int32_t CVhalCaptureControl::StartCapture(const captureInputType input_type) noexcept
{
	VHAL_LOGV_IN();

	int32_t	result{VHAL_SUCCESS};

	const auto it_info = std::find_if(p_capture_ctrl_infos_.begin(), p_capture_ctrl_infos_.end(),
		[input_type](const std::unique_ptr<capturecontrol_info_t> &p_capture_ctrl_info) noexcept -> bool{ return p_capture_ctrl_info->GetInputType() == input_type; });
	if (it_info == p_capture_ctrl_infos_.end())
	{
		VHAL_LOGE("capture_ctrl_info is not found.");
		result = VHAL_ERR_PARAM;
	}
	else
	{
		capturecontrol_info_t* const p_capture_ctrl_info{it_info->get()};
		if (nullptr == p_capture_ctrl_info)
		{
			VHAL_LOGW("parameter error. p_capture_ctrl_info=%p", p_capture_ctrl_info);
			result = VHAL_ERR_PARAM;
		}
		else
		{
			if (true != p_capture_ctrl_info->GetQcarcamStart())
			{
				p_capture_ctrl_info->LockMtxQcarcam();
				QCarCamRet_e qret {QCarCamReserve(p_capture_ctrl_info->GetHndl())};
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM	// F-VHAL-N-325
//				int32_t fail_ret{static_cast<int32_t>(qret)};
//				CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-N-325",fail_ret);
//				qret = static_cast<QCarCamRet_e>(fail_ret);
//#endif
				if (QCARCAM_RET_OK != qret)
				{
					VHAL_LOGE("QCarCamReserve failed. ret = %d", qret);
					result = VHAL_ERR_QCARCAM_API;
				}
				else
				{
					qret = QCarCamStart(p_capture_ctrl_info->GetHndl());
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//					int32_t fail_ret2{0};
//					bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-076",fail_ret2)};
//					if(true == fail)
//					{
//						qret = static_cast<QCarCamRet_e>(fail_ret2);
//					}
//#endif
					if (QCARCAM_RET_OK != qret)
					{
						VHAL_LOGE("QCarCamStart failed. ret = %d", qret);
						result = VHAL_ERR_QCARCAM_API;
					}
					else
					{
						for (uint32_t buf_index{0U}; buf_index < p_capture_ctrl_info->GetBufCnt(); ++buf_index)
						{
							uint32_t	request_id{p_capture_ctrl_info->GetSubmitRequestId()};
							SettingSubmitRequest(p_capture_ctrl_info->GetHndl(), request_id, buf_index, qret);
							if (QCARCAM_RET_OK != qret)
							{
								/* エラーメッセージはSettingSubmitRequestにて出力 */
								result = VHAL_ERR_QCARCAM_API;
								break;
							}
						}
						if (VHAL_SUCCESS == result)
						{
							p_capture_ctrl_info->SetQcarcamStart(true);
							p_capture_ctrl_info->SetCaptureChk(false);
							VHAL_LOGI("QCarCamStart.");
//#ifdef VHAL_SUPPORT_RESET_CAPTURE
							StartInterruptObserver(p_capture_ctrl_info);
//#endif
						}
					}
				}
				p_capture_ctrl_info->UnLockMtxQcarcam();
			}
		}
	}

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	キャプチャ停止
 引数    ：	const captureInputType	input_type			(i)デバイス種別
           	const bool				setting_black_buf	(i)黒画設定　（指定なしであれば'true'[黒画有効]）
 戻り値  ：	処理結果
           		VHAL_ERR_****				エラー
           		VHAL_SUCCESS				正常終了
 フェールセーフNo ：	F-VHAL-N-077
*****************************************************************************/
int32_t CVhalCaptureControl::StopCapture(const captureInputType input_type, const bool setting_black_buf) noexcept
{
	VHAL_LOGV_IN();

	int32_t	result{VHAL_SUCCESS};

	const auto it_info = std::find_if(p_capture_ctrl_infos_.begin(), p_capture_ctrl_infos_.end(),
		[input_type](const std::unique_ptr<capturecontrol_info_t> &p_capture_ctrl_info) noexcept { return p_capture_ctrl_info->GetInputType() == input_type; });
	if (it_info == p_capture_ctrl_infos_.end())
	{
		VHAL_LOGE("capture_ctrl_info is not found.");
		result = VHAL_ERR_PARAM;
	}
	else
	{
		capturecontrol_info_t* const p_capture_ctrl_info{it_info->get()};
		if (nullptr == p_capture_ctrl_info)
		{
			VHAL_LOGW("parameter error. p_capture_ctrl_info=%p", p_capture_ctrl_info);
			result = VHAL_ERR_PARAM;
		}
		else
		{
			if (false != p_capture_ctrl_info->GetQcarcamStart())
			{
				p_capture_ctrl_info->LockMtxQcarcam();
				QCarCamRet_e	qret{QCarCamStop(p_capture_ctrl_info->GetHndl())};

//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//				int32_t fail_ret{0};
//				bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-077",fail_ret)};
//				if(true == fail)
//				{
//					qret = static_cast<QCarCamRet_e>(fail_ret);
//				}
//#endif
				if (QCARCAM_RET_OK != qret)
				{
					VHAL_LOGE("QCarCamStop failed. ret = %d", qret);
					result = VHAL_ERR_QCARCAM_API;
				}
				else
				{
					qret = QCarCamRelease(p_capture_ctrl_info->GetHndl());
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM	// F-VHAL-N-326
//					int32_t fail_ret2{static_cast<int32_t>(qret)};
//					CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-N-326",fail_ret2);
//					qret = static_cast<QCarCamRet_e>(fail_ret2);
//#endif
					if (QCARCAM_RET_OK != qret)
					{
						VHAL_LOGE("QCarCamRelease failed. ret = %d", qret);
						result = VHAL_ERR_QCARCAM_API;
					}
					else
					{
						p_capture_ctrl_info->SetQcarcamStart(false);
						VHAL_LOGI("QCarCamStop.");
//#ifdef VHAL_SUPPORT_RESET_CAPTURE
						StopInterruptObserver(p_capture_ctrl_info);
//#endif
					}
				}
				p_capture_ctrl_info->UnLockMtxQcarcam();

				/* 黒画バッファをSend */
				if (true == setting_black_buf)
				{
					int32_t	ret{VHAL_SUCCESS};
					ret = SendFrameBlackBuffer(p_capture_ctrl_info);
					if (VHAL_SUCCESS != ret)
					{
						VHAL_LOGW("SendFrameBlackBuffer() error. ret=%d", ret);
					}
				}
			}
		}
	}

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	キャプチャ動作取得
 引数    ：	const captureInputType	input_type		(i)デバイス種別
 戻り値  ：	キャプチャ状態
           		true		キャプチャ動作中
           		false		キャプチャ停止中
*****************************************************************************/
bool CVhalCaptureControl::IsCaptureStatus(const captureInputType input_type) const noexcept
{
	VHAL_LOGV_IN();

	bool	ret{false};

	do
	{
		const auto it_info = std::find_if(p_capture_ctrl_infos_.begin(), p_capture_ctrl_infos_.end(),
			[input_type](const std::unique_ptr<capturecontrol_info_t> &p_capture_ctrl_info) noexcept { return p_capture_ctrl_info->GetInputType() == input_type; });
		if (it_info == p_capture_ctrl_infos_.end())
		{
			VHAL_LOGE("capture_ctrl_info is not found.");
			break;
		}

		const capturecontrol_info_t* const p_capture_ctrl_info{it_info->get()};
		if (nullptr == p_capture_ctrl_info)
		{
			VHAL_LOGW("parameter error. p_capture_ctrl_info=%p", p_capture_ctrl_info);
			break;
		}

		ret = p_capture_ctrl_info->GetQcarcamStart();

	} while (false);

	VHAL_LOGV_OUT();
	return ret;
}

/*****************************************************************************
 処理概要：	映像更新開始
 引数    ：	captureInputType	input_type	(i)デバイス種別
 戻り値  ：	処理結果
           		VHAL_ERR_****				エラー
           		VHAL_SUCCESS				正常終了
*****************************************************************************/
int32_t CVhalCaptureControl::Start(const captureInputType input_type)
{
	int32_t	result{VHAL_SUCCESS};

	VHAL_LOGV_IN();
	VHAL_LOGI("input_type[%d]", input_type);

	do
	{
		const auto it_info = std::find_if(p_capture_ctrl_infos_.begin(), p_capture_ctrl_infos_.end(),
			[input_type](const std::unique_ptr<capturecontrol_info_t> &p_capture_ctrl_info) noexcept { return p_capture_ctrl_info->GetInputType() == input_type; });
		if (it_info == p_capture_ctrl_infos_.end())
		{
			VHAL_LOGE("capture_ctrl_info is not found.");
			result = VHAL_ERR_PARAM;
			break;
		}

		capturecontrol_info_t* const p_capture_ctrl_info{it_info->get()};
		if (nullptr == p_capture_ctrl_info)
		{
			VHAL_LOGE("parameter error. p_capture_ctrl_info=%p", p_capture_ctrl_info);
			result = VHAL_ERR_PARAM;
			break;
		}

		/* キャプチャ映像送信状態 */
		p_capture_ctrl_info->SetCaptureSend(true);

	} while (false);

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	映像更新停止
 引数    ：	captureInputType	input_type	(i)デバイス種別
 戻り値  ：	処理結果
           		VHAL_ERR_****				エラー
           		VHAL_SUCCESS				正常終了
*****************************************************************************/
int32_t CVhalCaptureControl::Stop(const captureInputType input_type)
{
	int32_t	result{VHAL_SUCCESS};

	VHAL_LOGV_IN();
	VHAL_LOGI("input_type[%d]", input_type);

	do
	{
		const auto it_info = std::find_if(p_capture_ctrl_infos_.begin(), p_capture_ctrl_infos_.end(),
			[input_type](const std::unique_ptr<capturecontrol_info_t> &p_capture_ctrl_info) noexcept { return p_capture_ctrl_info->GetInputType() == input_type; });
		if (it_info == p_capture_ctrl_infos_.end())
		{
			VHAL_LOGW("capture_ctrl_info is not found.");
			result = VHAL_ERR_PARAM;
			break;
		}
		else
		{
			capturecontrol_info_t* const p_capture_ctrl_info{it_info->get()};
			if (nullptr == p_capture_ctrl_info)
			{
				VHAL_LOGE("parameter error. p_capture_ctrl_info=%p", p_capture_ctrl_info);
				result = VHAL_ERR_PARAM;
				break;
			}

			/* キャプチャ映像送信停止状態 */
			p_capture_ctrl_info->SetCaptureSend(false);
		}
	} while (false);

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	キャプチャフレーム送信
 引数    ：	captureInputType	input_type		(i)デバイス種別
           	int32_t				get_buf_indx	(i)送信バッファインデックス
 戻り値  ：	処理結果
           		VHAL_ERR_****				エラー
           		VHAL_SUCCESS				正常終了
*****************************************************************************/
int32_t CVhalCaptureControl::SendFrame(const captureInputType input_type, const int32_t get_buf_indx)
{
	int32_t	result{VHAL_SUCCESS};

//	VHAL_LOGV_IN();
	do
	{
		const auto it_info = std::find_if(p_capture_ctrl_infos_.begin(), p_capture_ctrl_infos_.end(),
			[input_type](const std::unique_ptr<capturecontrol_info_t> &p_capture_ctrl_info) noexcept { return p_capture_ctrl_info->GetInputType() == input_type; });
		if (it_info == p_capture_ctrl_infos_.end())
		{
			VHAL_LOGW("capture_ctrl_info is not found.");
			break;
		}

		capturecontrol_info_t* const p_capture_ctrl_info{it_info->get()};
		if (nullptr == p_capture_ctrl_info)
		{
			VHAL_LOGE("parameter error. p_capture_ctrl_info=%p", p_capture_ctrl_info);
			result = VHAL_ERR_PARAM;
			break;
		}

		if (false == p_capture_ctrl_info->GetQcarcamStart())
		{
			break;
		}

		/* キャプチャ映像送信状態 */
		if (true == p_capture_ctrl_info->GetCaptureSend())
		{
			if ((0 > get_buf_indx) || (p_capture_ctrl_info->GetBufCnt() <= static_cast<uint32_t>(get_buf_indx)))
			{
				VHAL_LOGE("parameter error. get_buf_indx=%d", get_buf_indx);
				result = VHAL_ERR_PARAM;
				break;
			}
			/* Sndbuf */
			capturecontrol_bufinfo_t*	p_buf_info{nullptr};
			p_buf_info = &p_capture_ctrl_info->GetBufInfo()[get_buf_indx];
			if (nullptr == p_buf_info)
			{
				VHAL_LOGE("parameter error. p_buf_info=%p", p_buf_info);
				result = VHAL_ERR_PARAM;
				break;
			}
			wlrenderer::CWaylandRendererVideoBuffer*	p_renderer_buffer{nullptr};
			wlrenderer::CWaylandRendererVideo*			p_renderer_video{nullptr};
			wlrenderer::CWaylandRendererConfig*			p_renderer_config{nullptr};
			p_renderer_buffer = p_buf_info->p_renderer_buffer;
			p_renderer_video  = p_capture_ctrl_info->GetRendererVideo();
			p_renderer_config = p_capture_ctrl_info->GetRendererConfig();
			if ((nullptr == p_renderer_buffer) || (nullptr == p_renderer_video) || (nullptr == p_renderer_config))
			{
				VHAL_LOGE("parameter error. p_renderer_buffer=%p, p_renderer_video=%p, p_renderer_config=%p", p_renderer_buffer, p_renderer_video, p_renderer_config);
				result = VHAL_ERR_PARAM;
				break;
			}
			int32_t					ret{WL_RENDERER_SUCCESS};
			std::vector<int32_t>	surface_ids{};
			p_renderer_config->GetSurfaceIds(surface_ids);
			ret = p_renderer_video->SendBuffer(p_renderer_buffer, surface_ids);
			if (WL_RENDERER_SUCCESS != ret)
			{
				VHAL_LOGE("SendBuffer[%d] failed. ret = %d", get_buf_indx, ret);
				result = VHAL_ERR_WAYLAND;
				break;
			}
		}

		/* カメラのみ */
		if (captureInputType::VHAL_CAPTURE_INPUT_CAMERA == input_type)
		{
			/* 映像キャプチャ表示準備通知 */
			p_captureRecv_listener_->NotifyReceiveCaptureFrame();
		}
	} while (false);

//	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	キャプチャイベント通知
 引数    ：	QCarCamHndl_t	hndl			(i)ハンドル
           	int32_t			get_buf_indx	(i)送信バッファインデックス
 戻り値  ：	なし
*****************************************************************************/
void CVhalCaptureControl::NotifyCaptureEvent(const QCarCamHndl_t hndl, const int32_t get_buf_indx)
{
//	VHAL_LOGV_IN();

	const auto it_info = std::find_if(p_capture_ctrl_infos_.begin(), p_capture_ctrl_infos_.end(),
		[hndl](const std::unique_ptr<capturecontrol_info_t> &p_capture_ctrl_info) noexcept { return p_capture_ctrl_info->GetHndl() == hndl; });
	if (it_info == p_capture_ctrl_infos_.end())
	{
		VHAL_LOGW("capture_ctrl_info is not found.");
	}
	else
	{
		do
		{
			capturecontrol_info_t* const p_capture_ctrl_info{it_info->get()};
			if (nullptr == p_capture_ctrl_info)
			{
				VHAL_LOGE("parameter error. p_capture_ctrl_info=%p", p_capture_ctrl_info);
				break;
			}

			/* 初回キャプチャ時のみログ出力 */
			if (true != p_capture_ctrl_info->GetCaptureChk())
			{
				VHAL_LOGI("First Capture[Input_type:%d][Index:%d]", p_capture_ctrl_info->GetInputType(), get_buf_indx);
				p_capture_ctrl_info->SetCaptureChk(true);
			}

// MISRA C++-2008 Rule 16-2-1 #ifdef VHAL_SUPPORT_FRAME_CNT
			// unsigned long long now_time = 0;
			// VhalGetTime(&now_time);
			// p_capture_ctrl_info->frame_cnt++;
			// if((now_time - p_capture_ctrl_info->bef_time) >= 1000)
			// {
			// 	VHAL_LOGI("input_type[%d] is [%d]fps", p_capture_ctrl_info->input_type, p_capture_ctrl_info->frame_cnt);
			// 	p_capture_ctrl_info->bef_time = now_time;
			// 	p_capture_ctrl_info->frame_cnt = 0;
			// }
// MISRA C++-2008 Rule 16-2-1 #endif
			event_param_t	event_param{};
			event_param.input_type        = p_capture_ctrl_info->GetInputType();
			event_param.event_type        = captureEventType::VHAL_CAPTURE_EVENT_FRAME_CAPTURE;
			event_param.get_buf_indx = get_buf_indx;
			(void)p_capture_ctrl_info->GetCaptureDraw()->WriteEvent(event_param);
		} while (false);
	}

//	VHAL_LOGV_OUT();
}

/* 24MMマージで誤らないためのコメント：BEVstep3ではSetHdcpAuthResultRseを削除 */

/*****************************************************************************
 処理概要：	イベントリスナー登録
 引数    ：	CVhalCaptureReceiveEventListenerBase* p_listener		(i)リスナーポインタ
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM			パラメータ不正
           		VHAL_SUCCESS			正常終了
****************************************************************************/
int32_t CVhalCaptureControl::RegisterEventListener(CVhalCaptureReceiveEventListenerBase* const p_listener)
{
	int32_t	result{VHAL_SUCCESS};

	if (nullptr == p_listener)
	{
		VHAL_LOGE("p_listener is null");
		result = VHAL_ERR_PARAM;
	}
	else
	{
		p_captureRecv_listener_ = p_listener;
	}

	return result;
}

/*****************************************************************************
 処理概要：	キャプチャ設定解除(リソース解放)
 引数    ：	capturecontrol_info_t*	p_capture_ctrl_info	(i)キャプチャコントロール管理構造体ポインタ
 戻り値  ：	なし
 フェールセーフNo ：	F-VHAL-C-072
*****************************************************************************/
void CVhalCaptureControl::ReleaseProcess(capturecontrol_info_t * const p_capture_ctrl_info)
{
	VHAL_LOGV_IN();

	if (nullptr != p_capture_ctrl_info)
	{
		/* 映像バッファ領域の解放 */
		if (nullptr != p_capture_ctrl_info->GetRendererVideo())
		{
			p_renderer_->RemoveRendererVideo(p_capture_ctrl_info->GetRendererVideo());
			p_capture_ctrl_info->SetRendererVideo(nullptr);
		}

		/* 描画設定の解放 */
		if (nullptr != p_capture_ctrl_info->GetRendererConfig())
		{
			p_capture_ctrl_info->FreeRenderConfig();
		}

		/* ハンドルクローズ */
		if (QCARCAM_HNDL_INVALID != p_capture_ctrl_info->GetHndl())
		{
			QCarCamRet_e	qret{QCARCAM_RET_OK};
			qret = QCarCamUnregisterEventCallback(p_capture_ctrl_info->GetHndl());
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM	// F-VHAL-N-327
//			int32_t fail_ret{static_cast<int32_t>(qret)};
//			CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-N-327",fail_ret);
//			qret = static_cast<QCarCamRet_e>(fail_ret);
//#endif
			if (QCARCAM_RET_OK != qret)
			{
				VHAL_LOGE("QCarCamUnregisterEventCallback failed. ret = %d", qret);
			}
			qret = QCarCamClose(p_capture_ctrl_info->GetHndl());
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//			int32_t fail_ret2{0};
//			bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-C-072",fail_ret2)};
//			if(true == fail)
//			{
//				qret = static_cast<QCarCamRet_e>(fail_ret2);
//			}
//#endif
			if (QCARCAM_RET_OK != qret)
			{
				VHAL_LOGE("QCarCamClose failed. ret = %d", qret);
			}
			p_capture_ctrl_info->SetHndl(QCARCAM_HNDL_INVALID);
		}

		/* バッファ管理領域の解放 */
		if (nullptr != p_capture_ctrl_info->GetBufInfo())
		{
			free(p_capture_ctrl_info->GetBufInfo());
			p_capture_ctrl_info->SetBufInfo(nullptr);
		}
	}

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	QCarCam初期化処理
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_ERR_QCARCAM_API	QCarCamAPI不正
           		VHAL_SUCCESS			正常終了
 フェールセーフNo ：	F-VHAL-R-067A
                    	F-VHAL-R-067B
*****************************************************************************/
int32_t CVhalCaptureControl::InitializeQcarcam(void)
{
	int32_t	result{VHAL_SUCCESS};

	VHAL_LOGV_IN();

	if (!initialized_qcarcam_)
	{
		QCarCamInit_t qcarcam_init{};

		qcarcam_init.apiVersion	= QCARCAM_VERSION;
		QCarCamRet_e	qret{QCARCAM_RET_OK};
		
		/* QCarCamInitializeループ：最大回数=50回 */
		for (uint32_t count{1U}; count <= kVhalQcarcamRetryCount; ++count)
		{
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//			int32_t fail_ret{static_cast<int32_t>(QCARCAM_RET_OK)};
//			CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-R-067",fail_ret, count);
//			qret = static_cast<QCarCamRet_e>(fail_ret);
//			if (QCARCAM_RET_OK == qret)
//#endif
			qret = QCarCamInitialize(&qcarcam_init);
			/* リトライ系 */
			/* メモリ不足 or サーバとの通信未確立 */
			if ((QCARCAM_RET_NOMEM == qret) || (QCARCAM_RET_NOT_FOUND == qret))
			{
				if (kVhalQcarcamRetryCount > count)
				{
					/* リトライ中 (wait200ms) */
					sif_mdelay(kVhalQcarcamRetryWait);
				}
				else
				{
					/* リトライオーバー */
					VHAL_LOGE("QCarCamInitialize() retry over qret=%d count=%u", qret, count);
					result = VHAL_ERR_QCARCAM_API;
					break;
				}
			}
			/* 非リトライ系 */
			else
			{
				if (QCARCAM_RET_OK != qret)
				{
					/* エラー時 */
					VHAL_LOGE("QCarCamInitialize() failed. qret=%d", qret);
					result = VHAL_ERR_QCARCAM_API;
				}
				else
				{
					/* 正常時 */
					initialized_qcarcam_ = true;
				}
				/* 正常時・エラー時共通 */
				VHAL_LOGV("QCarCamInitialize() ret=%d retry count=%u", qret, count);
				break;
			}
		}
	}

	VHAL_LOGV_OUT();

	return result;
}

/*****************************************************************************
 処理概要：	QCarCam終了処理
 引数    ：	なし
 戻り値  ：	なし
 フェールセーフNo ：	F-VHAL-C-068
*****************************************************************************/
void CVhalCaptureControl::FinalizeQcarcam(void)
{
	VHAL_LOGV_IN();

	/* 残っているタイマーイベントがあれば削除 */
	for (auto& elem : open_capture_timer_list_)
	{
		CVhalOpenCaptureTimer* const	p_timer{elem.second.get()};
		if (nullptr != p_timer)
		{
			p_timer->EndTimer();
			p_timer->Finalize();
		}
	}
	open_capture_timer_list_.clear();
	sethdcpauthkey_ = false;

	if (initialized_qcarcam_)
	{
		const QCarCamRet_e qret{QCarCamUninitialize()};
//	#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//		int32_t fail_ret{static_cast<int32_t>(QCARCAM_RET_OK)};
//		CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-C-068",fail_ret);
//		QCarCamRet_e* p_qret{const_cast<QCarCamRet_e*>(&qret)};
//		*p_qret = static_cast<QCarCamRet_e>(fail_ret);
//	#endif
		if (QCARCAM_RET_OK != qret)
		{
			VHAL_LOGW("QCarCamUninitialize failed. ret = %d", qret);
		}
		initialized_qcarcam_ = false;
	}
	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	QCarCam入力情報処理
 引数    ：	const char*					inputName		(i)QCarCam入力識別子
           	QCarCamInput_t*				p_query_input	(o)QCarCam入力情報ポインタ
			QCarCamInputSrc_t&			inputSource		(o)QCarCam入力ソース情報ポインタ
           	uint32_t&					uiModeIndex		(o)モードIndex
           	uint32_t 					res_width		(i)解像度幅　（指定なしであれば'0'）
           	uint32_t 					res_height		(i)解像度高さ（指定なしであれば'0'）
 戻り値  ：	処理結果
           		VHAL_ERR_QCARCAM_API	QCarCamAPI不正
           		VHAL_SUCCESS			正常終了
 フェールセーフNo ：	F-VHAL-R-069A
                    	F-VHAL-R-069B
                    	F-VHAL-R-070
*****************************************************************************/
int32_t CVhalCaptureControl::SettingQueryInput(	const char* inputName,
												QCarCamInput_t* const p_query_input,
												QCarCamInputSrc_t& inputSource,
												uint32_t& uiModeIndex,
												const uint32_t res_width,
												const uint32_t res_height)
{
	int32_t				result{VHAL_SUCCESS};
	QCarCamInput_t*	p_Inputs{nullptr};

	VHAL_LOGV_IN();

	do
	{
		QCarCamRet_e	qret{QCARCAM_RET_OK};
		uint32_t		queryNumInputs{0U};
		uint32_t		exec_cnt{0U};

		/* query数取得 */
		do
		{
			exec_cnt++;
			qret = QCarCamQueryInputs(nullptr, 0U, &queryNumInputs);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//			int32_t fail_ret{0};
//			bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem2("F-VHAL-R-070",exec_cnt,fail_ret)};
//			if(true == fail)
//			{
//				qret = static_cast<QCarCamRet_e>(fail_ret);
//				queryNumInputs = 0U;
//			}
//#endif
			if ((QCARCAM_RET_OK == qret) || (QCARCAM_RET_UNSUPPORTED == qret) || (QCARCAM_RET_BADPARAM == qret))
			{
				break;
			}
			sif_mdelay(kVhalQcarcamRetryWait);
		} while ((QCARCAM_RET_BUSY == qret) && (kVhalQcarcamRetryCount > exec_cnt));
		if (QCARCAM_RET_OK != qret)
		{
			VHAL_LOGE("QCarCamQueryInputs[%d][exec_cnt:%d] failed. ", qret, exec_cnt);
			result = VHAL_ERR_QCARCAM_API;
			break;
		}
		VHAL_LOGI("QCarCamQueryInputs[queryNumInputs:%d][exec_cnt:%d]", queryNumInputs, exec_cnt);

		/* query取得 */
		uint32_t	queryFilled{0U};
		p_Inputs = static_cast<QCarCamInput_t *>(calloc(static_cast<size_t>(queryNumInputs), sizeof(*p_Inputs)));
		if (nullptr == p_Inputs)
		{
			VHAL_LOGE("calloc() error.");
			result = VHAL_ERR_MEMORY;
			break;
		}
		exec_cnt = 0U;
		do
		{
			exec_cnt++;
			qret = QCarCamQueryInputs(p_Inputs, queryNumInputs, &queryFilled);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//			int32_t fail_ret{0};
//			bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem2("F-VHAL-R-069",exec_cnt,fail_ret)};
//			if(true == fail)
//			{
//				qret = static_cast<QCarCamRet_e>(fail_ret);
//			}
//#endif
			if ((QCARCAM_RET_OK == qret) || (QCARCAM_RET_UNSUPPORTED == qret) || (QCARCAM_RET_BADPARAM == qret))
			{
				break;
			}
			sif_mdelay(kVhalQcarcamRetryWait);
		} while ((QCARCAM_RET_BUSY == qret) && (kVhalQcarcamRetryCount > exec_cnt));
		if (QCARCAM_RET_OK != qret)
		{
			VHAL_LOGE("QCarCamQueryInputs[%d][exec_cnt:%d] failed. ", qret, exec_cnt);
			result = VHAL_ERR_QCARCAM_API;
			break;
		}

		uint32_t	querycnt{0U};
		for (querycnt = 0U; querycnt < queryFilled; querycnt++)
		{
			VHAL_LOGI("%d: inputId=%d, inputName=%s numModes=%d",
				querycnt,
				p_Inputs[querycnt].inputId,
				p_Inputs[querycnt].inputName,
				p_Inputs[querycnt].numModes);

			uint32_t uiModes {p_Inputs[querycnt].numModes};
			QCarCamInputModes_t queryInputModes {};
			QCarCamMode_t inputModes[uiModes] {};
			queryInputModes.numModes = uiModes;
			queryInputModes.pModes = inputModes;
			qret = QCarCamQueryInputModes(p_Inputs[querycnt].inputId, &queryInputModes);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM	// F-VHAL-R-323
//			int32_t fail_ret{static_cast<int32_t>(qret)};
//			CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-R-323",fail_ret);
//			qret = static_cast<QCarCamRet_e>(fail_ret);
//#endif
			if (QCARCAM_RET_OK != qret)
			{
				VHAL_LOGE("QCarCamQueryInputModes[%d][inputId:%u] failed. ", qret, p_Inputs[querycnt].inputId);
				result = VHAL_ERR_QCARCAM_API;
				break;
			}
			else
			{
				/* log all modes and sources */
				for (uint32_t uiMode {0U}; uiMode < uiModes; uiMode ++)
				{
					for (uint32_t uiSource {0U}; uiSource < inputModes[uiMode].numSources; uiSource ++)
					{
						QCarCamInputSrc_t src {inputModes[uiMode].sources[uiSource]};
						VHAL_LOGI("Mode=%u Source=%u res=%ux%u fps=%.2f", uiMode, uiSource, src.width, src.height, src.fps);
					}
				}
				/* get source */
				if ((strlen(p_Inputs[querycnt].inputName) == strlen(inputName))
				 && (0 == strncmp(p_Inputs[querycnt].inputName, inputName, strlen(inputName))))
				{
					(void)memcpy(p_query_input, &p_Inputs[querycnt], sizeof(QCarCamInput_t));
					if (!GetInputSource(inputModes, uiModes, res_width, res_height, inputSource, uiModeIndex))
					{
						VHAL_LOGW("Unmatch %ux%u [inputId:%u]. ", res_width, res_height, p_Inputs[querycnt].inputId);
					}
					break;
				}
			}
		}
		if (VHAL_SUCCESS != result)
		{
			break;
		}

		if (queryFilled <= querycnt)
		{
			VHAL_LOGE("Unmatch query_inputs. ");
			result = VHAL_ERR_QCARCAM_API;
		}
	} while (false);

	if (nullptr != p_Inputs)
	{
		free(p_Inputs);
		p_Inputs = nullptr;
	}

	VHAL_LOGV_OUT();

	return result;
}

/*****************************************************************************
 処理概要：	指定サイズに対応した入力ソースと入力モードindexを返す
 引数    ：	QCarCamMode_t*		inputModes	(i)入力モード配列
			uint32_t			uiModes		(i)入力モード配列の要素数
			uint32_t			width		(i)指定サイズ幅
			uint32_t			height		(i)指定サイズ高さ
			QCarCamInputSrc_t&	inputSource	(o)指定サイズに対応した入力ソース
			uint32_t&			uiModeIndex	(o)指定サイズに対応した入力モードindex
 戻り値  ：	処理結果
           		false	指定サイズに対応した入力ソースが見つからない
           		true	正常終了
*****************************************************************************/
bool CVhalCaptureControl::GetInputSource(const QCarCamMode_t* inputModes,
										 const uint32_t uiModes,
										 const uint32_t width,
										 const uint32_t height,
										 QCarCamInputSrc_t& inputSource,
										 uint32_t& uiModeIndex)
{
	bool bRtn {true};
	uiModeIndex = 0U;
	inputSource = inputModes[uiModeIndex].sources[0];
	if ((0U < width) && (0U < height))
	{
		bool bFound {false};
		for (uint32_t uiMode {0U}; uiMode < uiModes; uiMode ++)
		{
			for (uint32_t uiSource {0U}; uiSource < inputModes[uiMode].numSources; uiSource ++)
			{
				QCarCamInputSrc_t src {inputModes[uiMode].sources[uiSource]};
				if ((src.width == width) && (src.height == height))
				{
					uiModeIndex = uiMode;
					inputSource = src;
					bFound = true;
					break;
				}
			}
			if (bFound)
			{
				break;
			}
		}
		if (!bFound)
		{
			bRtn = false;
		}
	}
	return bRtn;
}

/*****************************************************************************
 処理概要：	Wayland描画設定処理
 引数    ：	VhalCaptureDeviceEntry*	p_entry				(i)デバイス管理構造体ポインタ
           	capturecontrol_info_t*	p_capture_ctrl_info	(i)キャプチャコントロール管理構造体ポインタ
           	uint32_t				width				(i)幅
           	uint32_t				height				(i)高さ
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM	パラメータ不正
           		VHAL_SUCCESS	正常終了
*****************************************************************************/
int32_t CVhalCaptureControl::SettingRendererConfig(VhalCaptureDeviceEntry * const p_entry, capturecontrol_info_t * const p_capture_ctrl_info, const uint32_t width, const uint32_t height)
{
	int32_t	result{VHAL_SUCCESS};

	VHAL_LOGV_IN();

	if (nullptr == p_entry)
	{
		VHAL_LOGE("parameter error. p_entry=%p", p_entry);
		result = VHAL_ERR_PARAM;
	}
	else
	{
		p_capture_ctrl_info->AllocateRendererConfig();
		/* BEVstep3 24/10/17 */
		wlrenderer::CWaylandRendererVideo::VideoBufferType	videoBufferType{wlrenderer::CWaylandRendererVideo::VIDEO_BUFFER_TYPE_GBM};
		if (true == kVhalRendererShm)
		{
			/* シミュレータ環境の場合 */
			videoBufferType = wlrenderer::CWaylandRendererVideo::VIDEO_BUFFER_TYPE_SHM;
		}
		p_capture_ctrl_info->GetRendererConfig()->SetBufferType(static_cast<int32_t>(videoBufferType));

		uint32_t	buf_count{I32ToUI32(p_entry->buf_count)};
		/* 黒描画用のバッファを用意する */
		++buf_count;
		p_capture_ctrl_info->GetRendererConfig()->SetBufferCount(buf_count);

		p_capture_ctrl_info->GetRendererConfig()->SetWidth(width);
		p_capture_ctrl_info->GetRendererConfig()->SetHeight(height);
		p_capture_ctrl_info->GetRendererConfig()->SetFormat(p_entry->format);
		p_capture_ctrl_info->GetRendererConfig()->SetLoopBuffer(true);
		p_capture_ctrl_info->GetRendererConfig()->SetColorSpace(p_entry->color_space);

		VHAL_LOGI("p_renderer_config=%p buf_count=%u width=%u height=%u color_space=%d", p_capture_ctrl_info->GetRendererConfig(),
			buf_count, width, height, p_entry->color_space);

	}

	VHAL_LOGV_OUT();

	return result;
}

/*****************************************************************************
 処理概要：	サーフェスID設定処理
 引数    ：	CVhalLayoutManager*		p_layout_mng		(i)レイアウト制御インスタンスポインタ
           	capturecontrol_info_t*	p_capture_ctrl_info	(i)キャプチャコントロール管理構造体ポインタ
           	VhalCaptureDeviceEntry*	p_entry				(i)デバイス管理構造体ポインタ
 戻り値  ：	処理結果
           		VHAL_ERR_QCARCAM_API	QCarCamAPI不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalCaptureControl::SettingIviId(CVhalLayoutManager * const p_layout_mng, capturecontrol_info_t * const p_capture_ctrl_info, VhalCaptureDeviceEntry * const p_entry)
{
	int32_t	result{VHAL_SUCCESS};

	VHAL_LOGV_IN();

	do
	{
		if ((nullptr == p_layout_mng) || (nullptr == p_capture_ctrl_info) || (nullptr == p_entry))
		{
			VHAL_LOGE("parameter error. p_layout_mng=%p, p_capture_ctrl_info=%p, p_entry=%p", p_layout_mng, p_capture_ctrl_info, p_entry);
			result = VHAL_ERR_PARAM;
			break;
		}

		wlrenderer::CWaylandRendererConfig*	p_renderer_config{nullptr};
		p_renderer_config = p_capture_ctrl_info->GetRendererConfig();
		if (nullptr == p_renderer_config)
		{
			VHAL_LOGE("parameter error. p_renderer_config=%p", p_renderer_config);
			result = VHAL_ERR_PARAM;
			break;
		}

		VhalCameraIviId	ivi_id{0,0};
		if (captureInputType::VHAL_CAPTURE_INPUT_HDMI == p_capture_ctrl_info->GetInputType())
		{
			/* HDMI用サーフェスID取得 */
			VHAL_LOGI("DeviceEntry[%s][%s]", VHAL_PATH_HDMI.c_str(), p_entry->inputName);

			int32_t ret{p_layout_mng->GetSurfaceIdFromVideoPath(VHAL_PATH_HDMI, VIDEO_OUTPUT_TARGET_FRONT, ivi_id.front)};
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("GetSurfaceIdFromVideoPath(%s) front error. ret=%d", VHAL_PATH_HDMI.c_str(), ret);
				result = VHAL_ERR_PARAM;
				break;
			}
			p_renderer_config->AddSurfaceId(ivi_id.front);

			ret = p_layout_mng->GetSurfaceIdFromVideoPath(VHAL_PATH_HDMI, VIDEO_OUTPUT_TARGET_REAR, ivi_id.rear);
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("GetSurfaceIdFromVideoPath(%s) rear error. ret=%d", VHAL_PATH_HDMI.c_str(), ret);
				/* 前席のサーフェスIDが取得出来ているなら処理継続 */
			}
			else
			{
				p_renderer_config->AddSurfaceId(ivi_id.rear);
			}
			VHAL_LOGI("Setting ivi_id front[%d], rear[%d]", ivi_id.front, ivi_id.rear);
		}
		else
		{
			/* カメラ用サーフェスID取得 */
			VHAL_LOGI("DeviceEntry[%s][%s]", VHAL_PATH_CAMERA.c_str(), p_entry->inputName);

			const int32_t ret{p_layout_mng->GetSurfaceIdFromVideoPath(VHAL_PATH_CAMERA, VIDEO_OUTPUT_TARGET_CAMERA, ivi_id.front)};
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("GetSurfaceIdFromVideoPath(%s) error. ret=%d", VHAL_PATH_CAMERA.c_str(), ret);
				result = VHAL_ERR_PARAM;
				break;
			}
			p_renderer_config->AddSurfaceId(ivi_id.front);
			VHAL_LOGI("Setting ivi_id front[%d]", ivi_id.front);
		}
	} while (false);

	VHAL_LOGV_OUT();

	return result;
}

/*****************************************************************************
 処理概要：	描画バッファ設定処理
 引数    ：	capturecontrol_info_t*	p_capture_ctrl_info	(i)キャプチャコントロール管理構造体ポインタ
           	QCarCamColorFmt_e		color_fmt			(i)カラーフォーマット
           	int32_t					buf_count			(i)バッファ数
           	uint32_t				width				(i)幅
           	uint32_t				height				(i)高さ
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM			パラメータ不正
           		VHAL_ERR_QCARCAM_API	QCarCamAPI不正
           		VHAL_SUCCESS			正常終了
 フェールセーフNo ：	F-VHAL-N-073
*****************************************************************************/
int32_t CVhalCaptureControl::SettingBuffer(capturecontrol_info_t * const p_capture_ctrl_info, const QCarCamColorFmt_e color_fmt, const int32_t buf_count, const uint32_t width, const uint32_t height)
{
	int32_t				result{VHAL_SUCCESS};
	QCarCamBufferList_t	buffers_output{};
	buffers_output.pBuffers = nullptr;

	VHAL_LOGV_IN();

	do
	{
		if (nullptr == p_capture_ctrl_info)
		{
			VHAL_LOGE("parameter error. p_capture_ctrl_info=%p", p_capture_ctrl_info);
			result = VHAL_ERR_PARAM;
			break;
		}

		QCarCamHndl_t	hndl{QCARCAM_HNDL_INVALID};
		hndl = p_capture_ctrl_info->GetHndl();
		if (QCARCAM_HNDL_INVALID == hndl)
		{
			VHAL_LOGE("parameter error. hndl=%lu", hndl);
			result = VHAL_ERR_PARAM;
			break;
		}

		wlrenderer::CWaylandRendererVideo *p_renderer_video{nullptr};
		/* 映像バッファ領域の取得 */
		p_renderer_video = p_renderer_->CreateRendererVideo(*p_capture_ctrl_info->GetRendererConfig());
		if (nullptr == p_renderer_video)
		{
			VHAL_LOGE("CreateRendererVideo() error. p_renderer_video=%p", p_renderer_video);
			result = VHAL_ERR_PARAM;
			break;
		}
		p_capture_ctrl_info->SetRendererVideo(p_renderer_video);

		/* バッファを設定 */
		buffers_output.colorFmt = color_fmt;
		buffers_output.nBuffers = I32ToUI32(buf_count);
		buffers_output.flags    = 0U;
		buffers_output.id       = kVHalBufferlistId;
		/* ※qcxclient.soのQCarCamSetBuffersでは内部でパラメータ保持する機能を持っていること。 */
		buffers_output.pBuffers = static_cast<QCarCamBuffer_t *>(calloc(static_cast<size_t>(buffers_output.nBuffers), sizeof(QCarCamBuffer_t)));
		if (nullptr == buffers_output.pBuffers)
		{
			VHAL_LOGE("buffers_output.pBuffers is null.");
			result = VHAL_ERR_PARAM;
			break;
		}

		/* バッファ管理用 */
		capturecontrol_bufinfo_t* const p_buf{static_cast<capturecontrol_bufinfo_t *>(calloc(static_cast<size_t>(buffers_output.nBuffers), sizeof(capturecontrol_bufinfo_t)))};
		if (nullptr == p_buf)
		{
			VHAL_LOGE("p_capture_ctrl_info->p_buf_info is null.");
		}
		p_capture_ctrl_info->SetBufInfo(p_buf);
		p_capture_ctrl_info->SetBufCnt(buffers_output.nBuffers);

		/* バッファ数分 */
		for (uint32_t i{0U}; i < buffers_output.nBuffers; ++i)
		{
			/* バッファの取得 */
			wlrenderer::CWaylandRendererVideoBuffer *p_renderer_buffer{nullptr};
			p_renderer_buffer = p_renderer_video->GetBuffer();
			if (nullptr == p_renderer_buffer)
			{
				VHAL_LOGE("p_renderer_video->GetBuffer() error.");
				result = VHAL_ERR_PARAM;
				break;
			}

			/* バッファプレーンの設定 */
			QCarCamBuffer_t* p_buffer{nullptr};
			p_buffer = &buffers_output.pBuffers[i];
			p_buffer->numPlanes = p_renderer_buffer->GetPlaneCount();	/* プレーン配列数 */
			for (uint32_t planeNo{0U}; planeNo < p_buffer->numPlanes; ++planeNo)
			{
				/* BEVstep3 24/10/17 */
				if (true == kVhalRendererShm)
				{
					p_buffer->planes[planeNo].memHndl = static_cast<uint64_t>(reinterpret_cast<uintptr_t>(p_renderer_buffer->GetPlaneAddr(planeNo)));
				}
				else
				{
					p_buffer->planes[planeNo].memHndl = reinterpret_cast<uint64_t>(static_cast<uintptr_t>(I32ToUI64(p_renderer_buffer->GetDmafd(planeNo))));
				}
				/* 複数プレーン時かつ各プレーンのサイズが異なる場合はwidth、heightを調整すること */
				p_buffer->planes[planeNo].width  = width;
				p_buffer->planes[planeNo].height = height;
				p_buffer->planes[planeNo].stride = I32ToUI32(p_renderer_buffer->GetStride(planeNo));
				p_buffer->planes[planeNo].size   = p_buffer->planes[planeNo].stride * p_buffer->planes[planeNo].height;

				uint8_t *p_addr{nullptr};
				p_addr = static_cast<uint8_t *>(p_renderer_buffer->GetPlaneAddr(planeNo));
				VHAL_LOGI("IDX[%u] p_addr[%p] FD[%d]", i, p_addr, p_renderer_buffer->GetDmafd(planeNo));
				/* バッファ内をクリア */
				(void)memset(p_addr, 0xff, static_cast<size_t>(p_buffer->planes[planeNo].size));
			}

			/* バッファ解放時用に関連付け */
			capturecontrol_bufinfo_t* const p_buf_info{&p_capture_ctrl_info->GetBufInfo()[i]};
			p_buf_info->frame_buf_indx           = i;
			p_buf_info->input_type               = p_capture_ctrl_info->GetInputType();
			p_buf_info->p_renderer_buffer        = p_renderer_buffer;
		}
		/* バッファ処理にてエラー */
		if (VHAL_SUCCESS != result)
		{
			break;
		}

		int32_t	ret{VHAL_SUCCESS};
		ret = SettingBlackBuffer(p_capture_ctrl_info, height);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGW("SettingBlackBuffer() error. ret=%d", ret);
		}

		/* バッファ設定 */
		QCarCamRet_e	qret{QCARCAM_RET_OK};
		qret = QCarCamSetBuffers(hndl, &buffers_output);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//		int32_t fail_ret{0};
//		bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-073",fail_ret)};
//		if(true == fail)
//		{
//			qret = static_cast<QCarCamRet_e>(fail_ret);
//		}
//#endif
		if (QCARCAM_RET_OK != qret)
		{
			VHAL_LOGE("QCarCamSetBuffers failed. ret = %d", qret);
			result = VHAL_ERR_QCARCAM_API;
		}
	} while (false);

	/* 後始末 */
	if (nullptr != buffers_output.pBuffers)
	{
		free(buffers_output.pBuffers);
		buffers_output.pBuffers = nullptr;
	}

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	QCarCamパラメータ処理(コールバック設定)
 引数    ：	QCarCamHndl_t	hndl	(i)ハンドル
 戻り値  ：	処理結果
           		VHAL_ERR_QCARCAM_API	QCarCamAPI不正
           		VHAL_SUCCESS			正常終了
 フェールセーフNo ：	F-VHAL-N-074
*****************************************************************************/
int32_t CVhalCaptureControl::SettingParamEventCallBack(const QCarCamHndl_t hndl)
{
	int32_t	result{VHAL_SUCCESS};

	VHAL_LOGV_IN();

	do
	{
		if (QCARCAM_HNDL_INVALID == hndl)
		{
			VHAL_LOGE("parameter error. hndl=%lu", hndl);
			result = VHAL_ERR_PARAM;
			break;
		}

		QCarCamRet_e			qret{QCARCAM_RET_OK};

		/* パラメータを設定(コールバック) */
		qret = QCarCamRegisterEventCallback(hndl, &CVhalCaptureEventCallBack, nullptr);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//		int32_t fail_ret{0};
//		bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-074",fail_ret)};
//		if(true == fail)
//		{
//			qret = static_cast<QCarCamRet_e>(fail_ret);
//		}
//#endif
		if (QCARCAM_RET_OK != qret)
		{
			VHAL_LOGE("QCarCamRegisterEventCallback failed. ret = %d", qret);
			result = VHAL_ERR_QCARCAM_API;
			break;
		}

		/* パラメータを設定(イベントmask設定) */
		uint32_t eventMask {static_cast<uint32_t>(QCARCAM_EVENT_FRAME_READY) |
			static_cast<uint32_t>(QCARCAM_EVENT_ERROR) | static_cast<uint32_t>(QCARCAM_EVENT_VENDOR)};
		qret = QCarCamSetParam(hndl, QCARCAM_STREAM_CONFIG_PARAM_EVENT_MASK, &eventMask, sizeof(eventMask));
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM	// F-VHAL-N-324
//		int32_t fail_ret2{static_cast<int32_t>(qret)};
//		CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-N-324",fail_ret2);
//		qret = static_cast<QCarCamRet_e>(fail_ret2);
//#endif
		if (QCARCAM_RET_OK != qret)
		{
			VHAL_LOGE("QCarCamSetParam(QCARCAM_PARAM_EVENT_MASK) failed. ret = %d", qret);
			result = VHAL_ERR_QCARCAM_API;
			break;
		}

		/* パラメータを設定 (opMode = QCARCAM_OPMODE_ISP) */
		QCarCamIspUsecaseConfig_t ispConfig{};
		constexpr QCarCamIspUsecase_e kUsecaseId{static_cast<QCarCamIspUsecase_e>(16)};
		ispConfig.usecaseId = kUsecaseId;
		qret = QCarCamSetParam(hndl, QCARCAM_STREAM_CONFIG_PARAM_ISP_USECASE, &ispConfig, sizeof(ispConfig));
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM	// F-VHAL-N-336
//		int32_t fail_ret3{static_cast<int32_t>(qret)};
//		CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-N-336",fail_ret3);
//		qret = static_cast<QCarCamRet_e>(fail_ret3);
//#endif
		if (QCARCAM_RET_OK != qret)
		{
			VHAL_LOGE("QCarCamSetParam(QCARCAM_STREAM_CONFIG_PARAM_ISP_USECASE) failed. ret = %d", qret);
			result = VHAL_ERR_QCARCAM_API;
			break;
		}
	} while (false);

	VHAL_LOGV_OUT();

	return result;
}

/*****************************************************************************
 処理概要：	QCarCamフレームリクエスト送信
 引数    ：	QCarCamHndl_t	hndl		(i)ハンドル
           	uint32_t		request_id	(i)リクエストID
           	uint32_t		buf_index	(i)バッファインデックス
           	QCarCamRet_e&	qret		(o)qcarcam実施結果
 戻り値  ：	無し
 注意    : 	実施はQCarCamStart()以降であること
 フェールセーフNo ：	F-VHAL-N-345
*****************************************************************************/
void CVhalCaptureControl::SettingSubmitRequest(const QCarCamHndl_t hndl, const uint32_t request_id, const uint32_t buf_index, QCarCamRet_e& qret) const noexcept
{
//	VHAL_LOGV_IN();	/* 呼び出し回数が多い為コメント化 */

	/* 次キャプチャを行うための指示 */
	QCarCamRequest_t	request{};
	request.requestId = request_id;
	request.numStreamRequests = 1U;
	request.streamRequests[0].bufferlistId = kVHalBufferlistId;
	request.streamRequests[0].bufferIdx    = buf_index;
	qret = QCarCamSubmitRequest(hndl, &request);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	int32_t fail_ret{static_cast<int32_t>(qret)};
//	CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-N-999",fail_ret);
//	qret = static_cast<QCarCamRet_e>(fail_ret);
//#endif
	VHAL_LOGV("QCarCamSubmitRequest qret=%d idx=%u", qret, buf_index);
	if (QCARCAM_RET_OK != qret)
	{
		VHAL_LOGW("QCarCamSubmitRequest failed. ret = %d bufidx = %u", qret, buf_index);
	}

//	VHAL_LOGV_OUT();	/* 呼び出し回数が多い為コメント化 */
}

/*****************************************************************************
 処理概要：	QCarCamフレームリクエスト送信準備
 引数    ：	QCarCamHndl_t	hndl		(i)ハンドル
           	uint32_t		buf_index	(i)バッファインデックス
           	QCarCamRet_e&	qret		(o)qcarcam実施結果
 戻り値  ：	無し
 注意    : 	実施はQCarCamStart()以降であること
*****************************************************************************/
void CVhalCaptureControl::SettingSubmitRequestPrev(const QCarCamHndl_t hndl, const uint32_t buf_index, QCarCamRet_e& qret)
{
	const auto it_info = std::find_if(p_capture_ctrl_infos_.begin(), p_capture_ctrl_infos_.end(),
		[hndl](const std::unique_ptr<capturecontrol_info_t> &p_capture_ctrl_info) noexcept -> bool { return p_capture_ctrl_info->GetHndl() == hndl; });
	if (it_info == p_capture_ctrl_infos_.end())
	{
		VHAL_LOGW("capture_ctrl_info is not found.");
		qret = QCARCAM_RET_BADPARAM;
	}
	else
	{
		/* it_info->get()は必ず実体があるためnull調査しない (it_info->get()がnullならば、上記のstd::find_ifで正常応答しないはず) */
		capturecontrol_info_t* const p_capture_ctrl_info{it_info->get()};
		p_capture_ctrl_info->LockMtxQcarcam();
		uint32_t	request_id{p_capture_ctrl_info->GetSubmitRequestId()};
		SettingSubmitRequest(hndl, request_id, buf_index, qret);
		p_capture_ctrl_info->UnLockMtxQcarcam();
	}
}

/*****************************************************************************
 処理概要：	QCarCamパラメータ処理(HDCP認証キー書込み)
 引数    ：	CVhalHdcpAuthRsltData&	hdcpkey_cdisp	HDCP認証キー(C-Disp)
           	CVhalHdcpAuthRsltData&	hdcpkey_rse		HDCP認証キー(RSE)
 戻り値  ：	処理結果
           		VHAL_ERR_QCARCAM_API	QCarCamAPI不正
           		VHAL_ERR				異常終了
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalCaptureControl::SetHdcpAuthKey(const CVhalHdcpAuthRsltData& hdcpkey_cdisp, const CVhalHdcpAuthRsltData& hdcpkey_rse)
{
	int32_t						result{VHAL_SUCCESS};

	VHAL_LOGV_IN();
	do
	{
		int32_t					ret{VHAL_SUCCESS};
		QCarCamRet_e			qret{QCARCAM_RET_OK};

		/* QCarCam初期化 */
		ret = InitializeQcarcam();
		if(VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("InitializeQcarcam() failed. ret=%d", ret);
			result = ret;
			break;
		}

		/* HDMIデバイスの検索 */
		const auto				itr_map = device_entries_.find(captureInputType::VHAL_CAPTURE_INPUT_HDMI);
		if (itr_map == device_entries_.end())
		{
			VHAL_LOGE("HDMI device is not supported.");
			result = VHAL_ERR;
			break;
		}

		/* QCarCamクエリ情報取得 */
		QCarCamInput_t			query_input{};
		QCarCamInputSrc_t		inputSource{};
		uint32_t				uiModeIndex{0U};
		ret = SettingQueryInput(itr_map->second.inputName, &query_input, inputSource, uiModeIndex, 0U, 0U);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("SettingQueryInput() failed. ret=%d", ret);
			result = ret;
			break;
		}

		/* QCarCamオープン準備 */
		QCarCamHndl_t			hndl{QCARCAM_HNDL_INVALID};
		QCarCamOpen_t			openParams{};
		openParams.opMode              = QCARCAM_OPMODE_ISP;		// (=1)
		openParams.numInputs           = 1U;
		openParams.inputs[0].inputId   = query_input.inputId;
		openParams.inputs[0].srcId     = inputSource.srcId;
		openParams.inputs[0].inputMode = uiModeIndex;

		/* QCarCamOpenオープン (ループ最大回数=6回(初回＋リトライ5回)) */
		constexpr uint32_t		kVhalQcarcamOpenLoopMax{5U};		/* リトライ5回 */
		bool					loop_end{false};
		for (uint32_t count{0U}; !loop_end; ++count)
		{
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//			int32_t fail_ret_open{static_cast<int32_t>(QCARCAM_RET_OK)};
//			CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-N-339",fail_ret_open);
//			CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-R-340",fail_ret_open, count);
//			qret = static_cast<QCarCamRet_e>(fail_ret_open);
//			if (QCARCAM_RET_OK == qret)
//#endif
			qret = QCarCamOpen(&openParams, &hndl);
			/* リトライ系 */
			if ((QCARCAM_RET_FAILED == qret)	||			/* 処理失敗 */
				(QCARCAM_RET_BADSTATE == qret)	||			/* 無効な状態 */
				(QCARCAM_RET_TIMEOUT == qret)	||			/* 処理タイムアウト */
				(QCARCAM_RET_NOMEM == qret)		||			/* メモリ不足 */
				(QCARCAM_RET_BUSY == qret)		||			/* 処理中 */
				(QCARCAM_RET_NOT_FOUND == qret)	||			/* サーバとの通信未確立 */
				(QCARCAM_RET_SENSOR_NOT_FOUND == qret))		/* センサーが無い */
			{
				if (kVhalQcarcamOpenLoopMax <= count)
				{
					/* リトライオーバー */
					VHAL_LOGE("QCarCamOpen retry over qret=%d count=%u", qret, count);
					result = VHAL_ERR_QCARCAM_API;
					loop_end = true;
				}
				else
				{
					/* リトライ中 (wait200ms) */
					sif_mdelay(kVhalQcarcamRetryWait);
				}
			}
			/* 非リトライ系 */
			else
			{
				if (QCARCAM_RET_OK != qret)
				{
					VHAL_LOGE("QCarCamOpen failed. qret=%d", qret);
					result = VHAL_ERR_QCARCAM_API;
				}
				loop_end = true;
			}
		}
		if (QCARCAM_RET_OK != qret)
		{
			break;
		}

		CVhalHdcpAuthRsltData	hdcp_cdisp;
		CVhalHdcpAuthRsltData	hdcp_rse;
		if (VHAL_HDCP_FIRST_AUTH_STS_SUCCESS == hdcpkey_cdisp.GetResult())
		{
			hdcp_cdisp = hdcpkey_cdisp;
		}
		else
		{
			/* HDCP認証キー(C-Disp)が成功以外の場合は、max_devs_exceededなど各値を0とする */
			hdcp_cdisp.Clear();
		}
		if (VHAL_HDCP_FIRST_AUTH_STS_SUCCESS == hdcpkey_rse.GetResult())
		{
			hdcp_rse = hdcpkey_rse;
		}
		else
		{
			/* HDCP認証キー(RSE)が成功以外の場合は、max_devs_exceededなど各値を0とする */
			hdcp_rse.Clear();
		}

		/* HDCP認証キー書込み */
		/* 固定値の定義 */
		constexpr uint32_t		kKsvSerDevNum{HDMI_SER_DEV_NUM};
		constexpr std::size_t	kKsvLength{HDMI_KSV_LENGTH};
		constexpr uint8_t		kMaxCountCdisp{1U};
		constexpr uint8_t		kMaxCountRse{3U};
		constexpr uint8_t		kAddDepth{1U};
		/* ローカル変数定義 */
		uint8_t					dev_count_cdisp{std::min(ToUI8(hdcp_cdisp.SizeReceiverIds()), kMaxCountCdisp)};
		uint8_t					dev_count_rse{std::min(ToUI8(hdcp_rse.SizeReceiverIds()), kMaxCountRse)};
		vendor_ext_property_t	param{};
		param.type = VENDOR_EXT_PROP_KSV_LIST;
		VenderHdmiKeyList_t&	keylist{param.value.keyList};

		/* VenderHdmiKeyList_tの各値設定 */
		keylist.max_devs_exceeded    = hdcp_cdisp.GetMaxDevs() | hdcp_rse.GetMaxDevs();
		keylist.device_count         = dev_count_cdisp + dev_count_rse;
		keylist.max_cascade_exceeded = hdcp_cdisp.GetMaxCascade() | hdcp_rse.GetMaxCascade();
		keylist.depth                = std::max(hdcp_cdisp.GetCascadeDepth(), hdcp_rse.GetCascadeDepth()) + kAddDepth;

		/* C-Dispのレシーバー設定 */
		uint32_t				pos{0U};
		std::vector<uint8_t>	o_receiver_id{};
		if (VHAL_HDCP_FIRST_AUTH_STS_SUCCESS == hdcp_cdisp.GetResult())
		{
			if (0U < dev_count_cdisp)
			{
				hdcp_cdisp.ConvReceiverIds(hdcp_cdisp.GetReceiverIds()[0], o_receiver_id);
				if (kKsvLength <= o_receiver_id.size())		/* 5バイト以上 */
				{
					memcpy(keylist.receiver_id[pos], o_receiver_id.data(), kKsvLength);
				}
			}
			++pos;
		}
		/* RSEのレシーバー設定 */
		if (VHAL_HDCP_FIRST_AUTH_STS_SUCCESS == hdcp_rse.GetResult())
		{
			for (uint8_t i{0U}; i < dev_count_rse; ++i)
			{
				hdcp_rse.ConvReceiverIds(hdcp_rse.GetReceiverIds()[i], o_receiver_id);
				if ((kKsvLength <= o_receiver_id.size()) && (kKsvSerDevNum > pos))
				{
					memcpy(keylist.receiver_id[pos], o_receiver_id.data(), kKsvLength);
					++pos;
				}
			}
		}

		if (dev_count_cdisp < hdcp_cdisp.SizeReceiverIds())
		{
			VHAL_LOGI("DeviceCount(C-Disp:%u) more than %u.", hdcp_cdisp.SizeReceiverIds(), kMaxCountCdisp);
		}
		if (dev_count_rse < hdcp_rse.SizeReceiverIds())
		{
			VHAL_LOGI("DeviceCount(RSE:%u) more than %u.", hdcp_rse.SizeReceiverIds(), kMaxCountRse);
		}
		VHAL_LOGI("VenderHdmiKeyList_t max_devs_exceeded=%u device_count=%u max_cascade_exceeded=%u depth=%u", \
			keylist.max_devs_exceeded, keylist.device_count, keylist.max_cascade_exceeded, keylist.depth);
		const auto&					rid = keylist.receiver_id;
		VHAL_LOGI("receiver_id [0]=%02X-%02X-%02X-%02X-%02X [1]=%02X-%02X-%02X-%02X-%02X " \
							  "[2]=%02X-%02X-%02X-%02X-%02X [3]=%02X-%02X-%02X-%02X-%02X", \
			rid[0][0], rid[0][1], rid[0][2], rid[0][3], rid[0][4], rid[1][0], rid[1][1], rid[1][2], rid[1][3], rid[1][4], \
			rid[2][0], rid[2][1], rid[2][2], rid[2][3], rid[2][4], rid[3][0], rid[3][1], rid[3][2], rid[3][3], rid[3][4]);

//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//		int32_t fail_ret_sprm{static_cast<int32_t>(QCARCAM_RET_OK)};
//		CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-N-186",fail_ret_sprm);
//		qret = static_cast<QCarCamRet_e>(fail_ret_sprm);
//		if (QCARCAM_RET_OK == qret)
//#endif
		qret = QCarCamSetParam(hndl, QCARCAM_VENDOR_PARAM, &param, sizeof(param));
		if (QCARCAM_RET_OK == qret)
		{
			sethdcpauthkey_ = true;
		}
		else
		{
			VHAL_LOGW("QCarCamSetParam(QCARCAM_VENDOR_PARAM) failed. ret=%d", qret);
			result = VHAL_ERR_QCARCAM_API;
		}

		/* QCarCamOpenクローズ */
		qret = QCarCamClose(hndl);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//		int32_t fail_ret_close{static_cast<int32_t>(qret)};
//		CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-C-341",fail_ret_close);
//		qret = static_cast<QCarCamRet_e>(fail_ret_close);
//#endif
		if (QCARCAM_RET_OK != qret)
		{
			VHAL_LOGE("QCarCamClose failed. qret=%d", qret);
			/* result = VHAL_ERR_QCARCAM_API;   キー書込み成功後はQCarCamCloseの結果に関わらず戻り値を正常とする */
			break;
		}
	} while(false);

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	HDCP認証キー書込み済フラグ取得
 引数    ：	なし
 戻り値  ：	HDCP認証書込み済フラグ(true=済 / false=未)
*****************************************************************************/
bool CVhalCaptureControl::GetHdcpAuthKeyWriteStatus(void) const noexcept
{
	return sethdcpauthkey_;
}

/*****************************************************************************
 処理概要：	HDCP認証キー書込み済フラグクリア
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalCaptureControl::ClearHdcpAuthKeyWriteStatus(void) noexcept
{
	sethdcpauthkey_ = false;
}

/* 24MMマージで誤らないためのコメント：BEVstep3ではSettingParamHdcpAuthResultRseを削除 */

/*****************************************************************************
 処理概要：	黒画設定処理
 引数    ：	capturecontrol_info_t*	p_capture_ctrl_info	(i)キャプチャコントロール管理構造体ポインタ
           	const uint32_t				height				(i)高さ
 戻り値  ：	処理結果
           		VHAL_ERR_****				エラー
           		VHAL_SUCCESS				正常終了
*****************************************************************************/
int32_t CVhalCaptureControl::SettingBlackBuffer(capturecontrol_info_t * const p_capture_ctrl_info, const uint32_t height) const
{
	int32_t	result{VHAL_SUCCESS};

	VHAL_LOGV_IN();

	do
	{
		if (nullptr == p_capture_ctrl_info)
		{
			VHAL_LOGE("parameter error. p_capture_ctrl_info=%p", p_capture_ctrl_info);
			result = VHAL_ERR_PARAM;
			break;
		}

		/* 黒描画用のバッファを用意する */
		if (nullptr == p_capture_ctrl_info->GetRendererVideo())
		{
			VHAL_LOGE("parameter error. p_renderer_video=%p", p_capture_ctrl_info->GetRendererVideo());
			result = VHAL_ERR_PARAM;
			break;
		}
		p_capture_ctrl_info->SetBlackBuffer(p_capture_ctrl_info->GetRendererVideo()->GetBuffer());
		if (nullptr == p_capture_ctrl_info->GetBlackBuffer())
		{
			VHAL_LOGE("p_capture_ctrl_info->GetBlackBuffer() erorr.");
			result = VHAL_ERR_PARAM;
			break;
		}
		uint32_t			planeCnt{0U};
		planeCnt = p_capture_ctrl_info->GetBlackBuffer()->GetPlaneCount();	/* プレーン配列数 */
		for (uint32_t planeNo{0U}; planeNo < planeCnt; ++planeNo)
		{
			uint32_t	planeSize{0U};
			uint32_t*	p_addr{nullptr};
			planeSize = I32ToUI32(p_capture_ctrl_info->GetBlackBuffer()->GetStride(planeNo)) * height;
			p_addr = static_cast<uint32_t*>(p_capture_ctrl_info->GetBlackBuffer()->GetPlaneAddr(planeNo));
			/* バッファ内を黒設定 */
			constexpr uint32_t	kVhalUyvBlack{0x10801080U};	/* サーフェス黒画塗りつぶし用 */
			for (size_t i{planeSize / sizeof(kVhalUyvBlack)}; i > 0U; i--)
			{
				*p_addr = kVhalUyvBlack;	/* black in uyvy */
				p_addr++;
			}
		}
	} while (false);

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	黒画描画処理
 引数    ：	const capturecontrol_info_t*	p_capture_ctrl_info	(i)キャプチャコントロール管理構造体ポインタ
 戻り値  ：	処理結果
           		VHAL_ERR_****				エラー
           		VHAL_SUCCESS				正常終了
*****************************************************************************/
int32_t CVhalCaptureControl::SendFrameBlackBuffer(const capturecontrol_info_t * const p_capture_ctrl_info) const
{
	int32_t	result{VHAL_SUCCESS};

	VHAL_LOGV_IN();

	do
	{
		if (nullptr == p_capture_ctrl_info)
		{
			VHAL_LOGE("parameter error. p_capture_ctrl_info=%p", p_capture_ctrl_info);
			result = VHAL_ERR_PARAM;
			break;
		}

		/* 黒描画用のバッファを送信する */
		if ((nullptr == p_capture_ctrl_info->GetRendererVideo()) || (nullptr == p_capture_ctrl_info->GetRendererConfig()))
		{
			VHAL_LOGE("parameter error. p_renderer_video=%p, p_renderer_config=%p", p_capture_ctrl_info->GetRendererVideo(), p_capture_ctrl_info->GetRendererConfig());
			result = VHAL_ERR_PARAM;
			break;
		}
		int32_t					ret{WL_RENDERER_SUCCESS};
		std::vector<int32_t>	surface_ids{0};
		p_capture_ctrl_info->GetRendererConfig()->GetSurfaceIds(surface_ids);
		ret = p_capture_ctrl_info->GetRendererVideo()->SendBuffer(p_capture_ctrl_info->GetBlackBuffer(), surface_ids);
		if (WL_RENDERER_SUCCESS != ret)
		{
			VHAL_LOGE("SendBuffer[black_buffer] failed. ret = %d", ret);
			result = VHAL_ERR_WAYLAND;
		}
	} while (false);

	VHAL_LOGV_OUT();
	return result;
}

/* 24MMマージで誤らないためのコメント：BEVstep3ではCheckGetBufferErrorを削除 */
/* 24MMマージで誤らないためのコメント：BEVstep3ではCheckReleaseBufferErrorを削除 */




/*****************************************************************************
 処理概要：	HDMI-Rxデバイス異常判定
 引数    ：	QCarCamHndl_t 		hndl		(i)ハンドル
 戻り値  ：	なし
*****************************************************************************/
void CVhalCaptureControl::CheckHdmiReceiverError(const QCarCamHndl_t hndl)
{
	const auto it_info = std::find_if(p_capture_ctrl_infos_.begin(), p_capture_ctrl_infos_.end(),
		[hndl](const std::unique_ptr<capturecontrol_info_t> &p_capture_ctrl_info) noexcept -> bool { return p_capture_ctrl_info->GetHndl() == hndl; });
	if( it_info == p_capture_ctrl_infos_.end() )
	{
		VHAL_LOGW("capture_ctrl_info is not found.");
	}
	else
	{
		if( captureInputType::VHAL_CAPTURE_INPUT_HDMI == (*it_info)->GetInputType() )
		{
			if( false == (*it_info)->GetCaptureResetEvent() )
			{
				/* キャプチャリセットイベント通知 */
				(*it_info)->SetCaptureResetEvent(true);
				NotifyCaptureResetEvent((*it_info)->GetInputType(), CaptureResetFactor::kHdmiReceiverError);
			}
		}
	}
}

/*****************************************************************************
 処理概要：	キャプチャ割り込み監視開始
 引数    ：	capturecontrol_info_t*	p_capture_ctrl_info	(i)キャプチャコントロール管理構造体ポインタ
 戻り値  ：	なし
 フェールセーフNo：	F-VHAL-R-079
*****************************************************************************/
void CVhalCaptureControl::StartInterruptObserver(capturecontrol_info_t* const p_capture_ctrl_info)
{
	p_capture_ctrl_info->AllocateObserver();
	p_capture_ctrl_info->GetObserver()->Initialize(this, p_capture_ctrl_info->GetInputType());

	const bool is_running{p_observer_control_->IsRunningObserver(p_capture_ctrl_info->GetObserver())};
	if (false == is_running)
	{
		/* 監視登録(異常により監視登録が解除されている場合は再登録) */
		constexpr int64_t kReceiveCaptureTimeout{2000};		/* キャプチャ取得タイムアウト時間(ms) */
		const int32_t ret{p_observer_control_->RegistryObserver(p_capture_ctrl_info->GetObserver(), kReceiveCaptureTimeout)};
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("CVhalObserverControl::RegistryObserver() error. ret=%d", ret);
		}
	}

}

/*****************************************************************************
 処理概要：	キャプチャ割り込み監視停止
 引数    ：	capturecontrol_info_t*	p_capture_ctrl_info	(i)キャプチャコントロール管理構造体ポインタ
 戻り値  ：	なし
 フェールセーフNo：	F-VHAL-R-079
*****************************************************************************/
void CVhalCaptureControl::StopInterruptObserver(capturecontrol_info_t* const p_capture_ctrl_info)
{
	if (nullptr != p_observer_control_)
	{
		const bool is_running{p_observer_control_->IsRunningObserver(p_capture_ctrl_info->GetObserver())};
		if (true == is_running)
		{
			/* 監視削除 */
			p_observer_control_->ClearObserver(p_capture_ctrl_info->GetObserver());
			p_capture_ctrl_info->GetObserver()->Finalize();
			p_capture_ctrl_info->FreeObserver();
		}
	}
}

/*****************************************************************************
 処理概要：	キャプチャ割り込み発生通知
 引数    ：	qcarcam_hndl_t 	hndl	(i)ハンドル
 戻り値  ：	なし
 フェールセーフNo：	F-VHAL-R-079
*****************************************************************************/
void CVhalCaptureControl::NotifyInterruptHeartbeat(const QCarCamHndl_t hndl)
{
	const auto it_info = std::find_if(p_capture_ctrl_infos_.begin(), p_capture_ctrl_infos_.end(),
		[hndl](const std::unique_ptr<capturecontrol_info_t> &p_capture_ctrl_info) noexcept -> bool { return p_capture_ctrl_info->GetHndl() == hndl; });
	if (it_info == p_capture_ctrl_infos_.end())
	{
		VHAL_LOGW("capture_ctrl_info is not found.");
	}
	else
	{
		(*it_info)->LockMtxHeartBeat();
		(*it_info)->SetHeartBeat(true);
		(*it_info)->UnLockMtxHeartBeat();
	}
}

/*****************************************************************************
 処理概要：	キャプチャ割り込み発生有無確認
 引数    ：	captureInputType	input_type			(i)デバイス種別
 戻り値  ：	false:キャプチャリセット発生なし true:キャプチャリセット発生あり
 フェールセーフNo：	F-VHAL-R-079
*****************************************************************************/
bool CVhalCaptureControl::CheckInterruptHeartbeat(const captureInputType input_type)
{
	bool reset{false};

	const auto it_info = std::find_if(p_capture_ctrl_infos_.begin(), p_capture_ctrl_infos_.end(),
		[input_type](const std::unique_ptr<capturecontrol_info_t> &p_capture_ctrl_info) noexcept -> bool { return p_capture_ctrl_info->GetInputType() == input_type; });
	if (it_info == p_capture_ctrl_infos_.end())
	{
		VHAL_LOGW("capture_ctrl_info is not found.");
	}
	else
	{
		(*it_info)->LockMtxHeartBeat();
		if( true == (*it_info)->GetHeartBeat() )
		{
			(*it_info)->SetHeartBeat(false);
		}
		else
		{
			if( false == (*it_info)->GetCaptureResetEvent() )
			{
				/* キャプチャリセットイベント通知 */
				(*it_info)->SetCaptureResetEvent(true);
				NotifyCaptureResetEvent((*it_info)->GetInputType(), CaptureResetFactor::kInterruptNothing);
			}
			/* StopInterruptObserverはコール不要                                                        */
			/* CVhalCaptureInterruptObserver::Notifyの戻り値を負の値にすることで、監視を終了させる      */
			/* (CVhalObserverControl::ClearObserverと同等)                                              */
			/* CVhalObserverControlからのコール内で、CVhalObserverControl::ClearObserverをコールしない  */
			(*it_info)->GetObserver()->Finalize();

			reset = true;
		}
		(*it_info)->UnLockMtxHeartBeat();
	}
	return reset;
}

/*****************************************************************************
 処理概要：	キャプチャ設定(遅延処理)
 引数    ：	const captureInputType	input_type	(i)デバイス種別
           	const uint32_t			interval	(i)タイマ周期
           	const uint32_t			cycle_count	(i)繰り返し回数
 戻り値  ：	処理結果
           		VHAL_ERR_****			エラー
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalCaptureControl::OpenCaptureDelayProcess(const captureInputType input_type, const uint32_t interval, const uint32_t cycle_count)
{
	int32_t result{VHAL_SUCCESS};

	const COpenCaptureTimerList::const_iterator	it{open_capture_timer_list_.find(input_type)};
	/* 既にopen_capture_timer_list_に入っている場合 */
	if (it != open_capture_timer_list_.end())
	{
		CVhalOpenCaptureTimer* const p_timer{it->second.get()};
	    /* 既存タイマを停止・解放し、新しいパラメータで再初期化して起動 */
		p_timer->EndTimer();
		p_timer->Finalize();
		int32_t ret{p_timer->Initialize(p_route_.get(), interval, cycle_count)};
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("Initialize error. ret=%d interval=%u cycle_count=%u", ret, interval, cycle_count);
			result = ret;
		}
		else
		{
			ret = p_timer->StartTimer();
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("StartTimer error. ret=%d", ret);
				result = ret;
			}
			else
			{
				VHAL_LOGI("OpenCaptureTimer Start[Input_type:%d][Interval:%ums][Cycle:%u]", input_type, interval, cycle_count);
			}
		}
	}
	else
	{
		if (nullptr == p_captureRecv_listener_)
		{
			VHAL_LOGE("parameter error. p_captureRecv_listener_=%p", p_captureRecv_listener_);
			result = VHAL_ERR_PARAM;
		}
		else
		{
			/* タイマ設定 */
			std::unique_ptr<CVhalOpenCaptureTimer>	p_timer{std::make_unique<CVhalOpenCaptureTimer>(p_captureRecv_listener_, input_type)};
			if (nullptr == p_timer)
			{
				VHAL_LOGE("error. p_timer=%p", p_timer.get());
				result = VHAL_ERR_TIMER;
			}
			else
			{
				int32_t ret{p_timer->Initialize(p_route_.get(), interval, cycle_count)};
				if (VHAL_SUCCESS != ret)
				{
					VHAL_LOGE("Initialize error. ret=%d", ret);
					result = ret;
				}
				else
				{
					/* タイマ起動 */
					ret = p_timer->StartTimer();
					if (VHAL_SUCCESS != ret)
					{
						VHAL_LOGE("StartTimer error. ret=%d", ret);
						result = ret;
					}
					else
					{
						(void)open_capture_timer_list_.insert(std::make_pair(input_type, std::move(p_timer)));
						VHAL_LOGI("OpenCaptureTimer Start[Input_type:%d][Interval:%ums]", input_type, interval);
					}
				}
			}
		}
	}

	return result;
}
/*****************************************************************************
 処理概要：	キャプチャリセットイベント通知
 引数    ：	captureInputType	input_type			(i)デバイス種別
           	CaptureResetFactor 	factor				(i)リセット要因
 戻り値  ：	なし
*****************************************************************************/
void CVhalCaptureControl::NotifyCaptureResetEvent(const captureInputType input_type, const CaptureResetFactor factor)
{
	VHAL_LOGW("capture system reset input=%d factor=%d", input_type, factor);

	/* キャプチャリセットイベント生成＆通知 */
	std::unique_ptr<CVhalEventItemCaptureReset> p_capture_init_event{std::make_unique<videohal::CVhalEventItemCaptureReset>()};
	if (nullptr != p_capture_init_event)
	{
		p_capture_init_event->SetName(std::string("capture reset"));
		p_capture_init_event->SetData(p_prop_, input_type);
		const int32_t ret{p_route_->WriteEvent(p_capture_init_event.get())};
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGEW("WriteEvent() error. ret=%d", ret);
		}
		else
		{
			(void)p_capture_init_event.release();
		}
	}
	else
	{
		VHAL_LOGE("NotifyCaptureResetEvent error.");
	}
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalCaptureInterruptObserver::~CVhalCaptureInterruptObserver(void)
{
	Finalize();
}

/*****************************************************************************
 処理概要：	初期化
 引数    ：	CVhalCaptureControl*	p_cap_control	(i)キャプチャ制御インスタンスポインタ
           	captureInputType		cap_input_type	(i)デバイス種別
 戻り値  ：	なし
*****************************************************************************/
void CVhalCaptureInterruptObserver::Initialize(CVhalCaptureControl* const p_cap_control, const captureInputType cap_input_type) noexcept
{
	p_capctrl_  = p_cap_control;
	input_type_ = cap_input_type;
}

/*****************************************************************************
 処理概要：	終了
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalCaptureInterruptObserver::Finalize(void) noexcept
{
	p_capctrl_  = nullptr;
	input_type_ = captureInputType::VHAL_CAPTURE_INPUT_MAX;
}

/*****************************************************************************
 処理概要：	キャプチャ割り込み監視周期コールバック
 引数    ：	なし
 戻り値  ：	処理結果
				VHAL_SUCCESS			正常終了
 フェールセーフNo：	F-VHAL-R-079
*****************************************************************************/
int32_t CVhalCaptureInterruptObserver::Notify(void)
{
	/* キャプチャ割り込み発生有無確認 */
	bool reset{false};
	if (nullptr != p_capctrl_)
	{
		reset = p_capctrl_->CheckInterruptHeartbeat(input_type_);
	}

	int32_t ret{VHAL_SUCCESS};
	if (true == reset)
	{
		/* キャプチャリセット発生した場合、割り込み監視を即終了するため、負の値を返す */
		ret = -1;
	}
	return ret;
}

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	CVhalCaptureReceiveEventListenerBase* const p_captureRecv_listener	(i)VhalCaptureReceiveイベントリスナインスタンスポインタ
           	const captureInputType	input_type		(i)デバイス種別
 戻り値  ：	なし
*****************************************************************************/
CVhalOpenCaptureTimer::CVhalOpenCaptureTimer(CVhalCaptureReceiveEventListenerBase* const p_captureRecv_listener, const captureInputType input_type) noexcept
	:CVhalTimer()
	,p_captureRecv_listener_(p_captureRecv_listener)
	,input_type_(input_type)
{
}

/*****************************************************************************
 処理概要：	タイマコールバックイベント
 引数    ：	const bool time_cycle_enable	(i)サイクリックタイマ有効状態
           		true	有効（タイマ回数制限なし or タイマ回数制限未到達）
           		false	無効（タイマ回数制限到達）
 戻り値  ：	処理結果
           		VHAL_SUCCESS			タイマ継続
           		VHAL_ERR				タイマ終了
*****************************************************************************/
int32_t CVhalOpenCaptureTimer::OnTimerImpl(const bool time_cycle_enable) const
{
	int32_t result{VHAL_SUCCESS};
	if (nullptr != p_captureRecv_listener_)
	{
		if (captureInputType::VHAL_CAPTURE_INPUT_CAMERA == input_type_)
		{
			bool movie_check{true};
			if (false == time_cycle_enable)
			{
				/* タイマ回数制限到達時は必ずカメラキャプチャ設定を実施 */
				VHAL_LOGD("OpenCaptureTimer time_cycle_enable:%d[Input_type:%d]", time_cycle_enable, input_type_);
				movie_check = false;
			}
			bool is_capture{false};
			const int32_t ret{p_captureRecv_listener_->NotifyReceiveOpenCameraCapture(movie_check, is_capture)};
			if ((true == is_capture) || (VHAL_SUCCESS != ret))
			{
				/* キャプチャ設定済もしくはキャプチャ処理に失敗した場合は次回タイマ起動は行わない */
				VHAL_LOGD("OpenCaptureTimer Stop[Input_type:%d]", input_type_);
				result = VHAL_ERR;
			}
		}
	}
	return result;
}

} /* namespace videohal */

