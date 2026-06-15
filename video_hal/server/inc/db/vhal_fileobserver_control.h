/*******************************************************************************
    機能名称    ：  FileObserver制御モジュール
    ファイル名称：  vhal_fileobserver_control.h
*******************************************************************************/
#ifndef	VHAL_FILEOBSERVER_CONTROL_H
#define	VHAL_FILEOBSERVER_CONTROL_H

#include "file_observer.h"
#include "vhal_log.h"
#include "vhal_observer_control.h"
#include "vhal_observer_client.h"
#include "vhal_capture_control.h"
#include "vhal_layout_video_setting.h"
#include "vhal_event_route.h"

namespace videohal
{
class CVhalMainControl;
class CVhalFileObserverControl;

enum class VhalHdmiState : uint32_t {
	kDisconnect = 0,	/* HDMI未接続 */
	kConnect,			/* HDMI接続 */
	kUnknown,			/* HDMI状態不明 */
	kMax
};

/* カメラ制御権 */
enum class VhalCameraDisplayStatus : uint32_t {
	kInitial = 0,		/* Unknown/Initial */
	kReady,				/* Ready */
	kMax
};

struct VhalFileObserverEntry
{
public:
	VhalFileObserverEntry(const uint32_t value, const std::function<void(const uint32_t value)> action)
	:value_(value)
	,action_(action)
	{
	}

	void SetValue(const uint32_t value) noexcept
	{
		value_ = value;
	}

	uint32_t GetValue(void) const noexcept
	{
		return value_;
	}

	void Action(const uint32_t value) const
	{
		if (nullptr != action_)
		{
			action_(value);
		}
		else
		{
			VHAL_LOGE("Not action error.");
		}
	}

private:
	uint32_t	value_;
	std::function<void(const uint32_t value)> action_;
};

/*****************************************************************************
 クラス名称：CVhalFileObserverHdmiObserver
 処理概要  ：HDMI接続状態更新監視クラス
*****************************************************************************/
class CVhalFileObserverHdmiObserver : public CVhalObserverClient {
public:
	CVhalFileObserverHdmiObserver(void) = default;
	~CVhalFileObserverHdmiObserver(void) override;
	CVhalFileObserverHdmiObserver(const CVhalFileObserverHdmiObserver& src) = delete;
	CVhalFileObserverHdmiObserver& operator=(const CVhalFileObserverHdmiObserver& src) & = delete;
	CVhalFileObserverHdmiObserver(CVhalFileObserverHdmiObserver&& src) = delete;
	CVhalFileObserverHdmiObserver& operator=(CVhalFileObserverHdmiObserver&& src) & = delete;

	void Initialize(CVhalFileObserverControl* const p_file_observer_ctl) noexcept;
	void Finalize(void) noexcept;
	
	void SetHdmiState(const VhalHdmiState state) noexcept;

	/* HDMI更新監視周期コールバック */
	int32_t Notify(void) override;
private:
	CVhalFileObserverControl*				p_file_observer_{nullptr};
	
	VhalHdmiState hdmi_state_{VhalHdmiState::kMax};
};

/*****************************************************************************
 クラス名称：CVhalFileObserverEventListenerBase
 処理概要  ：VhalFileObserverイベントリスナベース。
*****************************************************************************/
class CVhalFileObserverEventListenerBase {
public:
	CVhalFileObserverEventListenerBase(void) noexcept = default;
	virtual ~CVhalFileObserverEventListenerBase(void) = default;
  	CVhalFileObserverEventListenerBase(const CVhalFileObserverEventListenerBase& src) = delete;
	CVhalFileObserverEventListenerBase(CVhalFileObserverEventListenerBase&& src) = delete;

	virtual void NotifyReceiveCameraDisplayStatus(const VhalCameraDisplayStatus current_cam_disp_status) const noexcept = 0;
	virtual void NotifySettingConnectedHdmiDevice(const VhalHdmiState state) const noexcept = 0;
private:
	CVhalFileObserverEventListenerBase& operator=(const CVhalFileObserverEventListenerBase& src) & = delete;
	CVhalFileObserverEventListenerBase& operator=(CVhalFileObserverEventListenerBase&& src) & = delete;
};

/*****************************************************************************
 クラス名称：CVhalFileObserverControl
 処理概要  ：File Systemに対してプロパティの更新を行う。
*****************************************************************************/
class CVhalFileObserverControl final {
public:
	/* FileObserverアクセスリトライ回数 */
	static constexpr uint32_t kRetryMaxStartup{300U};		/* 起動時 */
	static constexpr uint32_t kRetryMaxNormal{15U};			/* 通常時 */
	/* FileObserverアクセスリトライ待ち時間(ms) */
	static constexpr uint32_t kRetryWaitStartup{200U};		/* 起動時 */
	static constexpr uint32_t kRetryWaitNormal{100U};		/* 通常時 */

	CVhalFileObserverControl(void);
	~CVhalFileObserverControl(void);
	CVhalFileObserverControl(const CVhalFileObserverControl& src) = delete;
	CVhalFileObserverControl& operator=(const CVhalFileObserverControl& src) & = delete;
	CVhalFileObserverControl(CVhalFileObserverControl&& src) = delete;
	CVhalFileObserverControl& operator=(CVhalFileObserverControl&& src) & = delete;


	int32_t Initialize(CVhalMainControl* const p_main_ctl, CVhalObserverControl * const p_observer_ctl);
	void Finalize(void);

	int32_t UpdatePropertyHdmi(const bool check_running, const VhalHdmiState hdmi_state, const uint32_t retry_num, const uint32_t retry_wait);

	int32_t GetPropertyHdmi(VhalHdmiState &hdmi_state) const;
	int32_t GetCameraDisplayStatus(VhalCameraDisplayStatus &current_cam_disp_status);

	void ExecFileObserverEvent(const std::string& monitor_path, const uint32_t value);
	void FileobPathCallback(const std::string monitor_path);

	/* イベントリスナー登録 */
	int32_t RegisterEventListener(CVhalFileObserverEventListenerBase* const p_listener);
	/* イベントリスナー解除 */
	void ClearEventListener(void) noexcept;
	
	void SetHdmiInitialized(const bool initialized)  noexcept
	{
		hdmi_initialized_ = initialized;
	}


private:
	std::shared_ptr<cockpit::bs::CFileObserver> p_fileob_;
	CVhalMainControl*							p_main_;
	CVhalFileObserverEventListenerBase*			p_fileobserver_listener_;
	CVhalFileObserverControl*					p_fileobserver_control_;
	std::unique_ptr<CVhalEventRoute>			p_route_;
	std::vector<int32_t>						fileob_id_list_;
	std::unordered_map<std::string, struct VhalFileObserverEntry> fileob_entries_;
	std::unique_ptr<CVhalFileObserverHdmiObserver>	p_hdmi_observer_;
	CVhalObserverControl*						p_observer_control_;
	CVhalPropertyControl*						p_prop_;
	
	/* HDMI接続状態初回書き込み実施有無 */
	bool hdmi_initialized_;
	/* HDMI接続状態現在値 */
	VhalHdmiState hdmi_state_current_;
	
	/* 対象ファイルパス値の取得 */
	int32_t GetStatus(const std::string& monitor_path, uint32_t &value, const uint32_t retry_num=kRetryMaxNormal, const uint32_t retry_wait=kRetryWaitNormal) const;
	/* 状態更新 */
	bool UpdateStatus(const std::string& monitor_path, const uint32_t value);
	/* FileObserver対象のイベント処理の実行（カメラ制御権） */
	void ExecCameraDisplayStatus(const uint32_t value);
};

} /* namespace videohal */

#endif	/* #ifndef	VHAL_FILEOBSERVER_CONTROL_H */
