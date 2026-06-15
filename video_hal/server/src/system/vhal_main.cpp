/*******************************************************************************
    機能名称    ：  VideoHALメインモジュール
    ファイル名称：  vhal_main.cpp
*******************************************************************************/
#include "vhal_main.h"

#include <cstdint>

#include "vhal_define.h"
#include "vhal_log.h"
#include "vhal_main_control.h"
#include "vhal_wait_event.h"
#include "vhal_event_item_power_state.h"
#include "vhal_str_mng.h"
#include "vhal_debug_system.h"

extern "C"
{
#include "pfm_tier1_public.h"
#include <sys_db_cnv_value_public.h>
}

namespace {

videohal::CVhalMainControl g_main_control{};

/*****************************************************************************
 処理概要：	(PFM)pre_initialize処理関数
 引数    ：	pfm_obj_t *obj          : PFMオブジェクトポインタ
         ： void *arg               : 引数ポインタ
 戻り値  ：	なし
*****************************************************************************/
extern "C" void vhal_pfm_pre_initialize(pfm_obj_t * const obj, void * const arg)
{
	VHAL_LOGI("");
}

/*****************************************************************************
 処理概要：	(PFM)initialize処理関数
 引数    ：	pfm_obj_t *obj          : PFMオブジェクトポインタ
         ： void *arg               : 引数ポインタ
 戻り値  ：	なし
*****************************************************************************/
extern "C" void vhal_pfm_initialize(pfm_obj_t * const obj, void * const arg)
{
	int32_t ret{videohal::VHAL_SUCCESS};

	VHAL_LOGI("start");		/* 初期化時間計測開始 */
	/* 初期化処理 */
	ret = g_main_control.Initialize();
	VHAL_LOGI("end");		/* 初期化時間計測終了 */
	if (videohal::VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("Initialize error ret=%d", ret);
	}
	else
	{
		/* サービス起動状態の設定 */
		videohal::CVhalPropertyControl* const p_prop{GetPropertyControl(&g_main_control)};
		if (nullptr != p_prop)
		{
			int32_t sysdb_ret{videohal::VHAL_SUCCESS};
			sysdb_ret = p_prop->SetServiceState(DAT_CNV_SERVICE_RUNNING);
			if (videohal::VHAL_SUCCESS != sysdb_ret)
			{
				VHAL_LOGE("SetServiceState(RUNNING) error. ret=%d", sysdb_ret);
			}
		}
	}
}

/*****************************************************************************
 処理概要：	(PFM)clear_data処理関数
 引数    ：	pfm_obj_t *obj          : PFMオブジェクトポインタ
         ： pfm_clear_kind_t kind   : クリア種別
         ： void *arg               : 引数ポインタ
 戻り値  ：	なし
*****************************************************************************/
extern "C" void vhal_pfm_clear_data(pfm_obj_t* const obj, const pfm_clear_kind_t kind, void * const arg)
{
	VHAL_LOGI("kind=%d", kind);

	switch (kind)
	{
		case PFM_CLEAR_ALL:
		case PFM_CLEAR_USER:
			{
				videohal::CVhalPropertyControl* const p_prop{GetPropertyControl(&g_main_control)};
				if (nullptr != p_prop)
				{
					int32_t ret{videohal::VHAL_SUCCESS};
					ret = p_prop->ClearColorStep();
					if (videohal::VHAL_SUCCESS != ret)
					{
						VHAL_LOGE("ClearColorStep() error. ret=%d", ret);
					}
					
					/* HDCP認証キークリア */
					ret = p_prop->ClearHdcpKey();
					if (videohal::VHAL_SUCCESS != ret)
					{
						VHAL_LOGE("ClearHdcpKey() error. ret=%d", ret);
					}
				}
			}
			break;

		case PFM_CLEAR_SVC_DIAG:
		case PFM_CLEAR_USER_REPRO:
		case PFM_CLEAR_DIAG_REPRO:
			/* 現状、処理無し */
			break;

		default:
			VHAL_LOGW("unknown kind: %d", kind);
			break;
	}
}

/*****************************************************************************
 処理概要：	(PFM)show_dbginfo処理関数
 引数    ：	pfm_obj_t *obj          : PFMオブジェクトポインタ
         ： void *arg               : 引数ポインタ
 戻り値  ：	なし
*****************************************************************************/
extern "C" void vhal_pfm_show_dbginfo(pfm_obj_t* const obj, void * const arg)
{
	VHAL_LOGI("");
}

/*****************************************************************************
 処理概要：	(PFM)shutdown処理関数
 引数    ：	pfm_obj_t *obj          : PFMオブジェクトポインタ
         ： void *arg               : 引数ポインタ
 戻り値  ：	なし
*****************************************************************************/
extern "C" void vhal_pfm_shutdown(pfm_obj_t * const obj, void * const arg)
{
	VHAL_LOGI("");

	videohal::CVhalPropertyControl* const p_prop{GetPropertyControl(&g_main_control)};
	if (nullptr != p_prop)
	{
		p_prop->Shutdown();
	}

	/* メインループ強制終了 */
	g_main_control.ExitEventLoop();
}

/*****************************************************************************
 処理概要：	(PFM)terminate処理関数
 引数    ：	pfm_obj_t *obj          : PFMオブジェクトポインタ
         ： void *arg               : 引数ポインタ
 戻り値  ：	なし
*****************************************************************************/
extern "C" void vhal_pfm_terminate(pfm_obj_t * const obj, void * const arg)
{
	VHAL_LOGI("");

	/* サービス起動状態の設定 */
	videohal::CVhalPropertyControl* const p_prop{GetPropertyControl(&g_main_control)};
	if (nullptr != p_prop)
	{
		int32_t sysdb_ret{videohal::VHAL_SUCCESS};
		sysdb_ret = p_prop->SetServiceState(DAT_CNV_SERVICE_STOP);
		if (videohal::VHAL_SUCCESS != sysdb_ret)
		{
			VHAL_LOGE("SetServiceState(STOP) error. ret=%d", sysdb_ret);
		}
	}

}

/*****************************************************************************
 処理概要：	(PFM)power_state処理関数
 引数    ：	pfm_obj_t *obj          : PFMオブジェクトポインタ
         ： pfm_pwr_sts_kind_t kind : 電源状態遷移種別
         ： void *arg               : 引数ポインタ
 戻り値  ：	なし
*****************************************************************************/
extern "C" void vhal_pfm_power_state(pfm_obj_t * const obj, const pfm_pwr_sts_kind_t kind, void * const arg)
{
	VHAL_LOGI("kind=%d", kind);
	
	videohal::ScenePowerState power_state{videohal::ScenePowerState::VHAL_PWR_STS_MAX};
	
	/* 電源状態によるMUTE設定 */
	switch (kind)
	{
		/* 乗車中状態 */
		case PFM_PWR_STS_IN_VEHICLE_START:
			power_state = videohal::ScenePowerState::VHAL_PWR_STS_IN_VEHICLE;
			break;

		/* 駐乗車起動状態 */
		case PFM_PWR_STS_BG_BOOT_START:
			power_state = videohal::ScenePowerState::VHAL_PWR_STS_BG_BOOT;
			break;

		/* 通常起動状態 */
		case PFM_PWR_STS_NML_BOOT_START:
			power_state = videohal::ScenePowerState::VHAL_PWR_STS_NML_BOOT;
			break;
		
		/* その他状態への遷移 */
		case PFM_PWR_STS_NML_BOOT_PRE_START:
		case PFM_PWR_STS_IN_VEHICLE_DONE:
		case PFM_PWR_STS_BG_BOOT_DONE:
		case PFM_PWR_STS_NML_BOOT_DONE:
			/* 処理なし */
			break;
		
		/* 異常値 */
		default:
			VHAL_LOGW("unknown kind: %d", kind);
			break;
	}
	
	if (videohal::ScenePowerState::VHAL_PWR_STS_MAX > power_state)
	{
		videohal::CVhalPropertyControl* const p_prop{GetPropertyControl(&g_main_control)};
		if (nullptr != p_prop)
		{
			std::unique_ptr<videohal::CVhalEventItemPowerState> p_power_state{std::make_unique<videohal::CVhalEventItemPowerState>()};
			if (nullptr == p_power_state)
			{
				VHAL_LOGE("new CVhalEventItemPowerState error.");
			}
			else
			{
				static videohal::CVhalWaitEvent g_wait_event{};			/* 電源状態遷移処理終了後（タイムアウト）にWriteEvent先からnotify_oneがコールされる可能性がある為、g_wait_eventはstaticで定義する */
				p_power_state->SetName(std::string("power state event"));
				p_power_state->SetData(p_prop, &g_wait_event, power_state);
				const int32_t ret{g_main_control.WriteEvent(p_power_state.get())};
				if (videohal::VHAL_SUCCESS == ret)
				{
					constexpr uint32_t kVhalTimerPwrStsWait{1000U};
					const bool intime{g_wait_event.Wait(kVhalTimerPwrStsWait)};
					if (false == intime)
					{
						VHAL_LOGEW("Change Power State Timeout");
					}
					(void)p_power_state.release();
				}
			}
		}
		else
		{
			VHAL_LOGE("parameter error. p_prop=%p", p_prop);
		}
	}
}

/*****************************************************************************
 処理概要：	(PFM)vehicle_power_state処理関数
 引数    ：	pfm_obj_t *obj                     : PFMオブジェクトポインタ
         ： pfm_vehicle_pwr_sts_data_t *p_kind : 車両電源ステート格納用オブジェクト
         ： void *arg                          : 引数ポインタ
 戻り値  ：	なし
 フェールセーフNo ：	F-VHAL-N-342
                    	F-VHAL-N-343
 備考　　： 本処理の実施スレッドはPFMのコールバックのみのため複数スレッドによる同期異常はない。
 *****************************************************************************/
extern "C" void vhal_pfm_vehicle_power_state(pfm_obj_t * const obj, pfm_vehicle_pwr_sts_data_t * const p_kind, void * const arg)
{

//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	int32_t fail_ret_none{0};
//	videohal::CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-N-343",fail_ret_none);
//	if (0 != fail_ret_none)
//	{
//		VHAL_LOGW("notify dropped. (Failsafe-343)");
//		return;		/* 通知を無かったことにする */
//	}
//#endif

	videohal::CVhalPropertyControl* const p_prop{GetPropertyControl(&g_main_control)};
	if ((nullptr == p_kind) || (nullptr == p_prop))
	{
		VHAL_LOGE("Detect null pointer. p_kind=%p p_prop=%p", p_kind, p_prop);
	}
	else
	{
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//		int32_t fail_ret_state{static_cast<int32_t>(p_kind->vehiclePwrSts.specialState)};
//		videohal::CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-N-342",fail_ret_state);
//		p_kind->vehiclePwrSts.specialState = static_cast<uint8_t>(fail_ret_state);
//#endif

		const int32_t					special_state{static_cast<int32_t>(p_kind->vehiclePwrSts.specialState)};		/* 特殊ステート */
		const int32_t					basic_state{static_cast<int32_t>(p_kind->vehiclePwrSts.basicState)};			/* 基本ステート */
		const int32_t					appearance_state{static_cast<int32_t>(p_kind->vehiclePwrSts.appearanceState)};	/* 見た目 */

		VHAL_LOGI("p_kind->transition=%d sp=%d bs=%d ap=%d", p_kind->transitionStatus, special_state, basic_state, appearance_state);

		/* 遷移開始通知時に実施 */
		if (PFM_VEHICLE_PWR_STS_TRANSIT_START == p_kind->transitionStatus)
		{
			videohal::VehiclePowerState	power_state{videohal::VehiclePowerState::VHAL_VPWR_STS_MAX};					/* 電源状態 */

			/* 各ステートの値から、power_stateを確定する */
			p_prop->ConvertVehiclePowerState(special_state, basic_state, appearance_state, power_state);

			/* 変換に失敗した場合は異常ログ */
			if (videohal::VehiclePowerState::VHAL_VPWR_STS_MAX <= power_state)
			{
				VHAL_LOGE("invalid power_state=%d", static_cast<int32_t>(power_state));
			}
			/* 変換に成功した場合はイベント送信 */
			else
			{
				/* 電源状態遷移処理終了後（タイムアウト）にWriteEvent先からnotify_oneがコールされる可能性がある為、g_wait_eventはstaticで定義する */
				static videohal::CVhalWaitEvent g_wait_event{};

				/* std::make_uniqueを使うと失敗時に例外が発生する。 */
				/* 例外が発生しない場合はp_power_stateの中身(videohal::CVhalEventItemVehiclePowerState)が存在するためnullチェックをしない。 */
				std::unique_ptr<videohal::CVhalEventItemVehiclePowerState> p_power_state{std::make_unique<videohal::CVhalEventItemVehiclePowerState>()};
				p_power_state->SetName(std::string("vehicle power state event"));
				p_power_state->SetData(p_prop, &g_wait_event, power_state);

				const int32_t ret{g_main_control.WriteEvent(p_power_state.get())};
				if (videohal::VHAL_SUCCESS == ret)
				{
					/* 所有権を放棄。実体解放はCVhalEventRoute::ExecEvent()にて実施 */
					(void)p_power_state.release();
					constexpr uint32_t kVhalTimerPwrStsWait{1000U};
					const bool intime{g_wait_event.Wait(kVhalTimerPwrStsWait)};
					if (false == intime)
					{
						/* タイムアウト時はリカバリ不要。ログにて警告は出しておく */
						VHAL_LOGW("Change Vehicle Power State Timeout");
					}
				}
			}
		}
	}
}

/*****************************************************************************
 処理概要：	(PFM)vhal_pfm_str_state処理関数
 引数    ：	pfm_obj_t *obj              : PFMオブジェクトポインタ
         ： pfm_str_sts_data_t *p_kind  : 車両電源ステート格納用オブジェクト
         ： void *arg                   : 引数ポインタ
 戻り値  ：	なし
 フェールセーフNo ：	F-VHAL-N-354
                    	F-VHAL-N-355
                    	F-VHAL-N-356
*****************************************************************************/
extern "C" void vhal_pfm_str_state(pfm_obj_t * const /* obj */, pfm_str_sts_data_t * const p_kind, void * const /* arg */)
{
	/* 引数objとargは未使用 */

	videohal::CVhalPropertyControl* const p_prop{GetPropertyControl(&g_main_control)};
	if ((nullptr == p_kind) || (nullptr == p_prop))
	{
		VHAL_LOGE("Detect null pointer. p_kind=%p p_prop=%p", p_kind, p_prop);
	}
	else
	{
		const bool							str_mode{videohal::CVhalStrManager::GetStrMode()};
		const pfm_str_transition_status_t 	status{p_kind->strTransitionStatus};

//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//		int32_t fail_ret354{static_cast<int32_t>(status)};		/* 通知パラメータがAPI仕様記載外の異常値 */
//		videohal::CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-N-354",fail_ret354);
//		pfm_str_transition_status_t* p_status{const_cast<pfm_str_transition_status_t*>(&status)};
//		*p_status = static_cast<pfm_str_transition_status_t>(fail_ret354);
//		int32_t fail_ret355{0};		/* PFMからのPFM_STR_STS_SUSPEND_START通知無し */
//		videohal::CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-N-355",fail_ret355);
//		if ((0 != fail_ret355) && (PFM_STR_STS_SUSPEND_START == status))
//		{
//			VHAL_LOGW("notify dropped. (failsafe-355)");
//			return;		/* 通知を無かったことにする */
//		}
//		int32_t fail_ret356{0};		/* PFMからのPFM_STR_STS_RESUME_START通知無し */
//		videohal::CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-N-356",fail_ret356);
//		if ((0 != fail_ret356) && (PFM_STR_STS_RESUME_START == status))
//		{
//			VHAL_LOGW("notify dropped. (failsafe-356)");
//			return;		/* 通知を無かったことにする */
//		}
//#endif

		VHAL_LOGI("p_kind->strTransitionStatus=%d", static_cast<int32_t>(status));

		/* STRサスペンド開始指示(非STR状態のみ有効) */
		if (!str_mode && (PFM_STR_STS_SUSPEND_START == status))
		{
			/* STR状態とする */
			videohal::CVhalStrManager::SetStrMode(true);

			/* STR受付処理(PFMスレッド：サスペンド) */
			int32_t ret{p_prop->StrStatePfmSuspend()};
			if (videohal::VHAL_SUCCESS != ret)
			{
				/* 失敗時はSTR状態を中断 */
				videohal::CVhalStrManager::SetStrMode(false);
			}
		}
		/* STRレジューム開始指示(STR状態のみ有効) */
		else if (str_mode && (PFM_STR_STS_RESUME_START == status))
		{
			/* STR受付処理(PFMスレッド：レジューム) */
			p_prop->StrStatePfmResume();
			/* 非STR状態とする */
			videohal::CVhalStrManager::SetStrMode(false);
		}
		else
		{
			/* 無処理 */
		}
	}
}

/*****************************************************************************
                          PF Module登録オブジェクト
*****************************************************************************/
/* 各イベント時に呼び出される関数を登録 */
/* pre_initialize       :コールドスタート(スナップショットイメージ取得前) */
/* initialize           :ホットスタート(スナップショットイメージ取得後) */
/* clear_data           :メモリクリア処理 */
/* show_dbginfo         :デバッグ情報表示 */
/* shutdown             :シャットダウン処理 */
/* terminate            :終了処理 */
/* power_state          :電源状態遷移処理 */
/* vehicle_power_state  :車両電源ステート処理 */
/* str_state            :STR(SuspendToRAM)処理 */
pfm_ops_t ops_list{
	.pre_initialize			= &vhal_pfm_pre_initialize,
	.initialize				= &vhal_pfm_initialize,
	.clear_data				= &vhal_pfm_clear_data,
	.show_dbginfo			= &vhal_pfm_show_dbginfo,
	.shutdown				= &vhal_pfm_shutdown,
	.terminate				= &vhal_pfm_terminate,
	.power_state			= &vhal_pfm_power_state,
	.vehicle_power_state	= &vhal_pfm_vehicle_power_state,
	.str_state				= &vhal_pfm_str_state,
};

STRING VHAL_PFM_NAME[]{"VHAL"};
STRING VHAL_PFM_DESC[]{"This process is videohal PFM Client."};
pfm_obj_t vhal_pfm_obj{
	.name = VHAL_PFM_NAME,							/* モジュール識別ID(ASCII4文字固定)	*/
	.desc = VHAL_PFM_DESC,							/* 説明 */
	.flags = static_cast<UINT32>(PFM_FLAG_NOHIB),	/* 動作変更フラグ 															*/
													/* PFM_FLAG_SERVER: サーバとして動作(デフォルトはクライアント)				*/
													/* PFM_FLAG_NOHIB: ハイバネーション(suspend用一時停止処理)を無効化 			*/
													/* PFM_FLAG_NOSIG: シグナル(sif_syncsig)をPFM内部で設定しない(自前で設定)	*/
	.res = {										/* リソース定義			*/
		.ops_num = 1U,								/* 関数エントリリスト数	*/
		.ops_list = &ops_list,						/* 関数エントリリスト	*/
		.thread_num = 0U,							/* (未使用)スレッド属性リスト数 */
		.thread_list = nullptr,						/* (未使用)スレッド属性(sif_thread_attr_t)リスト */
		.thrmsg_num = 0U,							/* (未使用)スレッド間メッセージ属性リスト数 */
		.thrmsg_list = nullptr,						/* (未使用)スレッド間メッセージ属性(sif_thrmsgq_attr_t)リスト */
		.pkgcom_num = 0U,							/* (未使用)PkgCom属性リスト数 */
		.pkgcom_list = nullptr,						/* (未使用)PkgCom属性(UtlPkgComAttrInternalFifo)リスト */
		.sig_num = 0U,								/* (未使用)モジュールで使用するシグナル数 */
		.sig_list = nullptr							/* (未使用)モジュールで使用するシグナルリスト */
	},
	.priv = nullptr									/* 個別データ（拡張用） */
};

/*****************************************************************************
 処理概要：	PFM初期化エントリ
 引数    ：	なし
 戻り値  ：	なし
 フェールセーフNo  ：	F-VHAL-C-153
*****************************************************************************/
void vhal_pfm_probe(void)
{
	pfm_ret_t ret{PFM_OK};

	/* PF Module登録 */
	ret = pfm_register(&vhal_pfm_obj);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	int32_t fail_ret{0};
//	bool fail{videohal::CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-C-153",fail_ret)};
//	if(true == fail)
//	{
//		ret = static_cast<pfm_ret_t>(fail_ret);
//	}
//#endif
	if (PFM_OK != ret)
	{
		VHAL_LOGE("pfm_register error. ret=%d", ret);
	}
}

} /* namespace */

/*****************************************************************************
                          << 初期化エントリ登録 >>
*****************************************************************************/
PFM_ENTRY(&vhal_pfm_probe);		/* 登録した関数は、pfm_activate実施時に実行される */

namespace videohal
{

/*****************************************************************************
 処理概要：	VideoHALメイン関数（スレッド実行対応用）
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t vhal_main(void)
{
	/* STR状態管理クラス(シングルトン)の生成 */
	(void)CVhalStrManager::GetInstance();

	/* PF Module起動処理(pre_initialize/initialize処理) */
	pfm_activate();

	const int32_t ret{g_main_control.RunEventLoop()};
	if (videohal::VHAL_SUCCESS != ret)
	{
		VHAL_LOGW("RunEventLoop error. ret=%d", ret);
	}

	g_main_control.Finalize();

	return ret;
}

} /* namespace videohal */

