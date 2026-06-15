/*******************************************************************************
    機能名称    ：  SysDB制御モジュール
    ファイル名称：  vhal_sysdb_control.h
*******************************************************************************/
#ifndef	VHAL_SYSDB_CONTROL_H
#define	VHAL_SYSDB_CONTROL_H

#include <cstddef>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <functional>

#include "vhal_event_route.h"
#include "vhal_worker_thread.h"

extern "C"
{
	#include <sys_db_api_public.h>
	#include <sys_db_cnv_value_public.h>
	#include <sys_ev_mng_api_public.h>
	#include <utl_pkg_com_api_public.h>

	#include "spf_timer_public.h"
}

namespace videohal
{
class CVhalMainControl;
class CVhalSysdbControl;

static const std::string VHAL_SYSDB_PATH_CONN_INFO_HDMI		{"/db/sys_info/conn_info/hdmi.dbf"};
static const std::string VHAL_SYSDB_PATH_HDMI_VIDEO_FORMAT	{"/db/sys_info/conn_info/hdmi_info/video_format.dbf"};
static const std::string VHAL_SYSDB_PATH_HDMI_AUDIO_FORMAT	{"/db/sys_info/conn_info/hdmi_info/audio_format.dbf"};
static const std::string VHAL_SYSDB_PATH_SERVICE_STATE		{"/db/sys_info/service/video_hal_svc.dbf"};
static const std::string VHAL_SYSDB_PATH_CCMSVC_STATE		{"/db/sys_info/service/ccm_svc.dbf"};
static const std::string VHAL_SYSDB_PATH_VMSTR_CHANGE		{"/db/sys_info/power/vm_str_change_status.dbf"};


/* SysDb Event Type */
enum class sysdbEventType : uint8_t {
  VHAL_SYSDB_EVENT_NONE,
  VHAL_SYSDB_EVENT_VMSTR_CHANGE,
  VHAL_SYSDB_EVENT_MAX
};

struct VhalSysDbEventEntry
{
public:
	VhalSysDbEventEntry(const uint32_t dbeveid, const std::string dbpath, const std::function<void(void)> action)
	:dbeveid_(dbeveid)
	,dbpath_(dbpath)
	,action_(action)
	{
	}

	void SetDbeveid(const uint32_t dbeveid) noexcept
	{
		dbeveid_ = dbeveid;
	}

	uint32_t GetDbeveid(void) const noexcept
	{
		return dbeveid_;
	}

	std::string GetDbpath(void) const noexcept
	{
		return dbpath_;
	}

	void Action(void) const noexcept
	{
		if (nullptr != action_)
		{
			action_();
		}
	}

private:
	uint32_t			dbeveid_;
	const std::string	dbpath_;
	std::function<void(void)> action_;
};

/*****************************************************************************
 クラス名称：CVhalSysdbEventListenerBase
 処理概要  ：VhalSysdbControlイベントリスナベース。
*****************************************************************************/
class CVhalSysdbEventListenerBase {
public:
	CVhalSysdbEventListenerBase(void) noexcept = default;
	virtual ~CVhalSysdbEventListenerBase(void) = default;
  	CVhalSysdbEventListenerBase(const CVhalSysdbEventListenerBase& src) = delete;
	CVhalSysdbEventListenerBase(CVhalSysdbEventListenerBase&& src) = delete;

	/* 後日CVhalSysdbEventListenerイベントが追加される可能性があるのでクラス自体は残す */
private:
	CVhalSysdbEventListenerBase& operator=(const CVhalSysdbEventListenerBase& src) & = delete;
	CVhalSysdbEventListenerBase& operator=(CVhalSysdbEventListenerBase&& src) & = delete;
};

/*****************************************************************************
 クラス名称：CVhalSysdbControl
 処理概要  ：SysDBの制御を行う。
*****************************************************************************/
class CVhalSysdbControl {
public:
	/* SysDB初期化情報 */
	struct SysdbInitStatus {
		bool		ul_func_id_;
		bool		client_open_;
		bool		dv_event_id_;
		bool		evmng_open_;
		bool		delv_clbk_;
		uint32_t	count_ul_func_id_;
		uint32_t	count_client_open_;
		uint32_t	count_dv_event_id_;
		uint32_t	count_evmng_open_;
		uint32_t	count_delv_clbk_;
	};
	
	CVhalSysdbControl(void);
	virtual ~CVhalSysdbControl(void);
	CVhalSysdbControl(const CVhalSysdbControl& src) = delete;
	CVhalSysdbControl& operator=(const CVhalSysdbControl& src) & = delete;
	CVhalSysdbControl(CVhalSysdbControl&& src) = delete;
	CVhalSysdbControl& operator=(CVhalSysdbControl&& src) & = delete;

	int32_t Initialize(CVhalMainControl * const p_main_ctl);
	void Finalize(void);

	int32_t GetValue(const std::string &sysdbPath, int32_t * const state);
	int32_t SetValue(const std::string &sysdbPath, int32_t state);

	void NotifySysdbEvent(const SysDbEvtDataHeader* const head);
	void ExecSysdbEvent(const sysdbEventType evtcode);

	int32_t GetHdmiVideoFormat(const int32_t state, uint32_t& width, uint32_t& height) const;
	bool IsColdBoot(void);
	int32_t OpenSysdb(struct SysdbInitStatus& status, const bool& terminate);

	/* イベントリスナー登録 */
	int32_t RegisterEventListener(CVhalSysdbEventListenerBase* const p_listener);
	/* イベントリスナー解除 */
	void ClearEventListener(void) noexcept;
	/* VMSTR移行ステータス通知処理 */
	void ActionVmStrChange(void);

private:
	CVhalMainControl		*p_main_;
	CVhalSysdbEventListenerBase*			p_sysdb_listener_;
	std::unique_ptr<CVhalEventRoute>		p_route_;


	SysDbClientObj			clientObj_;		/* SysDB(Client)オブジェクト		*/
	SysEvMngObj				evMngObj_;		/* イベント管理オブジェクト			*/

	std::unordered_map<sysdbEventType, struct VhalSysDbEventEntry> sysdbevent_entries_;

	bool					sysdb_open_status;		/* SysDbClientOpen完了状態 */
	/* SysDB書き込みデータ構造体 */
	struct sysdbWriteInfo {
	public:
		sysdbWriteInfo(const std::string sysdbPath, const int32_t state)
			:sysdbPath_(sysdbPath)
			,state_(state)
		{}
		std::string GetSysDbPath(void) const noexcept
		{
			return sysdbPath_;
		}
		int32_t GetState(void) const noexcept
		{
			return state_;
		}
	private:
		std::string			sysdbPath_;				/* SysDBパス */
		int32_t				state_;					/* Writeする値 */
	};
	std::vector<sysdbWriteInfo> sysdb_write_list_;
	mutable std::mutex			mtx_sysdb_;

	/* HDMIビデオフォーマット情報 */
	struct hdmiVideoFormat
	{
		uint32_t			width;
		uint32_t			height;
	};
	/* HDMIビデオフォーマットリスト */
	static std::unordered_map<uint32_t, struct hdmiVideoFormat> hdmi_videoformat_list_;

	int32_t NotifySysdbEvent(const sysdbEventType evtcode);

	/* SysDB初期化スレッド */
	class CVhalSysdbInitThread : public CVhalWorkerThread<struct SysdbInitStatus> {
	public:
		CVhalSysdbInitThread(CVhalSysdbControl* const p_sysdb_control) noexcept
			:p_sysdb_control_(p_sysdb_control)
		{
		}
		~CVhalSysdbInitThread(void) override = default;
	  	CVhalSysdbInitThread(const CVhalSysdbInitThread& src) = delete;
		CVhalSysdbInitThread& operator=(const CVhalSysdbInitThread& src) & = delete;
		CVhalSysdbInitThread(CVhalSysdbInitThread&& src) = delete;
		CVhalSysdbInitThread& operator=(CVhalSysdbInitThread&& src) & = delete;
	private:
		int32_t Execute(const struct SysdbInitStatus& data, const bool terminate) const noexcept override;
		std::unique_ptr<CVhalSysdbControl>	p_sysdb_control_;
	};
	std::unique_ptr<CVhalSysdbInitThread>	p_init_thread_;

	int32_t AddSysdbInfoList(const std::string &sysdbPath, const int32_t state);
	int32_t WriteSysdbInfoList(CVhalSysdbControl * const p_sysdbctrl);
	int32_t GetValue(const std::string &sysdbPath, void* const state, const std::size_t length);

};

void CVhalDbFileEventCb(SysEvMngComEveHeadInf* const comHead, int8_t* const eveDat);

} /* namespace videohal */


#endif	/* #ifndef	VHAL_SYSDB_CONTROL_H */
