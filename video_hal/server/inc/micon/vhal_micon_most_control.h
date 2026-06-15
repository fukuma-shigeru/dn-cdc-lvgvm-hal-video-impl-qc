/*******************************************************************************
    機能名称    ：  マイコン間MOST通信制御モジュール
    ファイル名称：  vhal_micon_most_control.h
*******************************************************************************/
#ifndef	VHAL_MICON_MOST_CONTROL_H
#define	VHAL_MICON_MOST_CONTROL_H

#include <vector>
#include <list>
#include <mutex>
#include <memory>

#include "vhal_event_item_base.h"
#include "vhal_timer.h"
#include "vhal_layout_mng.h"

extern "C"
{
#include "com_stddef.h"
#include "ls3_ctl_api_public.h"
}

namespace videohal
{
class CVhalEventItemBase;
class CVhalMiconReceiveItem;
class CVhalMiconCommMostControl;

/* コマンドID一覧 */
namespace MostCmdType
{
	static constexpr uint32_t kPrevSourceActivity{1U};					/* SourceActivity(切替前) */
	static constexpr uint32_t kNextSourceActivity{2U};					/* SourceActivity(切替後) */
	static constexpr uint32_t kRearAvMasterCurrentVideoSource{3U};		/* RearAVMaster.CurrentVideoSource */
	static constexpr uint32_t kFrontAvMasterCurrentVideoSource{4U};		/* FrontAVMaster.CurrentVideoSource */
	static constexpr uint32_t kRearDisplayVideoMode{5U};				/* VideoMode */
	static constexpr uint32_t kRearDisplayVideoMute{6U};				/* RearDisplay.VideoMute */
	static constexpr uint32_t kStatusType{0x100U};								/* 切替結果通知あり種別 */
	static constexpr uint32_t kStatusVpathRearNotification{kStatusType + 1U};	/* 切替結果通知あり（後席専用映像反映通知） */
	static constexpr uint32_t kStatusMuteRearDisp{kStatusType + 2U};			/* 切替結果通知あり（後席ディスプレイ全体MUTE） */
}

/* Ls3Ctlで未定義のため定義する */
/* FblockID一覧 */
enum class MostFBlockId : uint8_t {
	kCurrentVideoSource				= 0xC0U,
	kRearDisplay					= 0x60U,
	kSourceActivityDtv				= 0xD0U,
	kSourceActivityMiracast			= 0xD2U,
	kSourceActivityHdmi				= 0xD3U,
	kSourceActivityBrowser			= 0xD4U,
	kSourceActivityMultiSensory		= 0xD4U,
	kSourceActivityApp				= 0xD9U,
	kSourceActivityVdsp				= 0xE0U,
	kSourceActivity					= 0xFFU,		/* SourceActivity. 映像パスに従う */
};

/* InstID一覧 */
enum class MostInstID : uint8_t {
	kFrontAVMaster					= 0x10U,
	kRearAVMaster					= 0x18U,
	kRearDisplay					= 0x76U,
	kSourceActivityHdmi				= 0x30U,
	kSourceActivityRearHdmi			= 0x32U,
	kSourceActivityRearHdmi2		= 0x34U,
	kSourceActivityDrc				= 0x36U,
	kSourceActivityMultiSensory		= 0x36U,
	kSourceActivityRearMiracast		= 0x38U,
	kSourceActivityRearBrowser		= 0x3AU,
	kSourceActivityApp				= 0x42U,
	kSourceActivityVdsp				= 0x50U,
	kSourceActivityRearVdsp			= 0x52U,
	kSourceActivityRearAudiosel		= 0x54U,
	kSourceActivity					= 0xFFU,		/* SourceActivity. 映像パスに従う */
};

/* SourceNr一覧 */
/* SourceID(VHAL_VSRC_ID_OTHER)互換のためuint32_tで定義 */
namespace MostSourceNr
{
	static constexpr uint32_t kRseRearHdmi{0x0BU};
	static constexpr uint32_t kRseRearHdmi2{0x0BU};
	static constexpr uint32_t kRseRearVdsp{0xBAU};
	static constexpr uint32_t kRseRearMiracast{0xBAU};
	static constexpr uint32_t kRseRearBrowser{0x0BU};
	static constexpr uint32_t kRseRearAudiosel{0xBAU};
}

/* FktID一覧 */
enum class MostFktID : uint16_t {
	kSourceActivity					= 0x103U,
	kVideoMute						= 0xC93U,
	kVideoMode						= 0xDF8U,
	kCurrentVideoSource				= 0xDF9U,
};

/* OPType一覧 */
enum class MostOPType : uint8_t {
	kSet							= 0x00U,
	kStatus							= 0x0CU,
	kStartResult					= 0x02U,
	kProcessing						= 0x0BU,
	kResult							= 0x0CU,
	kError							= 0x0FU,
};

/* Length一覧 */
enum class MostLength : uint16_t {
	kSourceActivity					= 2U,
	kVideoMode						= 16U,
	kCurrentVideoSource				= 3U,
	kVideoMute						= 1U,
};

/* 受信イベント種別 */
namespace MostRecvEvent
{
	static constexpr uint8_t kLs3Data{0U};			/* LS3CTL Callbackデータ */
	static constexpr uint8_t kResponsTimeout{1U};	/* LS3CTL 応答タイムアウト(10sec) */
	static constexpr uint8_t kRetryCountEnd{2U};	/* LS3CTL コマンド送信リトライ終了(200ms*3回) */
}

/* FblockID+InstID情報データ構造体 */
struct fblockidInstidInfo {
	MostFBlockId		fblockid_;			/* FblockID */
	MostInstID			instid_;			/* InstID */
	uint32_t			sourcenr_;			/* SourceNr */
};
using most_video_path_map = std::map<std::string, struct fblockidInstidInfo>;

/* 後席系映像パス情報変換用 */
const most_video_path_map most_rear_path_table_
{
	/* 映像パス名,				{FblockID,									InstID,										SourceNr}  */
	/* 後席映像パス変換用 */
	{"",						{MostFBlockId::kSourceActivityVdsp,			MostInstID::kSourceActivityRearVdsp,		VHAL_VSRC_ID_OTHER	} },
	{"DTV",						{MostFBlockId::kSourceActivityDtv,			MostInstID::kSourceActivityHdmi,			VHAL_VSRC_ID_DTV	} },
	{"HDMI",					{MostFBlockId::kSourceActivityHdmi,			MostInstID::kSourceActivityHdmi,			VHAL_VSRC_ID_HDMI	} },
	{"MULTISENSORY",			{MostFBlockId::kSourceActivityMultiSensory,	MostInstID::kSourceActivityMultiSensory,	VHAL_VSRC_ID_OTHER	} },
	/* 後席専用映像パス変換用 */
	{"REAR-MIRACAST",			{MostFBlockId::kSourceActivityMiracast,		MostInstID::kSourceActivityRearMiracast,	MostSourceNr::kRseRearMiracast	} },
	{"REAR-BROWSER",			{MostFBlockId::kSourceActivityBrowser,		MostInstID::kSourceActivityRearBrowser,		MostSourceNr::kRseRearBrowser	} },
	{"REAR-VDSP",				{MostFBlockId::kSourceActivityVdsp,			MostInstID::kSourceActivityRearVdsp,		MostSourceNr::kRseRearVdsp	} },
	{"REAR-AUDIOSEL",			{MostFBlockId::kSourceActivityVdsp,			MostInstID::kSourceActivityRearAudiosel,	MostSourceNr::kRseRearAudiosel	} },
	{"REAR-HDMI",				{MostFBlockId::kSourceActivityHdmi,			MostInstID::kSourceActivityRearHdmi,		MostSourceNr::kRseRearHdmi	} },
	{"REAR-HDMI2",				{MostFBlockId::kSourceActivityHdmi,			MostInstID::kSourceActivityRearHdmi2,		MostSourceNr::kRseRearHdmi2	} },
};

/* 送受信コマンド構造体 ヘッダ部 */
struct Ls3CtlApiDatHead
{
    uint16_t		deviceid_;			/* DeviceID */
	MostFBlockId	fblockid_;			/* FblockID */
	MostInstID		instid_;			/* InstID */
    MostFktID 		fktid_;				/* FktID */
    MostOPType 		optype_;			/* OPType */
    uint16_t		datatype_;			/* DataType */
    MostLength		length_;			/* Length */
} ;
using ls3ctl_cmd_head_map = std::map<uint32_t, struct Ls3CtlApiDatHead>;

/* コマンド情報変換用（ヘッダ部） */
const ls3ctl_cmd_head_map cmd_head_table_
{
	/* コマンド種別,								{DeviceID,						FblockID,							InstID,							FktID,							OPType,						DataType,					Length}  */
	{MostCmdType::kPrevSourceActivity,				{LS3_CTL_REGISTRATION_DEVICEID, MostFBlockId::kSourceActivity,		MostInstID::kSourceActivity,	MostFktID::kSourceActivity,		MostOPType::kStartResult,	LS3_CTL_TRANSFER_MEMORY,	MostLength::kSourceActivity	} },		/* SourceActivity(切替前) */
	{MostCmdType::kNextSourceActivity,				{LS3_CTL_REGISTRATION_DEVICEID, MostFBlockId::kSourceActivity,		MostInstID::kSourceActivity,	MostFktID::kSourceActivity,		MostOPType::kStartResult,	LS3_CTL_TRANSFER_MEMORY,	MostLength::kSourceActivity	} },		/* SourceActivity(切替後) */
	{MostCmdType::kRearAvMasterCurrentVideoSource,	{LS3_CTL_REGISTRATION_DEVICEID, MostFBlockId::kCurrentVideoSource,	MostInstID::kRearAVMaster,		MostFktID::kCurrentVideoSource,	MostOPType::kStatus,		LS3_CTL_TRANSFER_MEMORY,	MostLength::kCurrentVideoSource	} },	/* RearAVMaster.CurrentVideoSource */
	{MostCmdType::kFrontAvMasterCurrentVideoSource,	{LS3_CTL_REGISTRATION_DEVICEID, MostFBlockId::kCurrentVideoSource,	MostInstID::kFrontAVMaster,		MostFktID::kCurrentVideoSource,	MostOPType::kStatus,		LS3_CTL_TRANSFER_MEMORY,	MostLength::kCurrentVideoSource	} },	/* FrontAVMaster.CurrentVideoSource */
	{MostCmdType::kRearDisplayVideoMode,			{LS3_CTL_REGISTRATION_DEVICEID, MostFBlockId::kRearDisplay,			MostInstID::kRearDisplay,		MostFktID::kVideoMode,			MostOPType::kStartResult,	LS3_CTL_TRANSFER_MEMORY,	MostLength::kVideoMode	} },			/* VideoMode */
	{MostCmdType::kRearDisplayVideoMute,			{LS3_CTL_REGISTRATION_DEVICEID, MostFBlockId::kRearDisplay,			MostInstID::kRearDisplay,		MostFktID::kVideoMute,			MostOPType::kSet,			LS3_CTL_TRANSFER_MEMORY,	MostLength::kVideoMute	} },			/* VideoMute */
};

/*****************************************************************************
 クラス名称：CVhalMiconMostRecvEventItem
 処理概要  ：MOST通信イベントクラス
*****************************************************************************/
class CVhalMiconMostRecvEventItem : public CVhalEventItemBase {
public:

	CVhalMiconMostRecvEventItem(CVhalMiconCommMostControl * const p_most_control) noexcept
		:p_most_control_{p_most_control}
		,ls3_data_{}
	{
	}
	~CVhalMiconMostRecvEventItem(void) override = default;
  	CVhalMiconMostRecvEventItem(const CVhalMiconMostRecvEventItem& src) = delete;
	CVhalMiconMostRecvEventItem& operator=(const CVhalMiconMostRecvEventItem& src) & = default;
	CVhalMiconMostRecvEventItem(CVhalMiconMostRecvEventItem&& src) = delete;
	CVhalMiconMostRecvEventItem& operator=(CVhalMiconMostRecvEventItem&& src) & = delete;

	/* イベント処理の実施 */
	int32_t Exec(void) const override;

	/* データ設定 */
	void SetData(const Ls3CtlApiDat &recv_data) noexcept;

private:
	CVhalMiconCommMostControl*		p_most_control_;
	Ls3CtlApiDat					ls3_data_;
};

/*****************************************************************************
 クラス名称：CVhalMostSendTimer
 処理概要  ：定期送信用タイマ処理
*****************************************************************************/
class CVhalMostSendTimer : public CVhalTimer {
public:

	CVhalMostSendTimer(CVhalMiconCommMostControl* const p_most_ctrl) noexcept;
	~CVhalMostSendTimer(void) override = default;
  	CVhalMostSendTimer(const CVhalMostSendTimer& src) = delete;
	CVhalMostSendTimer& operator=(const CVhalMostSendTimer& src) & = default;
	CVhalMostSendTimer(CVhalMostSendTimer&& src) = delete;
	CVhalMostSendTimer& operator=(CVhalMostSendTimer&& src) & = delete;

private:
	/* タイマー満了実装 */
	int32_t OnTimerImpl(const bool time_cycle_enable) const override;

	CVhalMiconCommMostControl* p_most_control_;

};

/*****************************************************************************
 クラス名称：CVhalMiconCommMostControl
 処理概要  ：マイコン間AVC-LANstep3通信制御を行う。
*****************************************************************************/
class CVhalMiconCommMostControl final {
public:
	CVhalMiconCommMostControl(void) noexcept;
	~CVhalMiconCommMostControl(void) = default;
  	CVhalMiconCommMostControl(const CVhalMiconCommMostControl& src) = delete;
	CVhalMiconCommMostControl& operator=(const CVhalMiconCommMostControl& src) & = default;
	CVhalMiconCommMostControl(CVhalMiconCommMostControl&& src) = delete;
	CVhalMiconCommMostControl& operator=(CVhalMiconCommMostControl&& src) & = delete;

	int32_t Initialize(CVhalEventRoute* const p_event_route, CVhalLayoutManager * const p_layout_mng);
	void Finalize(void);
	void ReInit(void) noexcept;

	int32_t InitializeLs3Ctl(void);
	void FinalizeLs3Ctl(void);

	int32_t Send(std::vector<uint8_t> &data);
	static void Ls3CtlCallback(const int32_t event, const uint8_t fBlockID, const uint8_t instID, const Ls3CtlApiDat& data);

	/* コマンド送信繰り返し終了 */
	void NotifySendRetryCountEnd(void);

	/* 内部イベントデータ受信CB登録 */
	int32_t RegistryReceiver(CVhalMiconReceiveItem* const p_rcv_item);

	/* 内部イベントデータ受信CB削除 */
	int32_t ClearReceiver(CVhalMiconReceiveItem* const p_rcv_item);

	/* 内部イベント通知 */
	void NotifyReceiveEvent(const Ls3CtlApiDat &recv_data);

	/* 内部イベント処理 */
	void ExecReceiveEventLs3Data(const uint8_t recv_event, const Ls3CtlApiDat &recv_data);
	void ExecReceiveEvent(const uint8_t recv_event);

private:
	CVhalEventRoute*				p_route_;
	CVhalLayoutManager*				p_layout_mng_;
	std::unique_ptr<CVhalMostSendTimer>		p_most_send_timer_;
	Ls3CtlApiObj					ls3ctl_obj_;
	Ls3CtlApiDat					send_data_;
	mutable std::mutex				mtx_recv_item_;
	bool							ls3ctl_initialized_;

	/* ReceiveItemリスト */
	std::list<CVhalMiconReceiveItem*> receive_list_;

	int32_t Open(void);
	void Close(void);
	int32_t RegistryCallBack(void);
	void ClearCallBack(void);
};

extern "C" void MiconCommMostControlLs3CtlCallback(const int32_t event, const uint8_t FBlockID , const uint8_t InstID, const Ls3CtlApiDat* const data);

} /* namespace videohal */

#endif	/* #ifndef	VHAL_MICON_MOST_CONTROL_H */
