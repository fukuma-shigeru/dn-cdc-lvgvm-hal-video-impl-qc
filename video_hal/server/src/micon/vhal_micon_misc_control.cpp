/*******************************************************************************
    機能名称    ：  マイコン間MISC_通信制御モジュール
    ファイル名称：  vhal_micon_misc_control.cpp
*******************************************************************************/
#include "vhal_define.h"
#include "vhal_log.h"
#include "vhal_round_cast.h"
#include "vhal_main_control.h"
#include "vhal_event_route.h"
#include "vhal_sysdb_control.h"

#include "vhal_micon_comm_control.h"
#include "vhal_micon_receive_item.h"
#include "vhal_debug_system.h"
#include "sys_db_cnv_value_public.h"

extern "C"
{
#include "ccm_public.h"
#include "misc_ctrl_api_public.h"
}

namespace videohal
{

namespace 
{
CVhalMiconCommMiscControl *p_misc_control_{nullptr};
}

/*****************************************************************************
 処理概要：	イベント処理の実施
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
 注意    ： Execは、VideoHALメインイベントシステムからコールされる
*****************************************************************************/
int32_t CVhalMiscEventItem::Exec(void) const
{
	/* イベントを受信したときの処理 */
	constexpr std::size_t kMinDataSize{2U};
	if (kMinDataSize <= data_.size())
	{
		VHAL_LOGD("CVhalMiscEventItem::Exec called. opc=%02Xh-%02Xh",data_.back(),data_[0]);
	}
	else
	{
		VHAL_LOGD("CVhalMiscEventItem::Exec called.");
	}

	if (nullptr != p_misc_control_)
	{
		/* 受信データを渡す */
		p_misc_control_->ExecReceiveEvent(data_);
	}
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	データ取得
 引数    ：	uint32_t		dataSize	(i)受信データサイズ
           	void*			data		(i)受信データ
           	uint8_t			data_type	(i)データタイプ
 戻り値  ：	なし
*****************************************************************************/
void CVhalMiscEventItem::SetData(const uint32_t dataSize, const void* const data, const uint8_t data_type)
{
	/* 受信データを格納 */
	data_.resize(static_cast<std::size_t>(dataSize + 1U));
	(void)memcpy(data_.data(), data, static_cast<size_t>(dataSize));
	data_[dataSize] = data_type;
}


/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalMiconCommMiscControl::CVhalMiconCommMiscControl(void)
{
}


/*****************************************************************************
 処理概要：	初期化処理
 引数    ：	CVhalEventRoute*	p_event_route	(i)	内部イベント送信ルートインスタンスポインタ
           	CVhalSysdbControl*	p_sysdbctrl		(i)	SysDB制御インスタンスポインタ
           	void*				p_ccm_obj		(i)	CCMオブジェクトポインタ
           	bool				str_resume		(i)	起動種別(true:STRレジューム時 false:通常起動時)
 戻り値  ：	処理結果
           		VHAL_ERR_****			エラー
           		VHAL_SUCCESS			正常終了
 フェールセーフNo  ：	F-VHAL-N-301
                    	F-VHAL-R-302
                    	F-VHAL-N-304
                    	F-VHAL-R-305
                    	F-VHAL-R-366
*****************************************************************************/
int32_t CVhalMiconCommMiscControl::Initialize(CVhalEventRoute* const p_event_route, CVhalSysdbControl* const p_sysdbctrl, void* const p_ccm_obj, const bool str_resume)
{
	int32_t result{VHAL_SUCCESS};

	/* 引数調査 */
	if ((nullptr == p_event_route) || (nullptr == p_sysdbctrl) || (nullptr == p_ccm_obj))
	{
		VHAL_LOGE("parameter error. route[%p] sysdb[%p] ccmobj[%p]", p_event_route, p_sysdbctrl, p_ccm_obj);
		result = VHAL_ERR_PARAM;
	}

	constexpr uint32_t kMiscCtrlApiCbRetryMax{120U};			/* MISC通知CB登録リトライ回数 */
	constexpr uint32_t kMiscCtrlApiCbRetryWait{500U};			/* MISC通知CB登録リトライ待ち時間(ms) */

	/* ステータス通知Callback登録 */
	if (VHAL_SUCCESS == result)
	{
		p_event_route_ = p_event_route;
		p_misc_control_ = this;

		for (uint32_t i{1U}; i<=kMiscCtrlApiCbRetryMax; ++i)
		{
			/* MISC_ステータス通知Callback登録 */
			INT32 ret{MISC_CTRL_RET_SUCCESS};
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM	// F-VHAL-N-301, F-VHAL-R-302
//			CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-N-301",ret);
//			CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-R-302",ret);
//			if( MISC_CTRL_RET_SUCCESS == ret )
//#endif
			ret = MiscCtrlApiEntStsCB(p_ccm_obj, MiconCommMiscControlStsCallBack);

			/* リトライ系エラー */
			if ((MISC_CTRL_RET_ERR_SYSTEM == ret) || (MISC_CTRL_RET_ERR_COMM == ret) || (MISC_CTRL_RET_ERR_OTHER == ret))
			{
				/* リトライオーバー判定 */
				if (kMiscCtrlApiCbRetryMax <= i)
				{
					/* リトライオーバー */
					VHAL_LOGE("MiscCtrlApiEntStsCB error ret=%d retry max(%u)", ret, i);
					result = VHAL_ERR;
				}
				else
				{
					/* リトライ待ち */
					sif_mdelay(kMiscCtrlApiCbRetryWait);
				}
			}
			else
			{
				/* 正常または即中断系エラー */
				if( MISC_CTRL_RET_SUCCESS != ret )
				{
					/* 正常以外 */
					VHAL_LOGE("MiscCtrlApiEntStsCB error ret=%d count=%u", ret, i);
					result = VHAL_ERR;
				}
				else
				{
					VHAL_LOGI("MiscCtrlApiEntStsCB success!");
				}
				break;
			}
		}
	}

	/* ステータス通知待ち */
	if (VHAL_SUCCESS == result)
	{
		std::unique_lock<std::mutex> lock_sync(mtx_sts_sync_);
		constexpr uint32_t timeout_seconds{60U};
		const bool intime{cond_sts_sync_.wait_for(lock_sync, std::chrono::seconds(timeout_seconds), [] (void) noexcept { return (MISC_CTRL_STS_EXEC == p_misc_control_->GetCtrlSts()); })};
		if (!intime)
		{
			VHAL_LOGE("cond_sts_sync_.wait_for error");
			result = VHAL_ERR;
		}
	}

	/* CCMサービスの起動状態の確認 (STRレジューム時のみ実施：最大約10秒待ち) */
	if ((VHAL_SUCCESS == result) && (str_resume))
	{
		constexpr uint32_t kSysdbReadCcmRetryMax{100U};		/* ccm_svc確認リトライ回数 */
		constexpr uint32_t kSysdbReadCcmRetryWait{100U};	/* ccm_svc確認リトライ待ち時間(ms) */
		bool loop_flg{true};
		for (uint32_t count{1U}; (count <= kSysdbReadCcmRetryMax) && loop_flg; ++count)
		{
			int32_t ccm_svc{DAT_CNV_SERVICE_NONE};
			const int32_t ret{p_sysdbctrl->GetValue(VHAL_SYSDB_PATH_CCMSVC_STATE, &ccm_svc)};
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM	// F-VHAL-R-366 (ret値のフェールセーフはF-VHAL-R-049で対応)
//			CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-R-366",ccm_svc,count);
//#endif
			if ((VHAL_SUCCESS == ret) && (DAT_CNV_SERVICE_RUNNING == ccm_svc))
			{
				VHAL_LOGI("get ccm_svc running! count=%u", count);
				loop_flg = false;
			}
			else if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("GetValue failed. [%s] ret=%d count=%u",VHAL_SYSDB_PATH_CCMSVC_STATE.c_str(), ret, count);
				result = VHAL_ERR;
				loop_flg = false;
			}
			else if (count >= kSysdbReadCcmRetryMax)
			{
				VHAL_LOGE("ccm_svc is not DAT_CNV_SERVICE_RUNNING. ret=%d ccm_svc=%d count=%u", ret, ccm_svc, count);
				result = VHAL_ERR;
			}
			else
			{
				sif_mdelay(kSysdbReadCcmRetryWait);		/* リトライ待ち */
			}
		}
	}

	if (VHAL_SUCCESS == result)
	{
		std::vector<MiscCtrlApiNtyFilterData>	nty_filter_data{
			{ OCB_DATA_TYPE_CAM		, SUB_TYPE_CAMERA_TYPE				},	/* カメラ種別判別通知 */
			{ OCB_DATA_TYPE_CAM		, SUB_TYPE_CAMERA_SYNC				},	/* 同期検知・経路情報通知 */
			{ OCB_DATA_TYPE_HDMI	, SUB_TYPE_HDMI_CONNECT				},	/* HDMI機器接続状態通知 */
			{ OCB_DATA_TYPE_HDMI	, SUB_TYPE_HDMI_VIDEO_FORMAT		},	/* HDMIビデオフォーマット変更通知 */
			{ OCB_DATA_TYPE_HDMI	, SUB_TYPE_HDMI_AUDIO_FORMAT		},	/* HDMIオーディオフォーマット変更通知 */
			{ OCB_DATA_TYPE_HDMI	, SUB_TYPE_HDCP_AUTH_NTY_CDISP		},	/* C-Disp_HDCP認証応答 */
//			{ OCB_DATA_TYPE_HDMI	, SUB_TYPE_HDCP_AUTH_NTY_RSE		},	(予定)	/* RSE_HDCP認証応答 */
			{ OCB_DATA_TYPE_DISP	, SUB_TYPE_DISP_MODE_RSP			},	/* 画質モード応答 */
			{ OCB_DATA_TYPE_DISP	, SUB_TYPE_DISP_SCREEN_SHOT_RSP		},	/* スクリーンショット応答 */
			{ OCB_DATA_TYPE_DISP	, SUB_TYPE_DISP_HUD_FUNC_STATUS		},	/* HUD機能有無判定結果通知 */
			{ OCB_DATA_TYPE_DISP	, SUB_TYPE_DISP_HUD_DISTORTION_CORRECTION	},	/* HUD歪み補正通知 */
			{ OCB_DATA_TYPE_DISP	, SUB_TYPE_DISP_HUD_ROTATION		},	/* HUD回転パラメータ通知 */
		};
		MiscCtrlApiNtyFilter        nty_filter;
		nty_filter.num   = ToUI8(nty_filter_data.size());	/* フィルタデータ情報数 */
		nty_filter.array = nty_filter_data.data();

		for (uint32_t i{1U}; i<=kMiscCtrlApiCbRetryMax; ++i)
		{
			/* MISC_コマンド通知Callback登録 */
			INT32 ret{MISC_CTRL_RET_SUCCESS};
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM	// F-VHAL-N-304, F-VHAL-R-305
//			CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-N-304",ret);
//			CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-R-305",ret);
//			if( MISC_CTRL_RET_SUCCESS == ret )
//#endif
			ret = MiscCtrlApiEntDataNtyCB(p_ccm_obj, MiconCommMiscControlDataNtyCallBack, &nty_filter);
			/* リトライ系エラー */
			if ((MISC_CTRL_RET_ERR_SYSTEM == ret) || (MISC_CTRL_RET_ERR_COMM == ret) || (MISC_CTRL_RET_ERR_OTHER == ret))
			{
				/* リトライオーバー判定 */
				if (kMiscCtrlApiCbRetryMax <= i)
				{
					/* リトライオーバー */
					VHAL_LOGE("MiscCtrlApiEntDataNtyCB error ret=%d retry max(%u)", ret, i);
					result = VHAL_ERR;
				}
				else
				{
					/* リトライ待ち */
					sif_mdelay(kMiscCtrlApiCbRetryWait);
				}
			}
			else
			{
				/* 正常または即中断系エラー */
				if( MISC_CTRL_RET_SUCCESS != ret )
				{
					/* 正常以外 */
					VHAL_LOGE("MiscCtrlApiEntDataNtyCB error ret=%d count=%u", ret, i);
					result = VHAL_ERR;
				}
				else
				{
					VHAL_LOGI("MiscCtrlApiEntDataNtyCB success!");
				}
				break;
			}
		}
	}
	return result;
}

/*****************************************************************************
 処理概要：	終了処理
 引数    ：	void*	p_ccm_obj	(i)	CCMオブジェクトポインタ
 戻り値  ：	なし
*****************************************************************************/
void CVhalMiconCommMiscControl::Finalize(void* const p_ccm_obj)
{
	/* MiscCtrlとメンバ変数の初期化 */
	ReInit(p_ccm_obj);

	/* MISC_コマンド受信リストの削除 */
	if (false == receive_list_.empty())
	{
		receive_list_.clear();
	}
}

/*****************************************************************************
 処理概要：	再初期化処理
 引数    ：	void*	p_ccm_obj	(i)	CCMオブジェクトポインタ
 戻り値  ：	なし
 フェールセーフNo  ：	F-VHAL-C-316
                    	F-VHAL-C-317
*****************************************************************************/
void CVhalMiconCommMiscControl::ReInit(void* const p_ccm_obj) noexcept
{
	/* RESUME時に備え、MISC設定やメンバ変数を初期化 */
	if (nullptr != p_ccm_obj)
	{
		/* MISC_データ通知CB解除 */
		INT32 ret{MiscCtrlApiDelDataNtyCB(p_ccm_obj)};
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM	// F-VHAL-C-317
//		CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-C-317",ret);
//#endif
		if (MISC_CTRL_RET_SUCCESS != ret)
		{
			VHAL_LOGE("MiscCtrlApiDelDataNtyCB error ret=%d", ret);
		}

		/* MISC_ステータス通知CB解除 */
		ret = MiscCtrlApiDelStsCB(p_ccm_obj);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM	// F-VHAL-C-316
//		CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-C-316",ret);
//#endif
		if (MISC_CTRL_RET_SUCCESS != ret)
		{
			VHAL_LOGE("MiscCtrlApiDelStsCB error ret=%d", ret);
		}
	}

	/* RESUME後の初回通知を抑止しないため、比較用キャッシュをclearする */
	ctrl_sts_ = MISC_CTRL_STS_NONE;	/* 設定クリア */
	cmd_camera_sync_.clear();
	cmd_camera_type_.clear();

	cmd_hdmi_video_format_.clear();
	cmd_hdmi_audio_format_.clear();
	cmd_dispmode_resp_.clear();

	/* HUDの歪みパラメータについてもRESUME後の初回通知を抑止しないため、比較用キャッシュをclearする */
	cmd_disp_hud_func_status_.clear();				/* HUD機能ステータス */
	cmd_disp_hud_distortion_correction_.clear();	/* HUD歪み補正パラメータ */
	cmd_disp_hud_rotation_.clear();					/* HUD回転パラメータ */	
}

/*****************************************************************************
 処理概要：	MISC_状態通知Callback
 引数    ：	MiscCtrlApiSts*	p_sts	(i)	OS間通信状態
 戻り値  ：	なし
*****************************************************************************/
extern "C" void MiconCommMiscControlStsCallBack(MiscCtrlApiSts* p_sts)
{
	if ((nullptr == p_sts) || (nullptr == p_misc_control_))
	{
		VHAL_LOGE("error. p_sts=%p p_misc_control_=%p", p_sts, p_misc_control_);
	}
	else
	{
		const uint32_t	ctrl_sts{p_sts->ctrl_sts};
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM	// F-VHAL-R-303
//		int32_t fail_sts = static_cast<int32_t>(ctrl_sts);
//		CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-R-303",fail_sts);
//		uint32_t* p_ctrl_sts{const_cast<uint32_t*>(&ctrl_sts)};
//		*p_ctrl_sts = static_cast<uint32_t>(fail_sts);
//#endif
		VHAL_LOGI("MISC-STS-CB. ctrl_sts=%u", ctrl_sts);
		p_misc_control_->NotifyMiscCtrlSts(ctrl_sts);
	}
	return;
}


/*****************************************************************************
 処理概要：	MISC_コマンド通知Callback
 引数    ：	uint8_t		data_type	(i)	データタイプ
           	void*		p_cmd		(i)	コマンド情報ポインタ
           	uint32_t	len			(i)	コマンドデータサイズ
 戻り値  ：	なし
*****************************************************************************/
extern "C" void MiconCommMiscControlDataNtyCallBack(const UINT8 data_type, void* const p_cmd, const UINT32 len)
{
	if ((nullptr == p_cmd) || (nullptr == p_misc_control_) || (0U == len))
	{
		VHAL_LOGE("error. p_cmd=%p  p_misc_control_=%p len=%u", p_cmd, p_misc_control_, len);
	}
	else
	{
		/* カメラ、DISPLAYの通知について、前回と内容が同じの場合は通知しない */
		/* HDMIについてはエラー時の通知が前回と内容が同じの場合は通知しない */
		bool enabled{p_misc_control_->IsNotifyEnabled(len, p_cmd, data_type)};
		if (true == enabled)
		{
			VHAL_LOGI("MISC-NTFY-CB. data_type=0x%02X sub_type=0x%02X len=%u", data_type, ((UINT8*)p_cmd)[0], len);	// BEVstep3 25/01/22 ログ強化

			/* 受信データ事前通知（MISC_コールバックスレッドからのコール） */
			p_misc_control_->NotifyReceivePre(len, p_cmd, data_type);

			/* 内部イベント通知 */
			p_misc_control_->NotifyReceiveEvent(len, p_cmd, data_type);
		}
	}
	return;
}

/*****************************************************************************
 処理概要：	MISC_ステータス受信
 引数    ：	uint32_t		ctrl_sts	(i)Ctrlステータス
 戻り値  ：	なし
*****************************************************************************/
void CVhalMiconCommMiscControl::NotifyMiscCtrlSts(const uint32_t ctrl_sts)
{
	/* ステータスの通知処理 */
	{
		std::lock_guard<std::mutex> lock_sync{mtx_sts_sync_};
		ctrl_sts_ = ctrl_sts;
	}
	cond_sts_sync_.notify_one();
}

/*****************************************************************************
 処理概要：	MISC_コマンドデータ受信事前通知
 引数    ：	uint32_t	data_size	(i)受信データサイズ
           	void*		data		(i)受信データ
           	uint8_t		data_type	(i)データタイプ
 戻り値  ：	なし
*****************************************************************************/
void CVhalMiconCommMiscControl::NotifyReceivePre(const uint32_t data_size, const void* const data, const uint8_t data_type)
{
	const std::lock_guard<std::mutex> lock_data{mtx_recv_item_};
	std::vector<uint8_t> cmd{};

	/* 受信データを格納 */
	cmd.resize(static_cast<std::size_t>(data_size + 1U));
	(void)memcpy(static_cast<void *>(cmd.data()), data, static_cast<size_t>(data_size));
	cmd[data_size] = data_type;

	for (const auto p_rcv_item : receive_list_)
	{
		p_rcv_item->ReceivePreNotify(cmd);
	}
}

/*****************************************************************************
 処理概要：	MISC_コマンド送信
 引数    ：	void*					p_ccm_obj	(i)	CCMオブジェクトポインタ
           	std::vector<uint8_t>&	send_data	(i)	送信コマンド(最終データにdata_typeを格納)
 戻り値  ：	処理結果
           		VHAL_ERR_****			エラー
           		VHAL_SUCCESS			正常終了
 フェールセーフNo  ：	F-VHAL-N-318
                     	F-VHAL-R-319
                     	F-VHAL-R-320
                     	F-VHAL-C-321
                     	F-VHAL-R-322
						F-VHAL-R-349
*****************************************************************************/
int32_t CVhalMiconCommMiscControl::Send(void* const p_ccm_obj, std::vector<uint8_t> &send_data)
{
	int32_t result{VHAL_SUCCESS};

	/* 送信データなしチェック */
	constexpr uint32_t kMinSendDataSize{2U};  // sub_type + data_type
	if ((nullptr == p_ccm_obj) || (kMinSendDataSize > send_data.size()))
	{
		result = VHAL_ERR;
	}
	else
	{
		INT32 ret{MISC_CTRL_RET_SUCCESS};
		constexpr uint32_t kDataTypeLength{1U};				/* data_type長 */
		const uint32_t datasize{SizeToUI32(static_cast<std::size_t>(send_data.size() - kDataTypeLength))};
		constexpr uint32_t kMiscCtrlApiCmdSendRetryMax{3U};	/* MISC_コマンドリトライ回数 */
		bool done{false};
		for (uint32_t i{0U}; (!done) && (i<kMiscCtrlApiCmdSendRetryMax); ++i)
		{
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM	// F-VHAL-N-318, F-VHAL-R-319, F-VHAL-R-320, F-VHAL-C-321, F-VHAL-R-322
//			ret = MISC_CTRL_RET_SUCCESS;
//			CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-N-318",ret);
//			CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-R-319",ret);
//			if ((kDatatypeDisplay == send_data[datasize]) && (SUB_TYPE_DISP_MODE_REQ == send_data[0]))	// 画質モード要求
//			{
//				CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-R-320",ret);
//			}
//			if ((kDatatypeCamera == send_data[datasize]) && (SUB_TYPE_CAMERA_REQUEST == send_data[0]))	// カメラ種別判別要求
//			{
//				CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-C-321",ret);
//			}
//			if ((kDatatypeCamera == send_data[datasize]) && (SUB_TYPE_CAMERA_MODE == send_data[0]))		// カメラ映像モード通知
//			{
//				CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-R-322",ret);
//			}
//			if ((kDatatypeDisplay == send_data[datasize]) && (SUB_TYPE_DISP_SCREEN_SHOT_REQ == send_data[0]))		// スクリーンショット要求
//			{
//				CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-R-349",ret);
//			}
//			if (MISC_CTRL_RET_SUCCESS == ret)	// 非エラー時のみMiscCtrlApiDataSend()を呼ぶ
//#endif
			ret = MiscCtrlApiDataSend(p_ccm_obj, send_data[datasize], send_data.data(), datasize);
			if (MISC_CTRL_RET_SUCCESS == ret)
			{
				if ((kDatatypeCamera == send_data[datasize]) && (SUB_TYPE_CAMERA_MODE == send_data[0]))		// カメラ映像モード通知
				{
					VHAL_LOGV("MiscCtrlApiDataSend success! data_type=0x%02X sub_type=0x%02X len=%u", send_data[datasize], send_data[0], datasize);	// BEVstep3 25/01/22 ログ強化
				}
				else
				{
					VHAL_LOGI("MiscCtrlApiDataSend success! data_type=0x%02X sub_type=0x%02X len=%u", send_data[datasize], send_data[0], datasize);	// BEVstep3 25/01/22 ログ強化
				}
				done = true;
			}
			else if ((MISC_CTRL_RET_ERR_OVER == ret)
				 ||  (MISC_CTRL_RET_ERR_SYSTEM == ret)
				 ||  (MISC_CTRL_RET_ERR_COMM == ret)
				 ||  (MISC_CTRL_RET_ERR_OTHER == ret))
			{
				/* 送信リソース枯渇の場合、少し時間を空けてリトライする */
				VHAL_LOGW("MiscCtrlApiDataSend retry count=%d", i+1U);
				constexpr uint32_t kMiscCtrlApiCmdSendRetryWait{500U};	/* MISC_コマンド送信リトライ待ち時間(ms) */
				sif_mdelay(kMiscCtrlApiCmdSendRetryWait);
			}
			else
			{
				/* 送信失敗 */
				VHAL_LOGE("MiscCtrlApiDataSend error ret=%d", ret);
				done = true;
				result = VHAL_ERR;
			}
		}

		if ((MISC_CTRL_RET_SUCCESS != ret) && (true != done))
		{
			VHAL_LOGE("MiscCtrlApiDataSend error retry over");
			result = VHAL_ERR;
		}
	}

	return result;
}

/*****************************************************************************
 処理概要：	内部イベントデータ受信CB登録
 引数    ：	CVhalMiconReceiveItem*	p_rcv_item	(i)	受信アイテムモジュールポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalMiconCommMiscControl::RegistryReceiver(CVhalMiconReceiveItem* const p_rcv_item)
{
	const std::lock_guard<std::mutex> lock_data{mtx_recv_item_};

	/* リストにp_rcv_itemを登録 */
	receive_list_.push_back(p_rcv_item);

	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	内部イベントデータ受信CB削除
 引数    ：	CVhalMiconReceiveItem*	p_rcv_item	(i)	受信アイテムモジュールポインタ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalMiconCommMiscControl::ClearReceiver(CVhalMiconReceiveItem* const p_rcv_item)
{
	const std::lock_guard<std::mutex> lock_data{mtx_recv_item_};

	/* リストからp_rcv_itemを削除 */
	receive_list_.remove(p_rcv_item);

	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	内部イベント通知
 引数    ：	uint32_t	dataSize	(i)受信データサイズ
           	void*		data		(i)受信データ
 戻り値  ：	なし
*****************************************************************************/
void CVhalMiconCommMiscControl::NotifyReceiveEvent(const uint32_t dataSize, const void* const data, const uint8_t data_type)
{
	std::unique_ptr<CVhalMiscEventItem> p_recv_event{std::make_unique<CVhalMiscEventItem>(this)};

	p_recv_event->SetName(std::string("miscctrl event"));
	p_recv_event->SetData(dataSize, data, data_type);
	const int32_t ret{p_event_route_->WriteEvent(p_recv_event.get())};
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGEW("p_event_route_->WriteEvent ret=%d", ret);
	}
	else
	{
		(void)p_recv_event.release();
	}
}

/*****************************************************************************
 処理概要：	内部イベント処理
 引数    ：	std::vector<uint8_t>	recv_data		(i)受信データ
 戻り値  ：	なし
*****************************************************************************/
void CVhalMiconCommMiscControl::ExecReceiveEvent(const std::vector<uint8_t> recv_data)
{
	/* 本関数全域で排他(mtx_recv_item_)するとMute機能のMute状態一致確認(Receive処理内のwait_for)時に */
	/* MISC CBスレッドでの排他(mtx_recv_item_)ができずに必ず待ち合わせ(wait_forのタイムアウト時間分)が発生してしまう */
	/* 対策として、本関数での排他区間(mtx_recv_item_)を最小限に抑える(*1)ことで */
	/* MISC CBスレッドでの排他(mtx_recv_item_)待ち合わせが発生しなくなる */
	/* *1:Receive処理を排他区間から外すため、ReceiveItemリストのコピーを取得する */
	/* また、本対策ではReceiveItemリスト減少(内部CB登録解除)時に削除済みアイテムのReceive処理へアクセスする懸念があるが */
	/* ReceiveItemリスト減少(内部CB登録解除)処理は同一スレッド上で実行されるため問題は発生しない */
	std::list<CVhalMiconReceiveItem*> local_list{};
	{
		const std::lock_guard<std::mutex> lock_data{mtx_recv_item_};
		local_list = receive_list_;
	}

	/* コピーしたリストに登録されているReceive(recv_data)の実行 */
	for (const auto p_rcv_item : local_list)
	{
		p_rcv_item->Receive(recv_data);
	}

}

/*****************************************************************************
 処理概要： MISC_Controlステータス取得
 引数    ：	なし
 戻り値  ：	Controlステータス
*****************************************************************************/
uint32_t CVhalMiconCommMiscControl::GetCtrlSts(void)
{
	return ctrl_sts_;
}

/*****************************************************************************
 処理概要：	通知イベント有効判定
 引数    ：	uint32_t	data_size	(i)受信データサイズ
           	void*		data		(i)受信データ
           	uint8_t		data_type	(i)データタイプ
 戻り値  ：	判定結果
           		true	通知有効
           		false	通知無効
 フェールセーフNo  ：	F-VHAL-N-307
                    	F-VHAL-N-309
                    	F-VHAL-N-311
                    	F-VHAL-N-313
                    	F-VHAL-N-315
                    	F-VHAL-N-332
                    	F-VHAL-N-335
*****************************************************************************/
bool CVhalMiconCommMiscControl::IsNotifyEnabled(const uint32_t data_size, const void* const data, const uint8_t data_type)
{
	bool result{true};

	/* 呼出し元関数でチェック済みの為、本関数への引数は正しいはずであるがcoverity対応によりチェックを実装 */
	if ((0U == data_size) || (nullptr == data))
	{
		VHAL_LOGW("error. data=%p size=%u", data, data_size);
		result = false;
	}
	else
	{
		/* 受信データを格納 */
		std::vector<uint8_t> cmd(static_cast<std::size_t>(data_size));
		(void)memcpy(cmd.data(), data, static_cast<std::size_t>(data_size));
		const uint8_t	sub_type{cmd[0]};

		/* カメラ系 */
		if (kDatatypeCamera == data_type)
		{
			/* カメラ種別判別通知 (34h-02h) */
			if (SUB_TYPE_CAMERA_TYPE == sub_type)
			{
				/* 前回と内容同じならば通知対象外 */
				if (cmd == cmd_camera_type_)
				{
					result = false;
				}
				else
				{
					cmd_camera_type_ = std::move(cmd);
				}
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM	// F-VHAL-N-307
//				int32_t value{0};
//				CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-N-307",value);
//				if (0 != value)
//				{
//					result = false;
//					VHAL_LOGW("fail F-VHAL-N-307");
//				}
//#endif
			}

			/* カメラ同期検知通知 (34h-03h) */
			else if (SUB_TYPE_CAMERA_SYNC == sub_type)
			{
				/* 前回と内容同じならば通知対象外 */
				if (cmd == cmd_camera_sync_)
				{
					result = false;
				}
				else
				{
					cmd_camera_sync_ = std::move(cmd);
				}
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM	// F-VHAL-N-309
//				int32_t value{0};
//				CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-N-309",value);
//				if (0 != value)
//				{
//					result = false;
//					VHAL_LOGW("fail F-VHAL-N-309");
//				}
//#endif
			}
		}

		/* HDMI系 */
		else if (kDatatypeHdmi == data_type)
		{
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM	// F-VHAL-N-311, F-VHAL-N-313, F-VHAL-N-315, F-VHAL-N-332, F-VHAL-N-335
//			std::string fail_id{};
//			int32_t value{0};
//			switch (sub_type)
//			{
//				case SUB_TYPE_HDMI_CONNECT:
//					fail_id = "F-VHAL-N-311";
//					break;
//				case SUB_TYPE_HDMI_VIDEO_FORMAT:
//					fail_id = "F-VHAL-N-313";
//					break;
//				case SUB_TYPE_HDMI_AUDIO_FORMAT:
//					fail_id = "F-VHAL-N-315";
//					break;
//				case SUB_TYPE_HDCP_AUTH_NTY_CDISP:
//					fail_id = "F-VHAL-N-332";
//					break;
//				case SUB_TYPE_HDCP_AUTH_NTY_RSE:
//					fail_id = "F-VHAL-N-335";
//					break;
//				default:
//					fail_id = "nop";
//					break;
//			}
//			CVhalDebugSystem::GetInstance().UpdateFailData(fail_id,value);
//			if (0 != value)
//			{
//				result = false;
//				VHAL_LOGW("fail %s", fail_id.c_str());
//			}
//#endif
			/* ビデオフォーマット変更通知 (39h-02h) */
			if (SUB_TYPE_HDMI_VIDEO_FORMAT == cmd[0])
			{
				/* 前回値と比較するための最小データサイズを満たしているか */
				if (data_size >= kHDMIFormatDataSize)
				{
					/* 前回値が仕様外[80h]またはエラー[81h]かつ受信値が前回と同じならば通知対象外 */
					if (((cmd[1] == kHdmiFormatOutOfSpec) || (cmd[1] == kHdmiFormatError))
						 && (cmd == cmd_hdmi_video_format_))
					{
						result = false;
					}
					else
					{
						cmd_hdmi_video_format_ = std::move(cmd);
					}
				}
			}
			/* オーディオフォーマット変更通知 (39h-03h) */
			else if (SUB_TYPE_HDMI_AUDIO_FORMAT == cmd[0])
			{
				/* 前回値と比較するための最小データサイズを満たしているか */
				if (data_size >= kHDMIFormatDataSize)
				{
					/* 前回値が仕様外[80h]またはエラー[81h]かつ受信値が前回と同じならば通知対象外 */
					if (((cmd[1] == kHdmiFormatOutOfSpec) || (cmd[1] == kHdmiFormatError))
						 && (cmd == cmd_hdmi_audio_format_))
					{
						result = false;
					}
					else
					{
						cmd_hdmi_audio_format_ = std::move(cmd);
					}
				}
			}
		}

		/* DISPLAY系 */
		else if (kDatatypeDisplay == data_type)
		{
			result = IsNotifyDisplayEnabled(cmd);
		}
	}

	return result;
}
/*****************************************************************************
 処理概要：	通知イベント有効判定
 引数    ：	const std::vector<uint8_t> cmd	(i)受信データ
 戻り値  ：	判定結果
           		true	通知有効
           		false	通知無効
 フェールセーフNo  ：	F-VHAL-N-346
                    	F-VHAL-N-XXX
                    	F-VHAL-N-XXX
                    	F-VHAL-N-XXX
*****************************************************************************/
bool CVhalMiconCommMiscControl::IsNotifyDisplayEnabled(const std::vector<uint8_t> cmd)
{
	bool result{true};

	if (false == cmd.empty())
	{
		const uint8_t	sub_type{cmd[0]};

		/* 画質モード応答 (36h-02h) */
		if (SUB_TYPE_DISP_MODE_RSP == sub_type)
		{
			/* 前回と内容同じならば通知対象外 */
			if (cmd == cmd_dispmode_resp_)
			{
				result = false;
			}
			else
			{
				cmd_dispmode_resp_ = std::move(cmd);
			}
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM	// F-VHAL-N-346
//			int32_t value{0};
//			CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-N-346",value);
//			if (0 != value)
//			{
//				result = false;
//				VHAL_LOGW("fail F-VHAL-N-346");
//			}
//#endif
		}
		/* HUD機能有無判定結果通知 (36h-43h)*/
		else if (static_cast<uint8_t>(SUB_TYPE_DISP_HUD_FUNC_STATUS) == sub_type)
		{
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM // HUD機能有無判定結果通知(36h-43h)未受信
//			int32_t value_hud_status{0};
//			CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-N-XXX", value_hud_status);
//			if (0 != value_hud_status)
//			{
//				result = false;
//				VHAL_LOGW("fail F-VHAL-N-XXX");
//			}
//#else
			if (cmd == cmd_disp_hud_func_status_)
			{
				result = false;
			}
			else
			{
				cmd_disp_hud_func_status_ = std::move(cmd);
			}
//#endif
		}
		/* HUD歪み補正パラメータ通知 (36h-44h)*/
		else if (static_cast<uint8_t>(SUB_TYPE_DISP_HUD_DISTORTION_CORRECTION) == sub_type)
		{
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM	// HUD歪み補正パラメータ通知(36h-44h)未受信。
//			int32_t value_hud_correction{0};
//			CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-N-XXX", value_hud_correction);
//			if (0 != value_hud_correction)
//			{
//				result = false;
//				VHAL_LOGW("fail F-VHAL-N-XXX");
//			}
//#else
			if (cmd == cmd_disp_hud_distortion_correction_)
			{
				result = false;
			}
			else
			{
				cmd_disp_hud_distortion_correction_ = std::move(cmd);
			}
//#endif
		}
		/* HUD回転パラメータ通知 (36h-45h)*/
		else if (static_cast<uint8_t>(SUB_TYPE_DISP_HUD_ROTATION) == sub_type)
		{
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM	// HUD回転パラメータ通知(36h-45h)未受信
//			int32_t value_hud_rotation{0};
//			CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-N-XXX", value_hud_rotation);
//			if (0 != value_hud_rotation)
//			{
//				result = false;
//				VHAL_LOGW("fail F-VHAL-N-XXX");
//			}
//#else
			if (cmd == cmd_disp_hud_rotation_)
			{
				result = false;
			}
			else
			{
				cmd_disp_hud_rotation_ = std::move(cmd);
			}
//#endif
		}
	}
	else
	{
		/* 受信データが空の場合、通知無効 */
		VHAL_LOGW("cmd is empty");
		result = false;
	}

	return result;
}

/*****************************************************************************
 処理概要：	イベントリスナー登録
 引数    ：	CVhalMiscEventListenerBase* p_listener		(i)リスナーポインタ
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM			パラメータ不正
           		VHAL_SUCCESS			正常終了
****************************************************************************/
int32_t CVhalMiconMiscReceiver::RegisterEventListener(CVhalMiscEventListenerBase* const p_listener)
{
	int32_t	result{VHAL_SUCCESS};

	if (nullptr == p_listener)
	{
		VHAL_LOGE("p_listener is null");
		result = VHAL_ERR_PARAM;
	}
	else
	{
		p_misc_listener_ = p_listener;
	}

	return result;
}

/*****************************************************************************
 処理概要：	イベントリスナー解除
 引数    ：	なし
 戻り値  ：	なし
****************************************************************************/
void CVhalMiconMiscReceiver::ClearEventListener(void) noexcept
{
	p_misc_listener_ = nullptr;
}

/*****************************************************************************
 処理概要：	MISCコマンド通知受信処理（VideoHALイベントスレッドからのコール）
 引数    ：	const std::vector<uint8_t>&	data	(i)	コマンド情報ポインタ
 戻り値  ：	なし
*****************************************************************************/
void CVhalMiconMiscReceiver::Receive(const std::vector<uint8_t>& data)
{
	if (kMiscRecvDataSizeMin > data.size())
	{
		VHAL_LOGE("parameter error. data.size is %lu", data.size());
	}
	else if (kDatatypeHdmi == data[data.size() - 1])
	{
		VHAL_LOGI("received HDMI notification. sub_type=0x%02X param=0x%02X", data[DATA_INDEX_SUB_TYPE], data[1]);
		switch (data[DATA_INDEX_SUB_TYPE])
		{
			case SUB_TYPE_HDMI_CONNECT:
				ActionHdmiConnInfo(data);
				break;
			case SUB_TYPE_HDMI_VIDEO_FORMAT:
				ActionHdmiVideoFormat(data);
				break;
			case SUB_TYPE_HDMI_AUDIO_FORMAT:
				ActionHdmiAudioFormat(data);
				break;
			case SUB_TYPE_HDCP_AUTH_NTY_CDISP:
			case SUB_TYPE_HDCP_AUTH_NTY_RSE:	/* (追加予定) */
			case SUB_TYPE_HDCP_AUTH_KEY_CLEAR:
				/* 無処理 */
				break;
			default:
				VHAL_LOGE("invalid sub_type. 0x%X", data[DATA_INDEX_SUB_TYPE]);
				break;
		}
	}
	else
	{
		/* 無処理 */
	}
}

/*****************************************************************************
 処理概要：	MISCコマンド通知受信事前通知処理（通信モジュールスレッドからのコール）
 引数    ：	const std::vector<uint8_t>&	data	(i)	コマンド情報ポインタ
 戻り値  ：	なし
*****************************************************************************/
void CVhalMiconMiscReceiver::ReceivePreNotify(const std::vector<uint8_t>& data) noexcept
{
	/* 処理なし（Receiveで処理実施） */
}

/*****************************************************************************
 処理概要：	受信アイテム種別取得
 引数    ：	なし
 戻り値  ：	受信アイテム種別
           		RECEIVE_ITEM_TYPE_MISC
*****************************************************************************/
CVhalMiconReceiveItem::ReceiveItemType CVhalMiconMiscReceiver::GetItemType(void) const noexcept
{
	return CVhalMiconReceiveItem::ReceiveItemType::RECEIVE_ITEM_TYPE_MISC;
}

/*****************************************************************************
 処理概要：	HDMI接続検知イベント処理
 引数    ：	const std::vector<uint8_t>& recv_data   (i)受信データ
 戻り値  ：	なし
*****************************************************************************/
void CVhalMiconMiscReceiver::ActionHdmiConnInfo(const std::vector<uint8_t>& recv_data) const noexcept
{
	VHAL_LOGV_IN();
	int32_t state{kMiscRecvNotFound};
	uint8_t received_connection{recv_data[DATA_INDEX_CONNECTION]};
	switch (received_connection)
	{
		case 0x00U:
			state = DAT_CNV_SYSINFO_CONN_INFO_STAT_CONNECTING;	/* 0x00の場合は接続判断中(=2)とする  */
			break;
		case 0x01U:
			state = DAT_CNV_SYSINFO_CONN_INFO_STAT_DISCONNECT;	/* 0x01の場合は未接続(=1)とする */
			break;
		case 0x02U:
			state = DAT_CNV_SYSINFO_CONN_INFO_STAT_CONNECT;		/* 0x02の場合は接続済(=0)とする*/
			break;
		default:
			VHAL_LOGE("sub_type error. value=0x%02X", received_connection);
			break;
	}
	if (kMiscRecvNotFound != state)
	{
		p_misc_listener_->NotifyUpdateFilePropertyHdmi(state);
	}
	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	HDMIビデオフォーマット切替イベント処理
 引数    ：	const std::vector<uint8_t>& recv_data   (i)受信データ
 戻り値  ：	なし
*****************************************************************************/
void CVhalMiconMiscReceiver::ActionHdmiVideoFormat(const std::vector<uint8_t>& recv_data) const noexcept
{
	VHAL_LOGV_IN();
	uint8_t received_format{recv_data[DATA_INDEX_FORMAT]};
	int32_t format{kMiscRecvNotFound};
	switch (received_format)
	{
		case 0x00U:
			format = SYSINFO_CONN_INFO_VIDEO_FORMAT_INVALID;
			break;
		case 0x01U:
			format = SYSINFO_CONN_INFO_VIDEO_FORMAT_1280X720P;
			break;
		case 0x02U:
			format = SYSINFO_CONN_INFO_VIDEO_FORMAT_1920X1080P;
			break;
		case 0x03U:
			format = SYSINFO_CONN_INFO_VIDEO_FORMAT_720X480P;
			break;
		case 0x04U:
			format = SYSINFO_CONN_INFO_VIDEO_FORMAT_640X480P;
			break;
		case 0x80U:
			format = SYSINFO_CONN_INFO_VIDEO_FORMAT_OUTOFSPEC;
			VHAL_LOGW("HDMI video format is out of spec. uiFormat=0x%02X", received_format);
			break;
		case 0x81U:
			format = SYSINFO_CONN_INFO_VIDEO_FORMAT_ERROR;
			VHAL_LOGE("HDMI video format is abnormal. uiFormat=0x%02X", received_format);
			break;
		default:
			VHAL_LOGE("ActionHdmiVideoFormat() error. uiFormat=0x%02X", received_format);
			break;
	}
	if (kMiscRecvNotFound != format)
	{
		p_misc_listener_->NotifyChangeHdmiVideoFormat(format);
	}
	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	HDMIオーディオフォーマット切替イベント処理
 引数    ：	const std::vector<uint8_t>& recv_data   (i)受信データ
 戻り値  ：	なし
*****************************************************************************/
void CVhalMiconMiscReceiver::ActionHdmiAudioFormat(const std::vector<uint8_t>& recv_data) const noexcept
{
	VHAL_LOGV_IN();
	uint8_t received_format{recv_data[DATA_INDEX_FORMAT]};
	int32_t format{kMiscRecvNotFound};
	switch (received_format)
	{
		case 0x00U:
			format = SYSINFO_CONN_INFO_AUDIO_FORMAT_INVALID;
			break;
		case 0x01U:
			format = SYSINFO_CONN_INFO_AUDIO_FORMAT_32000;
			break;
		case 0x02U:
			format = SYSINFO_CONN_INFO_AUDIO_FORMAT_44100;
			break;
		case 0x03U:
			format = SYSINFO_CONN_INFO_AUDIO_FORMAT_48000;
			break;
		case 0x80U:
			format = SYSINFO_CONN_INFO_AUDIO_FORMAT_INVALID;
			VHAL_LOGW("HDMI audio format is abnormal. uiFormat=0x%02X", received_format);
			break;
		case 0x81U:
			format = SYSINFO_CONN_INFO_AUDIO_FORMAT_INVALID;
			VHAL_LOGE("HDMI audio format is abnormal. uiFormat=0x%02X", received_format);
			break;
		default:
			VHAL_LOGE("ActionHdmiAudioFormat() error. uiFormat=0x%02X", received_format);
			break;
	}
	if (kMiscRecvNotFound != format)
	{
		p_misc_listener_->NotifyChangeHdmiAudioFormat(format);
	}
	VHAL_LOGV_OUT();
}

} /* namespace videohal */

