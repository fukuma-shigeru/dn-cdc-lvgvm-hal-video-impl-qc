/*******************************************************************************
    機能名称    ：  FileObserver制御モジュール
    ファイル名称：  vhal_fileobserver_control.cpp
*******************************************************************************/
#include "vhal_fileobserver_control.h"

#include "vhal_define.h"
#include "vhal_log.h"
#include "vhal_round_cast.h"
#include "vhal_main_control.h"
#include "vhal_fileobserver_receive_event_item.h"
#include "vhal_capture_control.h"
#include "vhal_str_mng.h"
#include "vhal_debug_system.h"
extern "C"
{
#include "sif_util.h"
}

namespace videohal
{
namespace
{
/* File System HDMIプロパティパス */
const std::string kPropertyHdmiPath{"/run/arene/vehicle_fs/var/system/connection/front_hdmi"};
/* File System カメラ制御権プロパティパス */
const std::string kPropertyCamDispStatusPath{"/run/arene/vehicle_fs/var/system/camera/display/status"};

}

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalFileObserverControl::CVhalFileObserverControl(void)
	:p_fileob_{nullptr}
	,p_main_{nullptr}
	,p_fileobserver_listener_{nullptr}
	,p_route_{nullptr}
	,fileob_id_list_{}
	,fileob_entries_{
		{
			{kPropertyCamDispStatusPath,	 {static_cast<uint32_t>(VhalCameraDisplayStatus::kInitial), [this] (const uint32_t value) { ExecCameraDisplayStatus(value); }}},
		}
	}
	,p_hdmi_observer_{nullptr}
	,p_observer_control_{nullptr}
	,p_prop_{nullptr}
	,hdmi_initialized_{false}
	,hdmi_state_current_{VhalHdmiState::kMax}
{
 	VHAL_LOGV("CVhalFileObserverControl is created. this=%p", this);
	p_fileobserver_control_ = this;
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalFileObserverControl::~CVhalFileObserverControl(void)
{
	VHAL_LOGV("CVhalFileObserverControl is deleted. this=%p", this);
	Finalize();
}

/*****************************************************************************
 処理概要：	初期化処理（内部リソースの確保）
 引数    ：	CVhalMainControl		*p_main_ctl				(i)メインコントロールインスタンスポインタ
           	CVhalObserverControl	*p_observer_ctl			(i)状態監視制御インスタンスポインタ
 戻り値  ：	処理結果
           		VHAL_ERR_****		異常終了
           		VHAL_SUCCESS		正常終了
 フェールセーフNo  ：	F-VHAL-C-091
 						F-VHAL-R-092
*****************************************************************************/
int32_t CVhalFileObserverControl::Initialize(CVhalMainControl* const p_main_ctl, CVhalObserverControl * const p_observer_ctl)
{
	int32_t ret{VHAL_SUCCESS};
	
	do
	{
		bool loop_end{false};
		if (nullptr == p_main_ctl)
		{
			VHAL_LOGE("Invalid params. p_main=%p", p_main_ctl);
			ret = VHAL_ERR_PARAM;
			loop_end = true;
		}
		else
		{
			p_main_ = p_main_ctl;
			
			if (nullptr == p_observer_ctl)
			{
				VHAL_LOGE("Invalid params. p_observer_control=%p", p_observer_ctl);
				ret = VHAL_ERR_PARAM;
				loop_end = true;
			}
			else
			{
				p_observer_control_ = p_observer_ctl;

				p_route_ = std::make_unique<CVhalEventRoute>();
				if (nullptr == p_route_)
				{
					ret = VHAL_ERR_MEMORY;
					VHAL_LOGE("new CVhalEventRoute null.");
					loop_end = true;
				}
				else
				{
					ret = p_route_->Initialize();
					if (VHAL_SUCCESS != ret)
					{
						VHAL_LOGE("CVhalEventRoute Initialize error. ret=%d", ret);
						loop_end = true;
					}
					else
					{
						ret = p_main_->RegisterEventSource(p_route_.get());
						if (VHAL_SUCCESS != ret)
						{
							VHAL_LOGE("RegisterEventSource error. ret=%d", ret);
							loop_end = true;
						}
						else
						{
							p_fileob_ = cockpit::bs::CFileObserver::GetInstance();
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//							int32_t fail_ret{0};
//							bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-C-091",fail_ret)};
//							if(true == fail)
//							{
//								p_fileob_ = nullptr;
//							}
//#endif
							if( nullptr == p_fileob_ )
							{
								ret = VHAL_ERR;
								VHAL_LOGE("CFileObserver::GetInstance is nullptr.");
								loop_end = true;
							}
							else
							{
								/* HDMI接続状態更新監視クラス初期化 */
								p_hdmi_observer_ = std::make_unique<CVhalFileObserverHdmiObserver>();
								if (nullptr == p_hdmi_observer_)
								{
									ret = VHAL_ERR_MEMORY;
									VHAL_LOGE("new CVhalFileObserverHdmiObserver null.");
									loop_end = true;
								}
								else
								{
									p_hdmi_observer_->Initialize(this);
								}
							}
						}
					}
				}
			}
		}
		if (true == loop_end)
		{
			break;
		}

		/* カメラ制御権取得 */
		uint32_t	value{0U};
		ret = GetStatus(kPropertyCamDispStatusPath, value, kRetryMaxStartup, kRetryWaitStartup);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("GetStatus error. ret=%d property=%s", ret, kPropertyCamDispStatusPath.c_str());
			/* カメラ制御権Initial状態で処理継続 */
			ret = VHAL_SUCCESS;
		}
		else
		{
			/* 取得値に更新 */
			(void)UpdateStatus(kPropertyCamDispStatusPath, value);
		}

		/* 監視対象ファイルパスを登録 */
		std::vector<std::string> monitor_path_list{};
		for (auto itr_fileob = fileob_entries_.begin(); itr_fileob != fileob_entries_.end(); (void)++itr_fileob)
		{
			if (false == itr_fileob->first.empty())
			{
				monitor_path_list.push_back(itr_fileob->first);
			}
		}
		if (false == monitor_path_list.empty())
		{
			fileob_id_list_ = p_fileob_->RegisterListener([this](const std::string s) { FileobPathCallback(s); }, monitor_path_list);
			for (uint32_t i{0U}; i < fileob_id_list_.size(); (void)++i)
			{
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//				CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-R-092",fileob_id_list_[i]);
//#endif
				if ((p_fileob_->FILEOB_RET_FAILURE == fileob_id_list_[i]) || (p_fileob_->FILEOB_RET_NOENTRY == fileob_id_list_[i]))
				{
					VHAL_LOGE("CFileObserver::RegisterListener() error. path=%s, ret=%d", monitor_path_list[i].c_str(), fileob_id_list_[i]);
					/* 失敗しても処理継続（上位にエラーを返さない。初期化が中断されないようにする） */
				}
			}
		}
	} while (false);
	
	return ret;
}

/*****************************************************************************
 処理概要：	終了処理（内部リソースの解放）
 引数    ：	なし
 戻り値  ：	なし
 フェールセーフNo  ：	F-VHAL-N-093
*****************************************************************************/
void CVhalFileObserverControl::Finalize(void)
{
	if (nullptr != p_fileob_)
	{
		/* 監視対象ファイルパスを解除 */
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//		std::vector<int32_t> fileob_id_ret{p_fileob_->UnRegisterListener(fileob_id_list_)};
//#else
		const std::vector<int32_t> fileob_id_ret{p_fileob_->UnRegisterListener(fileob_id_list_)};
//#endif
		for (uint32_t i{0U}; i < fileob_id_ret.size(); (void)++i)
		{
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//			int32_t fail_ret{0};
//			bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-093",fail_ret)};
//			if(true == fail)
//			{
//				fileob_id_ret[i] = fail_ret;
//			}
//#endif
			if ((p_fileob_->FILEOB_RET_FAILURE == fileob_id_ret[i]) || (p_fileob_->FILEOB_RET_NOENTRY == fileob_id_ret[i]))
			{
				VHAL_LOGE("CFileObserver::UnRegisterListener() error. ret[%d]=%d", i, fileob_id_ret[i]);
			}
		}

		p_fileob_ = nullptr;
	}
	
	if (nullptr != p_observer_control_)
	{
		const bool is_running{p_observer_control_->IsRunningObserver(p_hdmi_observer_.get())};
		if (true == is_running)
		{
			/* 監視削除 */
			p_observer_control_->ClearObserver(p_hdmi_observer_.get());
		}
		p_observer_control_ = nullptr;
	}

	if (nullptr != p_route_)
	{
		p_main_->ClearEventSource(p_route_.get());
		p_route_ = nullptr;
	}
}

/*****************************************************************************
 処理概要：	プロパティ更新処理（HDMI接続状態）
 引数    ：	const bool	 check_running 		(i)監視チェック有無
           	const VhalHdmiState hdmi_state	(i)HDMI接続状態
           	const uint32_t retry_num 		(i)アクセスリトライ回数
           	const uint32_t retry_wait 		(i)アクセスリトライウェイト(ms)
 戻り値  ：	処理結果
           		VHAL_ERR_****		異常終了
           		VHAL_SUCCESS		正常終了
 フェールセーフNo  ：	F-VHAL-N-100
                    	F-VHAL-R-101
                    	F-VHAL-N-102
*****************************************************************************/
int32_t CVhalFileObserverControl::UpdatePropertyHdmi(const bool check_running , const VhalHdmiState hdmi_state, const uint32_t retry_num, const uint32_t retry_wait)
{
	int32_t result{VHAL_ERR};
	if( nullptr != p_fileob_ )
	{
		if( hdmi_state != hdmi_state_current_ )
		{
			if( ( VhalHdmiState::kDisconnect <= hdmi_state ) && ( VhalHdmiState::kMax > hdmi_state ) )
			{
				if (false == hdmi_initialized_)
				{
					/* HDMI接続状態保持 */
					p_hdmi_observer_->SetHdmiState(hdmi_state);
				}
				int32_t fileob_ret{p_fileob_->FILEOB_RET_SUCCESS};
				uint32_t count{0U};
				while( count < retry_num )
				{
					++count;
					const uint32_t state{static_cast<uint32_t>(hdmi_state)};
					fileob_ret = p_fileob_->Write(kPropertyHdmiPath, static_cast<const void*>(&state), UI64ToI32(sizeof(state)));
					if( p_fileob_->FILEOB_RET_SUCCESS == fileob_ret )
					{
						break;
					}
					sif_mdelay(retry_wait);
				}

//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//				int32_t fail_ret{0};
//				bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-R-101",fail_ret)};
//				if(true == fail)
//				{
//					fileob_ret = fail_ret;
//					count = retry_num;
//				}
//#endif
				bool is_running{true};
				if (true == check_running)
				{
					/* HDMI接続状態監視状態取得 */
					is_running = p_observer_control_->IsRunningObserver(p_hdmi_observer_.get());
				}

				if ( p_fileob_->FILEOB_RET_SUCCESS == fileob_ret )
				{
					VHAL_LOGI("Write property=%s state=%d count=%d/%d fileob_ret=%d", kPropertyHdmiPath.c_str(), hdmi_state, count, retry_num, fileob_ret);
					result = VHAL_SUCCESS;
					hdmi_state_current_ = hdmi_state;
					if (false == hdmi_initialized_)
					{
						/* HDMI接続状態初回書き込み済み */
						hdmi_initialized_ = true;
					}
				
					if (true == is_running)
					{
						/* 監視削除 */
						p_observer_control_->ClearObserver(p_hdmi_observer_.get());
					}
				}
				else
				{
					VHAL_LOGE("Write property=%s state=%d count=%d/%d fileob_ret=%d", kPropertyHdmiPath.c_str(), hdmi_state, count, retry_num, fileob_ret);
					if ((false == is_running) && (false == hdmi_initialized_))
					{
						constexpr int64_t kObserverHdmiUpdate{5000};	/* HDMI更新監視周期時間(ms) */
						const int32_t ret{p_observer_control_->RegistryObserver(p_hdmi_observer_.get(), kObserverHdmiUpdate)};
						if (VHAL_SUCCESS != ret)
						{
							VHAL_LOGE("CVhalObserverControl::RegistryObserver() error. ret=%d", ret);
						}
					}
				}
			}
			else
			{
				VHAL_LOGE("Illegal HDMI state=%d", hdmi_state);
			}
		}
		else
		{
			VHAL_LOGD("HDMI connect status value(%d) is not changed.", hdmi_state);
			result = VHAL_SUCCESS;

			if (false == hdmi_initialized_)
			{
				/* HDMI接続状態初回書き込み済み */
				hdmi_initialized_ = true;
			}
		}
	}
	else
	{
		VHAL_LOGE("FileObserver nullptr");
	}
	
	return result;
}

/*****************************************************************************
 処理概要：	プロパティ取得処理(HDMI接続状態)
 引数    ：	VhalHdmiState &hdmi_state (o)HDMI接続状態読み込み値
 戻り値  ：	処理結果
           	VHAL_ERR			異常終了
           	VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalFileObserverControl::GetPropertyHdmi(VhalHdmiState &hdmi_state) const
{
	uint32_t		value{0U};
	int32_t	ret{GetStatus(kPropertyHdmiPath, value)};
	if( VHAL_SUCCESS == ret )
	{
		/* データ有効チェック */
		if (static_cast<uint32_t>(VhalHdmiState::kMax) <= value)
		{
			VHAL_LOGE("Property Read data error. value=%d property=%s", value, kPropertyHdmiPath.c_str());
			ret = VHAL_ERR;
		}
		else
		{
			hdmi_state = static_cast<VhalHdmiState>(value);
		}
	}

	return ret;
}

/*****************************************************************************
 処理概要：	カメラ制御権取得
 引数    ：	VhalCameraDisplayStatus &current_cam_disp_status (o)カメラ制御権状態読み込み値
 戻り値  ：	処理結果
           	VHAL_ERR			異常終了
           	VHAL_SUCCESS		正常終了
 フェールセーフNo  ：	F-VHAL-N-097
*****************************************************************************/
int32_t CVhalFileObserverControl::GetCameraDisplayStatus(VhalCameraDisplayStatus &current_cam_disp_status)
{
	int32_t	ret{VHAL_SUCCESS};

	const auto itr_fileob = fileob_entries_.find(kPropertyCamDispStatusPath);
	if (itr_fileob == fileob_entries_.end())
	{
		VHAL_LOGE("Unmatch error. property=%s", kPropertyCamDispStatusPath.c_str());
		ret = VHAL_ERR;
	}
	else
	{
		VhalFileObserverEntry &entry{itr_fileob->second};
		/* データ有効チェック */
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//		uint32_t fileob_ret{0U};
//		int32_t fail_ret{0};
//		bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-097",fail_ret)};
//		if(true == fail)
//		{
//			fileob_ret = static_cast<uint32_t>(fail_ret);
//		}
//		else
//		{
//			fileob_ret = entry.GetValue();
//		}
//		if (static_cast<uint32_t>(VhalCameraDisplayStatus::kMax) <= fileob_ret)
//#else
		if (static_cast<uint32_t>(VhalCameraDisplayStatus::kMax) <= entry.GetValue())
//#endif
		{
			VHAL_LOGE("Property Read data error. value=%d property=%s", entry.GetValue(), kPropertyCamDispStatusPath.c_str());
			ret = VHAL_ERR;
		}
		else
		{
			current_cam_disp_status = static_cast<VhalCameraDisplayStatus>(entry.GetValue());
		}
	}

	return ret;
}

/*****************************************************************************
 処理概要：	対象ファイルパス値の取得
 引数    ：	const std::string&	monitor_path	(i)対象ファイルパス
           	uint32_t&			value			(o)読み込み値
           	const uint32_t retry_num 			(i)アクセスリトライ回数
           	const uint32_t retry_wait 			(i)アクセスリトライウェイト(ms)
 戻り値  ：	処理結果
           	VHAL_ERR			異常終了
           	VHAL_SUCCESS		正常終了
 フェールセーフNo  ：	F-VHAL-N-094
                     	F-VHAL-R-095
                     	F-VHAL-N-096
                     	F-VHAL-N-098
*****************************************************************************/
int32_t CVhalFileObserverControl::GetStatus(const std::string& monitor_path, uint32_t &value, const uint32_t retry_num, const uint32_t retry_wait) const
{
	int32_t ret{VHAL_SUCCESS};
	int32_t fileob_ret{p_fileob_->FILEOB_RET_SUCCESS};
	uint32_t count{0U};

	while( count < retry_num )
	{
		++count;
		fileob_ret = p_fileob_->Read(monitor_path, static_cast<void*>(&value), static_cast<int32_t>(sizeof(value)));
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//		int32_t fail_ret{0};
//		bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-R-095",fail_ret)};
//		if(true == fail)
//		{
//			fileob_ret = fail_ret;
//		}
//#endif
		if( p_fileob_->FILEOB_RET_SUCCESS == fileob_ret )
		{
			ret = VHAL_SUCCESS;
			break;
		}
		else
		{
			ret = VHAL_ERR;
			sif_mdelay(retry_wait);
		}
	}

	if ( VHAL_SUCCESS == ret )
	{
		VHAL_LOGI("Read property=%s state=%d count=%d/%d fileob_ret=%d", monitor_path.c_str(), value, count, retry_num, fileob_ret);
	}
	else
	{
		VHAL_LOGE("Read property=%s state=%d count=%d/%d fileob_ret=%d", monitor_path.c_str(), value, count, retry_num, fileob_ret);
	}

	return ret;
}

/*****************************************************************************
 処理概要：	状態更新
 引数    ：	const std::string&	monitor_path	(i)対象ファイルパス
           	const uint32_t		value			(i)状態値
 戻り値  ：	処理結果
           	false			更新なし
           	true			更新あり(管理テーブル値を更新)
*****************************************************************************/
bool CVhalFileObserverControl::UpdateStatus(const std::string& monitor_path, const uint32_t value)
{
	bool	ret{true};

	do
	{
		const auto itr_fileob = fileob_entries_.find(monitor_path);
		if (itr_fileob == fileob_entries_.end())
		{
			VHAL_LOGE("Unmatch error. property=%s", monitor_path.c_str());
			ret = false;
			break;
		}

		VhalFileObserverEntry &entry{itr_fileob->second};
		/* データ有効チェック */
		if ((0 == monitor_path.compare(kPropertyCamDispStatusPath)) && (static_cast<uint32_t>(VhalCameraDisplayStatus::kMax) <= value))
		{
			VHAL_LOGW("Property Read data Unknown. value=%d property=%s", value, monitor_path.c_str());
		}

		/* データ更新チェック */
		if (entry.GetValue() != value)
		{
			VHAL_LOGI("%s=%d -> %d", monitor_path.c_str(), entry.GetValue(), value);
			entry.SetValue(value);
		}
		else
		{
			ret = false;
		}
	} while (false);

	return ret;
}

/*****************************************************************************
 処理概要：	FileObserver対象のイベント処理の実行（カメラ制御権）
 引数    ：	uint32_t	value			(i)読み込み値
 戻り値  ：	なし
*****************************************************************************/
void CVhalFileObserverControl::ExecCameraDisplayStatus(const uint32_t value)
{
	/* 設定値変化の場合 */
	if (true == UpdateStatus(kPropertyCamDispStatusPath, value))
	{
		if (nullptr != p_fileobserver_listener_)
		{
			if (static_cast<uint32_t>(VhalCameraDisplayStatus::kMax) > value)
			{
				p_fileobserver_listener_->NotifyReceiveCameraDisplayStatus(static_cast<VhalCameraDisplayStatus>(value));
			}
			else
			{
				VHAL_LOGE("Invalid params. value=%u", value);
			}
		}
		else
		{
			VHAL_LOGE("Invalid params. p_fileobserver_listener_=%p", p_fileobserver_listener_);
		}
	}
}

/*****************************************************************************
 処理概要：	FileObserver対象のイベント処理の実行
 引数    ：	std::string	monitor_path			(i)監視対象ファイルパス
           	uint32_t	value					(i)読み込み値
 戻り値  ：	なし
*****************************************************************************/
void CVhalFileObserverControl::ExecFileObserverEvent(const std::string& monitor_path, const uint32_t value)
{
	VHAL_LOGD("path=%s", monitor_path.c_str());

	/* 監視対象ファイルパスチェック */
	const auto itr_fileob = fileob_entries_.find(monitor_path);
	if (itr_fileob == fileob_entries_.end())
	{
		VHAL_LOGE("Unmatch error. property=%s", monitor_path.c_str());
	}
	else
	{
		VhalFileObserverEntry &entry{itr_fileob->second};
		entry.Action(value);
	}
}

/*****************************************************************************
 処理概要：	監視対象ファイルパスのコールバック通知
 引数    ：	std::string monitor_path			(i)監視対象ファイルパス
 戻り値  ：	なし
*****************************************************************************/
void CVhalFileObserverControl::FileobPathCallback(const std::string monitor_path)
{
	VHAL_LOGV("path=%s", monitor_path.c_str());

	/* サスペンド状態の場合は無処理 */
	if (videohal::CVhalStrManager::GetSuspend())
	{
		VHAL_LOGV("NOP callback.");
		return;
	}

	const auto itr_fileob = fileob_entries_.find(monitor_path);
	if (itr_fileob != fileob_entries_.end())
	{
		int32_t		ret{VHAL_SUCCESS};
		uint32_t	value{0U};
		ret = GetStatus(monitor_path, value);
		if (VHAL_SUCCESS == ret)
		{
			/* VideoHALイベントシステムにイベント(処理結果)を自投げ */
			std::unique_ptr<CVhalFileObserverReceiveEventItem> p_fileobserver_event{std::make_unique<CVhalFileObserverReceiveEventItem>()};
			VHAL_LOGD("fileobserver event. path=%s", monitor_path.c_str());
			p_fileobserver_event->SetName("fileobserver event");
			p_fileobserver_event->SetData(p_fileobserver_control_, monitor_path, value);
			ret = p_route_->WriteEvent(p_fileobserver_event.get());
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGEW("WriteEvent(p_fileobserver_event) error. ret=%d", ret);
			}
			else
			{
				/* 所有権を放棄。実体解放はCVhalEventRoute::ExecEvent()にて実施 */
				(void)p_fileobserver_event.release();
			}
		}
		else
		{
			VHAL_LOGE("GetStatus error. ret=%d property=%s", ret, monitor_path.c_str());
		}

	}
}

/*****************************************************************************
 処理概要：	イベントリスナー登録
 引数    ：	CVhalFileObserverEventListenerBase* p_listener		(i)リスナーポインタ
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM			パラメータ不正
           		VHAL_SUCCESS			正常終了
****************************************************************************/
int32_t CVhalFileObserverControl::RegisterEventListener(CVhalFileObserverEventListenerBase* const p_listener)
{
	int32_t	result{VHAL_SUCCESS};

	if (nullptr == p_listener)
	{
		VHAL_LOGE("p_listener is null");
		result = VHAL_ERR_PARAM;
	}
	else
	{
		p_fileobserver_listener_ = p_listener;
	}

	return result;
}

/*****************************************************************************
 処理概要：	イベントリスナー解除
 引数    ：	なし
 戻り値  ：	なし
****************************************************************************/
void CVhalFileObserverControl::ClearEventListener(void) noexcept
{
	p_fileobserver_listener_ = nullptr;
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalFileObserverHdmiObserver::~CVhalFileObserverHdmiObserver(void)
{
	Finalize();
}

/*****************************************************************************
 処理概要：	初期化
 引数    ：	CVhalFileObserverControl*	p_file_observer_ctl	(i)FileObserver制御インスタンスポインタ
 戻り値  ：	なし
*****************************************************************************/
void CVhalFileObserverHdmiObserver::Initialize(CVhalFileObserverControl* const p_file_observer_ctl) noexcept
{
	p_file_observer_  = p_file_observer_ctl;
}

/*****************************************************************************
 処理概要：	終了処理
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalFileObserverHdmiObserver::Finalize(void) noexcept
{
	p_file_observer_  = nullptr;
}

/*****************************************************************************
 処理概要：	HDMI接続状態の設定
 引数    ：	VhalHdmiState state	(i)HDMI接続状態
 戻り値  ：	なし
*****************************************************************************/
void CVhalFileObserverHdmiObserver::SetHdmiState(const VhalHdmiState state) noexcept
{
	hdmi_state_  = state;
}

/*****************************************************************************
 処理概要：	HDMI接続状態更新監視周期コールバック
 引数    ：	なし
 戻り値  ：	処理結果
				VHAL_ERR			異常終了
				VHAL_SUCCESS		正常終了
 フェールセーフNo：	F-VHAL-N-102
*****************************************************************************/
int32_t CVhalFileObserverHdmiObserver::Notify(void)
{
	/* HDMI接続状態更新有無確認 */
	int32_t result{VHAL_ERR};
	if (nullptr != p_file_observer_)
	{
		VhalHdmiState current_hdmi_state{VhalHdmiState::kMax};
		result = p_file_observer_->GetPropertyHdmi(current_hdmi_state);
		if (VHAL_SUCCESS == result)
		{
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//			int32_t fail_ret{0};
//			bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-R-101",fail_ret)};
//			if(true == fail)
//			{
//				hdmi_state_ = VhalHdmiState::kUnknown;
//			}
//#endif
			if (current_hdmi_state != hdmi_state_)
			{
				/* 保持していたHDMI接続状態で更新 */
				result = p_file_observer_->UpdatePropertyHdmi(false, hdmi_state_, CVhalFileObserverControl::kRetryMaxNormal, CVhalFileObserverControl::kRetryWaitNormal);
				if (VHAL_SUCCESS != result)
				{
					VHAL_LOGE("UpdatePropertyHdmi error. state=%d, result=%d", hdmi_state_, result);
				}
			}
			else
			{
				/* HDMI接続状態が同一の為、監視登録を削除する。 */
				result = VHAL_SUCCESS;
				p_file_observer_->SetHdmiInitialized(true);
			}
		}
	}

	int32_t ret{VHAL_SUCCESS};
	if (VHAL_SUCCESS == result)
	{
		/* HDMI接続状態のWriteに成功した場合、割り込み監視を即終了するため、負の値を返す */
		ret = VHAL_ERR;
	}
	return ret;
}

} /* namespace videohal */

