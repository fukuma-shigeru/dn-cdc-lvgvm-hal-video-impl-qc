/*******************************************************************************
    機能名称    ：  SysDB制御モジュール
    ファイル名称：  vhal_sysdb_control.cpp
*******************************************************************************/
#include "vhal_sysdb_control.h"

#include "vhal_define.h"
#include "vhal_log.h"
#include "vhal_round_cast.h"
#include "vhal_main_control.h"
#include "vhal_property_control.h"
#include "vhal_sysdb_event_item.h"
#include "vhal_str_mng.h"
#include "vhal_debug_system.h"

extern "C"
{
#include <sys_db_cnv_value_public.h>
}

/*****************************************************************************
*	define																	 *
*****************************************************************************/

/*****************************************************************************
*	globals																	 *
*****************************************************************************/

/*****************************************************************************
*	statics																	 *
*****************************************************************************/

namespace videohal
{
namespace 
{
CVhalSysdbControl *p_sysdbctrl_{nullptr};
}

std::unordered_map<uint32_t, CVhalSysdbControl::hdmiVideoFormat> CVhalSysdbControl::hdmi_videoformat_list_
{
	{SYSINFO_CONN_INFO_VIDEO_FORMAT_1280X720P,	 { 1280U,  720U }},
	{SYSINFO_CONN_INFO_VIDEO_FORMAT_1920X1080P,  { 1920U, 1080U }},
	{SYSINFO_CONN_INFO_VIDEO_FORMAT_720X480P,	 {  720U,  480U }},
	{SYSINFO_CONN_INFO_VIDEO_FORMAT_640X480P,    {  640U,  480U }}
};

/*****************************************************************************
 処理概要：	イベント配信コールバック関数
 引数    ：	SysEvMngComEveHeadInf*	comHead		(i)イベントヘッダ構造体への先頭ポインタ
           	int8_t*					eveDat		(i)データヘッダポインタ
 戻り値  ：	なし
*****************************************************************************/
void CVhalDbFileEventCb(SysEvMngComEveHeadInf* const comHead, int8_t* const eveDat)
{
	SysDbEvtDataHeader* head;

	VHAL_LOGV_IN();

	/* サスペンド状態の場合は無処理 */
	if (videohal::CVhalStrManager::GetSuspend())
	{
		VHAL_LOGV("NOP callback.");
		return;
	}

	/* パラメータチェック */
	if ((nullptr == comHead) || (nullptr == eveDat))
	{
		VHAL_LOGE("parameter is null.(comHead=%p, eveDat=%p)", comHead, eveDat);
		return;
	}
	void* const evtHeader{eveDat};
	head = static_cast<SysDbEvtDataHeader*>(evtHeader);

	p_sysdbctrl_->NotifySysdbEvent(head);

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalSysdbControl::CVhalSysdbControl(void)
	:p_main_(nullptr)
	,p_sysdb_listener_(nullptr)
	,p_route_(nullptr)
	,clientObj_{}
	,evMngObj_{}
	,sysdbevent_entries_{
	{sysdbEventType::VHAL_SYSDB_EVENT_VMSTR_CHANGE,	 {0U, VHAL_SYSDB_PATH_VMSTR_CHANGE, [this] (void) { ActionVmStrChange(); }}}}
	,sysdb_open_status(false)
	,p_init_thread_(nullptr)
{
 	VHAL_LOGV("CVhalSysdbControl is created. this=%p", this);
 	p_sysdbctrl_ = this;
	sysdb_write_list_.clear();
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalSysdbControl::~CVhalSysdbControl(void)
{
	VHAL_LOGV("CVhalSysdbControl is deleted. this=%p", this);
	Finalize();
}

/*****************************************************************************
 処理概要：	初期化処理（内部リソースの確保）
 引数    ：	CVhalMainControl		*p_main_ctl		(i)メインコントロールインスタンスポインタ
 戻り値  ：	処理結果
           		VHAL_ERR_****		エラー
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalSysdbControl::Initialize(CVhalMainControl * const p_main_ctl)
{
	int32_t ret{VHAL_SUCCESS};

	if (nullptr == p_main_ctl)
	{
		ret = VHAL_ERR_PARAM;
	}
	else
	{
		p_main_ = p_main_ctl;

		p_route_ = std::make_unique<CVhalEventRoute>();
		ret = p_route_->Initialize();
		if (0 > ret)
		{
			VHAL_LOGE("EventRoute Initialize error. ret=%d", ret);
		}
		else
		{
			ret = p_main_->RegisterEventSource(p_route_.get());
			if (0 > ret)
			{
				VHAL_LOGE("RegisterEventSource error. ret=%d", ret);
			}
			else
			{
				/* SysDB初期化時に書き込む値を設定 */
				AddSysdbInfoList(VHAL_SYSDB_PATH_CONN_INFO_HDMI, DAT_CNV_SYSINFO_CONN_INFO_STAT_CONNECTING);	/* 接続判定中 */
				AddSysdbInfoList(VHAL_SYSDB_PATH_HDMI_VIDEO_FORMAT, SYSINFO_CONN_INFO_VIDEO_FORMAT_INVALID);	/* 未確定 */
				AddSysdbInfoList(VHAL_SYSDB_PATH_HDMI_AUDIO_FORMAT, SYSINFO_CONN_INFO_AUDIO_FORMAT_INVALID);	/* 未確定 */

				/* SysDB初期化処理 */
				/* １度トライして、エラーが発生すれば非同期で初期化リトライ */
				struct SysdbInitStatus init_status{};
				init_status.ul_func_id_        = false;
				init_status.client_open_       = false;
				init_status.dv_event_id_       = false;
				init_status.evmng_open_        = false;
				init_status.delv_clbk_         = false;
				init_status.count_ul_func_id_  = 1U;
				init_status.count_client_open_ = 1U;
				init_status.count_dv_event_id_ = 1U;
				init_status.count_evmng_open_  = 1U;
				init_status.count_delv_clbk_   = 1U;

				ret = OpenSysdb(init_status, false);
				if (VHAL_SUCCESS != ret)
				{
					constexpr uint32_t kVhalSysDbInitRetryCount{120U};	/* SysDB初期化のリトライ回数 */
					/* SysDB初期化リトライ */
					init_status.count_ul_func_id_  = kVhalSysDbInitRetryCount;
					init_status.count_client_open_ = kVhalSysDbInitRetryCount;
					init_status.count_dv_event_id_ = kVhalSysDbInitRetryCount;
					init_status.count_evmng_open_  = kVhalSysDbInitRetryCount;
					init_status.count_delv_clbk_   = kVhalSysDbInitRetryCount;

					p_init_thread_ = std::make_unique<CVhalSysdbInitThread>(this);
					if (nullptr != p_init_thread_ )
					{
						ret = p_init_thread_->Start();
						if (VHAL_SUCCESS == ret)
						{
							ret = p_init_thread_->Notify(init_status);
							if (VHAL_SUCCESS != ret)
							{
								VHAL_LOGE("CVhalSysdbInitThread::Notify error ret=%d", ret);
							}
						}
						else
						{
							VHAL_LOGE("CVhalSysdbInitThread::Start error ret=%d", ret);
						}
					}
					else
					{
						VHAL_LOGE("p_init_thread_ nullptr");
					}
				}
			}
		}
	}

	return ret;
}

/*****************************************************************************
 処理概要：	SysDB初期化スレッド実行処理
 引数    ：	const struct SysdbInitStatus& data		(i)SysDB初期化情報
	        const bool terminate					(i)処理強制終了(false:継続 true:終了)
 戻り値  ：	処理結果
				VHAL_ERR	本スレッドを終了させるためエラーのみ返す
*****************************************************************************/
int32_t CVhalSysdbControl::CVhalSysdbInitThread::Execute(const struct SysdbInitStatus& data, const bool terminate) const noexcept
{
	struct SysdbInitStatus status{data};
	const int32_t ret{p_sysdb_control_->OpenSysdb(status, terminate)};
	if(VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("OpenSysdb error ret=%d", ret);
	}

	/* 本スレッドを終了させるため必ずエラーを返す */
	return VHAL_ERR;
}

/*****************************************************************************
 処理概要：	終了処理（内部リソースの解放）
 引数    ：	なし
 戻り値  ：	なし
 フェールセーフNo  ：	F-VHAL-C-046
                    	F-VHAL-C-047
                     	F-VHAL-C-048
*****************************************************************************/
void CVhalSysdbControl::Finalize(void)
{
	int32_t ret{0};
	const std::lock_guard<std::mutex> lock_sysdb{mtx_sysdb_};

	/* Sysdb初期化スレッド削除 */
	if (nullptr != p_init_thread_ )
	{
		p_init_thread_->End();
		p_init_thread_ = nullptr;
	}

	/* イベント配信コールバック関数解放 */
	ret = SysEvMngUnregisterDelvClbk(&evMngObj_);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	int32_t fail_ret{0};
//	bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-C-046",fail_ret)};
//	if(true == fail)
//	{
//		ret = fail_ret;
//	}
//#endif
	if (SYS_EV_MNG_RET_OK != ret)
	{
		VHAL_LOGE("SysEvMngUnregisterDelvClbk() error. ret=%d", ret);
	}

	/* イベント管理オブジェクトクローズ */
	ret = SysEvMngClose(&evMngObj_);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	fail = CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-C-047",fail_ret);
//	if(true == fail)
//	{
//		ret = fail_ret;
//	}
//#endif
	if (SYS_EV_MNG_RET_OK != ret)
	{
		VHAL_LOGE("SysEvMngClose() error. ret=%d", ret);
	}

	/* SysDB(Client)クローズ */
	ret = SysDbClientClose(&clientObj_);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	fail = CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-C-048",fail_ret);
//	if(true == fail)
//	{
//		ret = fail_ret;
//	}
//#endif
	if (SYS_DB_SUCCESS != ret)
	{
		VHAL_LOGE("SysDbClientClose error. ret=%d", ret);
	}
	sysdb_open_status = false;

	if (nullptr != p_route_)
	{
		p_main_->ClearEventSource(p_route_.get());
		p_route_ = nullptr;
	}

	sysdb_write_list_.clear();
}

/*****************************************************************************
 処理概要：	SYSDBのRead処理
 引数    ：	const std::string&	sysdbPath		(i)SYSDBパス
           	void *				state			(o)Readした値
           	const std::size_t	length			(i)Readサイズ
 戻り値  ：	処理結果
           		VHAL_ERR_****		エラー
           		VHAL_SUCCESS		正常終了
 フェールセーフNo  ：	F-VHAL-R-049
*****************************************************************************/
int32_t CVhalSysdbControl::GetValue(const std::string& sysdbPath, void* const state, const std::size_t length)
{
	int32_t result{VHAL_SUCCESS};

	VHAL_LOGV_IN();

//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	int32_t fail_ret{0};
//	int32_t ret{SysDbRead(&clientObj_, reinterpret_cast<int8_t *>(const_cast<char *>(sysdbPath.c_str())), UI64ToUI32(length), state)};
//	bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-R-049",fail_ret)};
//	if(true == fail)
//	{
//		ret = fail_ret;
//	}
//#else
	int32_t const ret{SysDbRead(&clientObj_, reinterpret_cast<int8_t *>(const_cast<char *>(sysdbPath.c_str())), UI64ToUI32(length), state)};
//#endif
	if (SYS_DB_SUCCESS != ret)
	{
		VHAL_LOGE("SysDbRead error. ret=%d DbName=%s", ret, sysdbPath.c_str());
		result = VHAL_ERR_SYSDB_API;
	}

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	SYSDBのRead処理(32bit)
 引数    ：	const std::string&	sysdbPath		(i)SYSDBパス
           	int32_t*			state			(o)Readした値
 戻り値  ：	処理結果
           		VHAL_ERR_****		エラー
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalSysdbControl::GetValue(const std::string& sysdbPath, int32_t* const state)
{
	return GetValue(sysdbPath, state, sizeof(*state));
}

/*****************************************************************************
 処理概要：	SYSDBのWrite処理(Writeデータサイズ:int32_t)
 引数    ：	const std::string&	sysdbPath		(i)SYSDBパス
           	int32_t 			state			(i)Writeする値
 戻り値  ：	処理結果
           		VHAL_ERR_****		エラー
           		VHAL_SUCCESS		正常終了
 フェールセーフNo  ：	F-VHAL-R-054
*****************************************************************************/
int32_t CVhalSysdbControl::SetValue(const std::string &sysdbPath, int32_t state)
{
	int32_t result{VHAL_SUCCESS};
	VHAL_LOGV_IN();

	if (sysdbPath.empty())
	{
		VHAL_LOGE("parameter error. sysdbPath is empty");
		result = VHAL_ERR_PARAM;
	}
	else
	{
		int32_t ret{0};
		/* SysDB(Client)オープン状態チェック */
		if (false == sysdb_open_status)
		{
			/* SysDB書き込みデータを追加 */
			ret = AddSysdbInfoList(sysdbPath, state);
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("AddSysdbInfoList error. ret=%d", ret);
				result = VHAL_ERR;
			}
		}
		else
		{
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//			int32_t fail_ret{0};
//			bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-R-054",fail_ret)};
//			if(true == fail)
//			{
//				state = 0;
//			}
//#endif
			VHAL_LOGI("SysDbWrite DbName=%s, state=%d", sysdbPath.c_str(), state);
			ret = SysDbWrite(&clientObj_, reinterpret_cast<int8_t *>(const_cast<char *>(sysdbPath.c_str())), UI64ToUI32(sizeof(state)), &state);
			if (SYS_DB_SUCCESS != ret)
			{
				VHAL_LOGE("SysDbWrite error. ret=%d, DbName=%s, state=%d", ret, sysdbPath.c_str(), state);
				result = VHAL_ERR_SYSDB_API;
			}
		}
	}

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	SYSDBイベント通知
 引数    ：	sysdbEventType	evtcode		(i)イベント
 戻り値  ：	処理結果
           		VHAL_ERR_****		エラー
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalSysdbControl::NotifySysdbEvent(const sysdbEventType evtcode)
{
	int32_t	ret{VHAL_SUCCESS};

	VHAL_LOGV_IN();

	std::unique_ptr <CVhalSysdbEventItem>	p_sysdb_event{std::make_unique<CVhalSysdbEventItem>()};
	if (nullptr == p_sysdb_event)
	{
		VHAL_LOGE("Failed to create CVhalSysdbEventItem.");
		ret = VHAL_ERR;
	}
	else
	{
		p_sysdb_event->SetName(std::string("sysdb event"));
		p_sysdb_event->SetData(this, evtcode);
		ret = p_route_->WriteEvent(p_sysdb_event.get());
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGEW("WriteEvent() error. ret=%d", ret);
			p_sysdb_event = nullptr;
		}
		else
		{
			(void)p_sysdb_event.release();
		}
	}

	VHAL_LOGV_OUT();
	return ret;
}

/*****************************************************************************
 処理概要：	SYSDBイベント通知
 引数    ：	SysDbEvtDataHeader*	head		(i)データヘッダ
 戻り値  ：	なし
*****************************************************************************/
void CVhalSysdbControl::NotifySysdbEvent(const SysDbEvtDataHeader* const head)
{
	VHAL_LOGV_IN();

	for (auto itr_sysdb = sysdbevent_entries_.begin(); itr_sysdb != sysdbevent_entries_.end(); ++itr_sysdb)
	{
		VhalSysDbEventEntry	&entry{itr_sysdb->second};

		if (head->ulEvtId == entry.GetDbeveid())
		{
			sysdbEventType	evtcode{sysdbEventType::VHAL_SYSDB_EVENT_NONE};
			evtcode = itr_sysdb->first;

			const int32_t	ret{NotifySysdbEvent(evtcode)};
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGEW("NotifySysdbEvent() error. ret=%d", ret);
			}
			break;
		}
	}

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	SYSDB対象のイベント処理の実行
 引数    ：	sysdbEventType	evtcode		(i)イベント
 戻り値  ：	なし
*****************************************************************************/
void CVhalSysdbControl::ExecSysdbEvent(const sysdbEventType evtcode)
{
	VHAL_LOGV_IN();

	const auto itr_sysdb = sysdbevent_entries_.find(evtcode);
	if (itr_sysdb == sysdbevent_entries_.end())
	{
		VHAL_LOGE("SysDb Event[%d] is not supported.", evtcode);
		return;
	}
	VhalSysDbEventEntry &entry{itr_sysdb->second};

	entry.Action();

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	SysDB初期化処理
 引数    ：	SysdbInitStatus &status		(i)SysDB初期化情報
        	bool &terminate				(i)処理強制終了(false:継続 true:終了)
 戻り値  ：	処理結果
           		VHAL_ERR_****		エラー
           		VHAL_SUCCESS		正常終了
 フェールセーフNo  ：	F-VHAL-R-042A
                    	F-VHAL-R-042B
                    	F-VHAL-R-043A
                    	F-VHAL-R-043B
                    	F-VHAL-R-044A
                    	F-VHAL-R-044B
                    	F-VHAL-R-045A
                    	F-VHAL-R-045B
                    	F-VHAL-N-127
                    	F-VHAL-R-128
*****************************************************************************/
int32_t CVhalSysdbControl::OpenSysdb(struct SysdbInitStatus& status, const bool& terminate)
{
	int32_t	result{VHAL_SUCCESS};
	
	static const std::string VHAL_STAT_EPNAME{"video_hal_svc.sysdb"};
	std::string name{VHAL_STAT_EPNAME};

	/* 機能ID取得 */
	if ((false == status.ul_func_id_) && (0U < status.count_ul_func_id_))
	{
		uint32_t    count{0U};
		int32_t ret{UTL_PKG_COM_RET_OK};
		for(uint32_t i{0U}; i<status.count_ul_func_id_; (void)++i)
		{
			bool loop_end{false};
			(void)++count;
			ret = UtlPkgComGetFuncIDByName(reinterpret_cast<uint8_t *>(const_cast<char *>(name.c_str())), &clientObj_.ulFuncId);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//			int32_t fail_ret{0};
//			bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-R-042",fail_ret)};
//			if(true == fail)
//			{
//				ret = fail_ret;
//			}
//#endif
			if (UTL_PKG_COM_RET_OK == ret)
			{
				status.ul_func_id_ = true;
				loop_end = true;
			}
			else
			{
				/* 復旧見込みなし */
				if ((UTL_PKG_COM_RET_ERR_ARG == ret) || (UTL_PKG_COM_RET_ERR_SYS == ret) || (UTL_PKG_COM_RET_ERR_NOENT == ret))
				{
					loop_end = true;
				}
				else
				{
					/* 処理強制終了 */
					if (true == terminate )
					{
						loop_end = true;
					}
					else
					{
						constexpr uint32_t kVhalSysDbInitRetryWait{500U};	/* SysDB初期化のリトライ待ち時間(ms) */
						(void)usleep(kVhalSysDbInitRetryWait * 1000U);
					}
				}
			}
			if (true == loop_end)
			{
				break;
			}
		}
		if( false == status.ul_func_id_ )
		{
			VHAL_LOGE("UtlPkgComGetFuncIDByName(%s) error ret=%d count=%d/%d", VHAL_STAT_EPNAME.c_str(), ret, count, status.count_ul_func_id_);
			result = VHAL_ERR_SYSDB_API;
		}
		else
		{
			VHAL_LOGV("UtlPkgComGetFuncIDByName(%s) success ret=%d count=%d/%d", VHAL_STAT_EPNAME.c_str(), ret, count, status.count_ul_func_id_);
		}
	}

	/* SysDB(Client)オープン */
	if ((VHAL_SUCCESS == result) && (false == status.client_open_) && (0U < status.count_client_open_))
	{
		uint32_t    count{0U};
		int32_t ret{SYS_DB_SUCCESS};
		for(uint32_t i{0U}; i<status.count_client_open_; (void)++i)
		{
			bool loop_end{false};
			(void)++count;
			ret = SysDbClientOpen(&clientObj_);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//			int32_t fail_ret{0};
//			bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-R-043",fail_ret)};
//			if(true == fail)
//			{
//				ret = fail_ret;
//			}
//#endif
			if (SYS_DB_SUCCESS == ret)
			{
				status.client_open_ = true;
				sysdb_open_status = true;
				loop_end = true;
			}
			else
			{
				/* 復旧見込みなし */
				if (SYS_DB_ERR_ARGUMENT == ret)
				{
					loop_end = true;
				}
				else
				{
					/* 処理強制終了 */
					if (true == terminate )
					{
						loop_end = true;
					}
					else
					{
						constexpr uint32_t kVhalSysDbInitRetryWait{500U};	/* SysDB初期化のリトライ待ち時間(ms) */
						(void)usleep(kVhalSysDbInitRetryWait * 1000U);
					}
				}
			}
			if (true == loop_end)
			{
				break;
			}
		}
		if( false == status.client_open_ )
		{
			VHAL_LOGE("SysDbClientOpen error ret=%d count=%d/%d", ret, count, status.count_client_open_);
			result = VHAL_ERR_SYSDB_API;
		}
		else
		{
			VHAL_LOGV("SysDbClientOpen success ret=%d count=%d/%d", ret, count, status.count_client_open_);
		}
	}

	if (VHAL_SUCCESS == result)
	{
		/* DBファイル名⇔イベントID変換テーブル作成 */
		if ((false == status.dv_event_id_) && (0U < status.count_dv_event_id_))
		{
			for (auto itr_sysdb = sysdbevent_entries_.begin(); itr_sysdb != sysdbevent_entries_.end(); ++itr_sysdb)
			{
				VhalSysDbEventEntry &entry{itr_sysdb->second};

				bool loop_end{false};
				uint32_t count{0U};
				int32_t ret{SYS_DB_SUCCESS};
				uint32_t dbeveid{};
				for(uint32_t i{0U}; i < status.count_dv_event_id_; ++i)
				{
					(void)++count;
					ret = SysDbPathToEvtId(&clientObj_, reinterpret_cast<int8_t *>(const_cast<char *>(entry.GetDbpath().c_str())), &dbeveid);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//					int32_t fail_ret{0};
//					bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-R-128",fail_ret)};
//					if(true == fail)
//					{
//						ret = fail_ret;
//					}
//#endif
					if (SYS_DB_SUCCESS == ret)
					{
						status.dv_event_id_ = true;
						loop_end = true;
					}
					else
					{
						if ((SYS_DB_ERR_UNINITIALIZED != ret) && (SYS_DB_ERR_COMMUNICATION != ret))
						{
							/* 復旧見込みなし */
							loop_end = true;
						}
						else
						{
							/* 処理強制終了 */
							if (true == terminate )
							{
								loop_end = true;
							}
							else
							{
								/* リトライ有り */
								constexpr uint32_t kVhalSysDbInitRetryWait{500U};	/* SysDB初期化のリトライ待ち時間(ms) */
								(void)usleep(kVhalSysDbInitRetryWait * 1000U);
							}
						}
					}
					
					if (true == loop_end)
					{
						break;
					}
				}
				if (SYS_DB_SUCCESS != ret)
				{
					VHAL_LOGE("SysDbPathToEvtId() error.ret=%d [path=%s] count=%d/%d", ret, entry.GetDbpath().c_str(), count, status.count_dv_event_id_);
					result = VHAL_ERR_SYSDB_API;
				}
				else
				{
					VHAL_LOGV("SysDbPathToEvtId() succes ret=%d [path=%s] count=%d/%d", ret, entry.GetDbpath().c_str(), count, status.count_dv_event_id_);
					entry.SetDbeveid(dbeveid);
				}
			}
		}
	}

	/* イベント配信用の機能ID取得 */
	if ((VHAL_SUCCESS == result) && (false == status.evmng_open_) && (0U < status.count_ul_func_id_) && (0U < status.count_evmng_open_) )
	{
		/* イベント配信用の通信端点名 */
		static const std::string VHAL_EVMNG_EPNAME{"video_hal_svc.evmng"};
		uint32_t funcId{0U};
		uint32_t count{0U};
		name.clear();
		name = VHAL_EVMNG_EPNAME;
		int32_t ret{UTL_PKG_COM_RET_OK};
		for(uint32_t i{0U}; i<status.count_ul_func_id_; (void)++i)
		{
			bool loop_end{false};
			(void)++count;
			ret = UtlPkgComGetFuncIDByName(reinterpret_cast<uint8_t *>(const_cast<char *>(name.c_str())), &funcId);
			if (UTL_PKG_COM_RET_OK == ret)
			{
				loop_end = true;
			}
			else
			{
				/* 復旧見込みなし */
				if ((UTL_PKG_COM_RET_ERR_ARG == ret) || (UTL_PKG_COM_RET_ERR_SYS == ret) || (UTL_PKG_COM_RET_ERR_NOENT == ret))
				{
					loop_end = true;
				}
				else
				{
					/* 処理強制終了 */
					if (true == terminate )
					{
						loop_end = true;
					}
					else
					{
						constexpr uint32_t kVhalSysDbInitRetryWait{500U};	/* SysDB初期化のリトライ待ち時間(ms) */
						(void)usleep(kVhalSysDbInitRetryWait * 1000U);
					}
				}
			}
			if (true == loop_end)
			{
				break;
			}
		}
		if( UTL_PKG_COM_RET_OK != ret )
		{
			VHAL_LOGE("UtlPkgComGetFuncIDByName(%s) error ret=%d count=%d/%d", VHAL_EVMNG_EPNAME.c_str(), ret, count, status.count_ul_func_id_);
			result = VHAL_ERR_SYSDB_API;
		}
		else
		{
			VHAL_LOGV("UtlPkgComGetFuncIDByName(%s) success ret=%d count=%d/%d", VHAL_EVMNG_EPNAME.c_str(), ret, count, status.count_ul_func_id_);
		}

		if (VHAL_SUCCESS == result)
		{
			/* イベント管理オブジェクトオープン */
			count = 0U;
			ret = SYS_EV_MNG_RET_OK;
			for(uint32_t i{0U}; i<status.count_evmng_open_; (void)++i)
			{
				bool loop_end{false};
				(void)++count;
				ret = SysEvMngOpen(funcId, &evMngObj_);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//				int32_t fail_ret{0};
//				bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-R-044",fail_ret)};
//				if(true == fail)
//				{
//					ret = fail_ret;
//				}
//#endif
				if (SYS_EV_MNG_RET_OK == ret)
				{
					status.evmng_open_ = true;
					loop_end = true;
				}
				else
				{
					/* 復旧見込みなし */
					if (SYS_EV_MNG_RET_ARG_ERR == ret)
					{
						loop_end = true;
					}
					else
					{
						/* 処理強制終了 */
						if (true == terminate )
						{
							loop_end = true;
						}
						else
						{
							constexpr uint32_t kVhalSysDbInitRetryWait{500U};	/* SysDB初期化のリトライ待ち時間(ms) */
							(void)usleep(kVhalSysDbInitRetryWait * 1000U);
						}
					}
				}
				if (true == loop_end)
				{
					break;
				}
			}
			if( false == status.evmng_open_ )
			{
				VHAL_LOGE("SysEvMngOpen error ret=%d count=%d/%d", ret, count, status.count_evmng_open_);
				result = VHAL_ERR_SYSDB_API;
			}
			else
			{
				VHAL_LOGV("SysEvMngOpen success ret=%d count=%d/%d", ret, count, status.count_evmng_open_);
			}
		}
	}

	/* イベント配信コールバック関数登録 */
	if ((VHAL_SUCCESS == result) && (false == status.delv_clbk_) && (0U < status.count_delv_clbk_))
	{
		uint32_t    count{0U};
		int32_t ret{SYS_EV_MNG_RET_OK};
		for(uint32_t i{0U}; i<status.count_delv_clbk_; (void)++i)
		{
			bool loop_end{false};
			(void)++count;
			ret = SysEvMngRegisterDelvClbk(&evMngObj_, reinterpret_cast<SysEvMngNotifyDelv*>(&CVhalDbFileEventCb));
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//			int32_t fail_ret{0};
//			bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-R-045",fail_ret)};
//			if(true == fail)
//			{
//				ret = fail_ret;
//			}
//#endif
			if (SYS_EV_MNG_RET_OK == ret)
			{
				status.delv_clbk_ = true;
				loop_end = true;
			}
			else
			{
				/* 復旧見込みなし */
				if (SYS_EV_MNG_RET_ARG_ERR == ret)
				{
					loop_end = true;
				}
				else
				{
					/* 処理強制終了 */
					if (true == terminate )
					{
						loop_end = true;
					}
					else
					{
						constexpr uint32_t kVhalSysDbInitRetryWait{500U};	/* SysDB初期化のリトライ待ち時間(ms) */
						(void)usleep(kVhalSysDbInitRetryWait * 1000U);
					}
				}
			}
			if (true == loop_end)
			{
				break;
			}
		}
		if( false == status.delv_clbk_ )
		{
			VHAL_LOGE("SysEvMngRegisterDelvClbk error ret=%d count=%d/%d", ret, count, status.count_delv_clbk_);
			result = VHAL_ERR_SYSDB_API;
		}
		else
		{
			VHAL_LOGV("SysEvMngRegisterDelvClbk success ret=%d count=%d/%d", ret, count, status.count_delv_clbk_);
		}
	}

	if (VHAL_SUCCESS == result)
	{
		/* SysDB書き込みデータがあれば書き込み実施 */
		const int32_t ret{WriteSysdbInfoList(this)};
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("WriteSysdbInfoList() error. ret=%d", ret);
			result = VHAL_ERR;
		}
		else
		{
			VHAL_LOGI("success");
		}
	}

	return result;
}

/*****************************************************************************
 処理概要：	イベントリスナー登録
 引数    ：	CVhalSysdbEventListenerBase* p_listener		(i)リスナーポインタ
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM			パラメータ不正
           		VHAL_SUCCESS			正常終了
****************************************************************************/
int32_t CVhalSysdbControl::RegisterEventListener(CVhalSysdbEventListenerBase* const p_listener)
{
	int32_t	result{VHAL_SUCCESS};

	if (nullptr == p_listener)
	{
		VHAL_LOGE("p_listener is null");
		result = VHAL_ERR_PARAM;
	}
	else
	{
		p_sysdb_listener_ = p_listener;
	}

	return result;
}

/*****************************************************************************
 処理概要：	イベントリスナー解除
 引数    ：	なし
 戻り値  ：	なし
****************************************************************************/
void CVhalSysdbControl::ClearEventListener(void) noexcept
{
	p_sysdb_listener_ = nullptr;
}

/*****************************************************************************
 処理概要：	SysDB書き込みデータリストへデータ追加(Writeデータサイズ:int32_t)
 引数    ：	const std::string&	sysdbPath		(i)SYSDBパス
           	int32_t 			state			(i)Writeする値
 戻り値  ：	処理結果
           		VHAL_ERR_****		エラー
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalSysdbControl::AddSysdbInfoList(const std::string &sysdbPath, const int32_t state)
{
	const std::lock_guard<std::mutex> lock_sysdb{mtx_sysdb_};

	if (sysdbPath.empty())
	{
		VHAL_LOGE("parameter error. sysdbPath is empty. state=%d", state);
		return VHAL_ERR_PARAM;
	}

	VHAL_LOGI("DbName=%s, state=%d", sysdbPath.c_str(), state);

	/* SysDB書き込みデータを追加 */
	const sysdbWriteInfo write_info{sysdbPath, state};
	sysdb_write_list_.push_back(write_info);
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	SysDB書き込みデータリスト情報をSysDBへ書込む
 引数    ：	CVhalSysdbControl *p_sysdbctrl		(i)SysDB制御インスタンスポインタ
 戻り値  ：	処理結果
           		VHAL_ERR_****		エラー
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalSysdbControl::WriteSysdbInfoList(CVhalSysdbControl * const p_sysdbctrl)
{
	const std::lock_guard<std::mutex> lock_sysdb{mtx_sysdb_};

	if (nullptr == p_sysdbctrl)
	{
		VHAL_LOGE("parameter error. p_sysdbctrl is empty.");
		return VHAL_ERR_PARAM;
	}

	for (const auto& write_info : p_sysdbctrl->sysdb_write_list_)
	{
		const int32_t ret{p_sysdbctrl_->SetValue(write_info.GetSysDbPath(), write_info.GetState())};
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("SetValue error. ret=%d", ret);
			return VHAL_ERR;
		}
	}

	p_sysdbctrl->sysdb_write_list_.clear();
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	VMSTR移行ステータス通知処理
 引数    ：	なし
 戻り値  ：	なし
 フェールセーフNo ：	F-VHAL-N-357
                    	F-VHAL-N-358
                    	F-VHAL-N-359
                    	F-VHAL-N-362
*****************************************************************************/
void CVhalSysdbControl::ActionVmStrChange(void)
{
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	int32_t fail_ret_none{0};		/* 変化通知なし */
//	CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-N-362",fail_ret_none);
//	if (0 != fail_ret_none)
//	{
//		VHAL_LOGW("notify dropped. (failsafe-362)");
//		return;		/* 通知を無かったことにする */
//	}
//#endif

	VHAL_LOGV_IN();

	/* VHAL_SYSDB_PATH_VMSTR_CHANGEは 2バイト構成のため、2バイト読込が必要。 */
	/* 状態判定に使用する有効値は先頭1バイトのみである。 */
	uint8_t		vmstr[2]{};
	const int32_t	ret{GetValue(VHAL_SYSDB_PATH_VMSTR_CHANGE, vmstr, sizeof(vmstr))};
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("SysDB %s Read error. ret=%d", VHAL_SYSDB_PATH_VMSTR_CHANGE.c_str(), ret);
	}
	else
	{
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//		int32_t		fail_ret{static_cast<int32_t>(vmstr[0])};
//		CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-N-357",fail_ret);
//		CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-N-358",fail_ret);
//		CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-N-359",fail_ret);
//		vmstr[0] = ToUI8(fail_ret);
//#endif
		const int32_t	vmstr_val{static_cast<int32_t>(vmstr[0])};

		switch (vmstr_val)
		{
			/* 処理成功 */
			case DAT_CNV_VM_STR_CHANGE_SUCCESS :
				VHAL_LOGI("VMSTR success. [%d]", vmstr_val);
				/* 取得値を退避 */
				CVhalStrManager::GetInstance().SetVmstrValue(VHAL_SUCCESS);
				/* 待ち状態を解除する　★１(CVhalPropertyControl::StrStatePfmSuspend関数に対応) */
				CVhalStrManager::GetInstance().NotifyOne(StrWaitState::SuspendApps);
				break;

			/* 処理失敗 */
			case DAT_CNV_VM_STR_CHANGE_FAILURE :
				VHAL_LOGE("VMSTR failed. [%d]", vmstr_val);
				/* 取得値を退避 */
				CVhalStrManager::GetInstance().SetVmstrValue(VHAL_ERR);
				/* 待ち状態を解除する　★１(CVhalPropertyControl::StrStatePfmSuspend関数に対応) */
				CVhalStrManager::GetInstance().NotifyOne(StrWaitState::SuspendApps);
				break;

			/* 処理中・初期化 */
			case DAT_CNV_VM_STR_CHANGE_PROCESSING :
			case DAT_CNV_VM_STR_CHANGE_UNPROCESSED :
				VHAL_LOGD("VMSTR nop. [%d]", vmstr_val);
				break;

			/* 異常値 */
			default :
				VHAL_LOGE("VMSTR invalid. [%d]", vmstr_val);
				break;
		}
	}
	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	HDMIビデオフォーマット取得
 引数    ：	int32_t					state				(i)解像度値
           	uint32_t& 				width				(o)解像度幅
           	uint32_t& 				height				(o)解像度高さ
 戻り値  ：	処理結果
           		VHAL_SUCCESS		正常終了
           		VHAL_ERR_PARAM		パラメータ不正
*****************************************************************************/
int32_t CVhalSysdbControl::GetHdmiVideoFormat(const int32_t state, uint32_t& width, uint32_t& height) const
{
	int32_t ret{VHAL_SUCCESS};

	VHAL_LOGV_IN();

	/* HDMI解像度取得 */
	VHAL_LOGI("hdmi video format path=%s state=%d", VHAL_SYSDB_PATH_HDMI_VIDEO_FORMAT.c_str(), state);
	const auto itr_format = hdmi_videoformat_list_.find(I32ToUI32(state));
	if (itr_format == hdmi_videoformat_list_.end())
	{
		VHAL_LOGE("invalid hdmi video format=%d", state);
		ret = VHAL_ERR_PARAM;
	}
	else
	{
		width  = itr_format->second.width;
		height = itr_format->second.height;
	}

	VHAL_LOGV_OUT();

	return ret;
}

/*****************************************************************************
 処理概要：	コールドブート(+B起動)状態取得
 引数    ：	なし
 戻り値  ：	処理結果
           		true			コールドブート状態
           		false			上記以外
*****************************************************************************/
bool CVhalSysdbControl::IsColdBoot(void)
{
	bool result{false};
	int32_t state{DAT_CNV_CAR_INFO_POWER_WAKEUP_FLAG_UNKNOWN};

	VHAL_LOGV_IN();

	/* 起動要因取得 */
	static const std::string VHAL_SYSDB_PATH_WAKEUP_FLAG		{"/db/car_info/power/wakeup_flag.dbf"};
	const int32_t ret{GetValue(VHAL_SYSDB_PATH_WAKEUP_FLAG, &state)};
	if (VHAL_SUCCESS != ret)
	{
		/* SysDB読み込み失敗時は、+B起動扱い（フェールセーフ） */
		result = true;
		VHAL_LOGE("SysDB %s Read error. ret=%d", VHAL_SYSDB_PATH_WAKEUP_FLAG.c_str(), ret);
	}
	else
	{
		switch(state)
		{
			case DAT_CNV_CAR_INFO_POWER_WAKEUP_FLAG_WNL:
			case DAT_CNV_CAR_INFO_POWER_WAKEUP_FLAG_WAL:
				/* ACC起動 */
				result = false;
				break;

			case DAT_CNV_CAR_INFO_POWER_WAKEUP_FLAG_UNKNOWN:
			case DAT_CNV_CAR_INFO_POWER_WAKEUP_FLAG_CNL:
			case DAT_CNV_CAR_INFO_POWER_WAKEUP_FLAG_CAL:
			default:
				/* +B起動。取得値不正の場合は、+B起動扱い（フェールセーフ） */
				result = true;
				break;
		}
		VHAL_LOGI("SysDB=%s state=%d, result=%d", VHAL_SYSDB_PATH_WAKEUP_FLAG.c_str(), state, result);
	}

	VHAL_LOGV_OUT();

	return result;
}

} /* namespace videohal */

