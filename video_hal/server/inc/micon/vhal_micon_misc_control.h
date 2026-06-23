/*******************************************************************************
    機能名称    ：  マイコン間MISC通信制御モジュール
    ファイル名称：  vhal_micon_misc_control.h
*******************************************************************************/
#ifndef	VHAL_MICON_MISC_CONTROL_H
#define	VHAL_MICON_MISC_CONTROL_H

#include <vector>
#include <list>
#include <mutex>
#include <condition_variable>

#include "vhal_event_item_base.h"
#include "vhal_event_route.h"
#include "vhal_micon_misc_receiver.h"
#include "vhal_micon_misc_opc.h"

extern "C"
{
#include "com_stddef.h"
#include "misc_ctrl_api_public.h"
}

enum : uint8_t {
      DATA_INDEX_SUB_TYPE = 0U
	, DATA_INDEX_FORMAT = 1U
	, DATA_INDEX_CONNECTION = 1U
};

namespace videohal
{
class CVhalMiconReceiveItem;
class CVhalMiconCommMiscControl;
class CVhalSysdbControl;

/*****************************************************************************
 クラス名称：CVhalMiscEventItem
 処理概要  ：MISC通信イベントクラス
*****************************************************************************/
class CVhalMiscEventItem : public CVhalEventItemBase {
public:

	CVhalMiscEventItem(CVhalMiconCommMiscControl * const p_misc_control)
		:p_misc_control_(p_misc_control)
	{
	}
	~CVhalMiscEventItem(void) override = default;
  	CVhalMiscEventItem(const CVhalMiscEventItem& src) = delete;
	CVhalMiscEventItem& operator=(const CVhalMiscEventItem& src) & = default;
	CVhalMiscEventItem(CVhalMiscEventItem&& src) = delete;
	CVhalMiscEventItem& operator=(CVhalMiscEventItem&& src) & = delete;

	/* イベント処理の実施 */
	int32_t Exec(void) const override;

	/* データ設定 */
	void SetData(const uint32_t dataSize, const void* const data, const uint8_t data_type);

protected:
	CVhalMiconCommMiscControl *p_misc_control_;
	std::vector<uint8_t>     data_;
};

/*****************************************************************************
 クラス名称：CVhalMiscEventItemHdmiDetect
 処理概要  ：HDMI機器接続状態変更通知クラス
*****************************************************************************/
class CVhalMiscEventItemHdmiDetect : public CVhalMiscEventItem {
public:

	CVhalMiscEventItemHdmiDetect(CVhalMiconCommMiscControl * const p_misc_control)
		:CVhalMiscEventItem(p_misc_control)
	{
	}
	~CVhalMiscEventItemHdmiDetect(void) override = default;
  	CVhalMiscEventItemHdmiDetect(const CVhalMiscEventItemHdmiDetect& src) = delete;
	CVhalMiscEventItemHdmiDetect& operator=(const CVhalMiscEventItemHdmiDetect& src) & = default;
	CVhalMiscEventItemHdmiDetect(CVhalMiscEventItemHdmiDetect&& src) = delete;
	CVhalMiscEventItemHdmiDetect& operator=(CVhalMiscEventItemHdmiDetect&& src) & = delete;

	/* CVhalMiscEventItemからの差分が必要ならば記載する */
};

/*****************************************************************************
 クラス名称：CVhalMiscEventItemHdmiVideoFormat
 処理概要  ：HDMIビデオフォーマット変更通知クラス
*****************************************************************************/
class CVhalMiscEventItemHdmiVideoFormat : public CVhalMiscEventItem {
public:

	CVhalMiscEventItemHdmiVideoFormat(CVhalMiconCommMiscControl * const p_misc_control)
		:CVhalMiscEventItem(p_misc_control)
	{
	}
	~CVhalMiscEventItemHdmiVideoFormat(void) override = default;
  	CVhalMiscEventItemHdmiVideoFormat(const CVhalMiscEventItemHdmiVideoFormat& src) = delete;
	CVhalMiscEventItemHdmiVideoFormat& operator=(const CVhalMiscEventItemHdmiVideoFormat& src) & = default;
	CVhalMiscEventItemHdmiVideoFormat(CVhalMiscEventItemHdmiVideoFormat&& src) = delete;
	CVhalMiscEventItemHdmiVideoFormat& operator=(CVhalMiscEventItemHdmiVideoFormat&& src) & = delete;

	/* CVhalMiscEventItemからの差分が必要ならば記載する */
};

/*****************************************************************************
 クラス名称：CVhalMiscEventItemHdmiAudioFormat
 処理概要  ：HDMIオーディオフォーマット変更通知クラス
*****************************************************************************/
class CVhalMiscEventItemHdmiAudioFormat : public CVhalMiscEventItem {
public:
	CVhalMiscEventItemHdmiAudioFormat(CVhalMiconCommMiscControl * const p_misc_control)
		:CVhalMiscEventItem(p_misc_control)
	{
	}
	~CVhalMiscEventItemHdmiAudioFormat(void) override = default;
  	CVhalMiscEventItemHdmiAudioFormat(const CVhalMiscEventItemHdmiAudioFormat& src) = delete;
	CVhalMiscEventItemHdmiAudioFormat& operator=(const CVhalMiscEventItemHdmiAudioFormat& src) & = default;
	CVhalMiscEventItemHdmiAudioFormat(CVhalMiscEventItemHdmiAudioFormat&& src) = delete;
	CVhalMiscEventItemHdmiAudioFormat& operator=(CVhalMiscEventItemHdmiAudioFormat&& src) & = delete;

	/* CVhalMiscEventItemからの差分が必要ならば記載する */
};

/*****************************************************************************
 クラス名称：CVhalMiconCommMiscControl
 処理概要  ：マイコン間MISC通信制御を行う。
*****************************************************************************/
class CVhalMiconCommMiscControl {
public:
	CVhalMiconCommMiscControl(void);
	~CVhalMiconCommMiscControl(void) = default;
  	CVhalMiconCommMiscControl(const CVhalMiconCommMiscControl& src) = delete;
	CVhalMiconCommMiscControl& operator=(const CVhalMiconCommMiscControl& src) & = default;
	CVhalMiconCommMiscControl(CVhalMiconCommMiscControl&& src) = delete;
	CVhalMiconCommMiscControl& operator=(CVhalMiconCommMiscControl&& src) & = delete;

	int32_t Initialize(CVhalEventRoute* const p_event_route, CVhalSysdbControl* const p_sysdbctrl, void* const p_ccm_obj, const bool str_resume);
	void Finalize(void* const p_ccm_obj);
	void ReInit(void* const p_ccm_obj) noexcept;

	/* MISCデータ送信 */
	static int32_t Send(void* const p_ccm_obj, std::vector<uint8_t> &send_data);

	/* 内部イベントデータ受信CB登録 */
	int32_t RegistryReceiver(CVhalMiconReceiveItem* const p_rcv_item);

	/* 内部イベントデータ受信CB削除 */
	int32_t ClearReceiver(CVhalMiconReceiveItem* const p_rcv_item);

	/* 内部イベント通知 */
	void NotifyReceiveEvent(const uint32_t dataSize, const void* const data, const uint8_t data_type);

	/* 内部イベント処理 */
	void ExecReceiveEvent(const std::vector<uint8_t> recv_data);

	/* 通知イベント有効判定 */
	bool IsNotifyEnabled(const uint32_t data_size, const void* const data, const uint8_t data_type);

	/* MISC_ステータス受信 */
	void NotifyMiscCtrlSts(const uint32_t ctrl_sts);

	/* MISCコマンドデータ受信事前通知 */
	void NotifyReceivePre(const uint32_t data_size, const void* const data, const uint8_t data_type);

	/* MISC_Controlステータス取得 */
	uint32_t GetCtrlSts(void);

	/* イベントリスナー登録 */
	int32_t RegisterEventListener(CVhalMiscEventListenerBase* const p_listener);

	/* イベントリスナー解除 */
	void ClearEventListener(void) noexcept;

private:
	CVhalEventRoute* p_event_route_{nullptr};

	/* Display用通知イベント有効判定 */
	bool IsNotifyDisplayEnabled(std::vector<uint8_t>& cmd);

	mutable std::mutex mtx_recv_item_;

	/* ReceiveItemリスト */
	std::list<CVhalMiconReceiveItem*> receive_list_;

	/* MiscCtrlApiSts待ちイベント */
	std::mutex mtx_sts_sync_;
	std::condition_variable cond_sts_sync_;
	uint32_t	ctrl_sts_{MISC_CTRL_STS_NONE};

	/* HDMIフォーマット無効値 */
	static constexpr uint8_t kHdmiFormatOutOfSpec{0x80U};
	static constexpr uint8_t kHdmiFormatError    {0x81U};
	
	static constexpr uint8_t kHDMIFormatDataSize{2U};

	/* 比較用通知データ */
	std::vector<uint8_t>	cmd_camera_sync_;
	std::vector<uint8_t>	cmd_camera_type_;
	std::vector<uint8_t>	cmd_hdmi_video_format_;
	std::vector<uint8_t>	cmd_hdmi_audio_format_;
	std::vector<uint8_t>	cmd_dispmode_resp_;
	std::vector<uint8_t>	cmd_disp_hud_func_status_;				/* HUD機能ステータス */
	std::vector<uint8_t>	cmd_disp_hud_distortion_correction_;	/* HUD歪み補正パラメータ */
	std::vector<uint8_t>	cmd_disp_hud_rotation_;					/* HUD回転パラメータ */
};

extern "C" void MiconCommMiscControlStsCallBack(MiscCtrlApiSts* p_sts);
extern "C" void MiconCommMiscControlDataNtyCallBack(const UINT8 data_type, void* const p_cmd, const UINT32 len);

} /* namespace videohal */

#endif	/* #ifndef	VHAL_MICON_MISC_CONTROL_H */
