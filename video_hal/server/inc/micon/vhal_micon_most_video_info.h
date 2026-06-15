/*******************************************************************************
    機能名称    ：  MOST映像情報通知送信モジュール
    ファイル名称：  vhal_micon_most_video_info.h
*******************************************************************************/
#ifndef	VHAL_MICON_MOST_VIDEO_INFO_H
#define	VHAL_MICON_MOST_VIDEO_INFO_H

#include <string>
#include "vhal_micon_send_item.h"
#include "vhal_micon_most_control.h"
#include "vhal_micon_receive_item.h"
#include "vhal_net_sequence_manager.h"

extern "C"
{
}

namespace videohal
{
class CVhalMiconReceiveItem;
class CVhalMiconCommControl;
class CVhalMiconMostSendItemVideoInfo;
class CVhalMiconMostVideoInfoReceiver;
class CVhalMiconMostReceiveEventListenerBase;

/* RSE接続状態（フルRSE 又は用品RSE接続） */
enum class MostConnectedState : uint32_t {
	kDisConnected,								/* RSE未接続/脱落 */
	kConnectedFirst,							/* RSE接続（初回） */
	kReConnected,								/* RSE接続（脱落復帰） */
};

/* シーケンスID一覧 */
enum class MostSequenceType : uint32_t {
	kSetFrontPath,								/* 前席映像パス切替設定 */
	kSetRsePathNotify,							/* 後席専用映像反映通知 */
	kColdBoot,									/* +B起動 */
	kRecoveryRse,								/* RSE脱落復帰 */
	kSetMuteRearDisp,							/* 後席ディスプレイ全体MUTE設定 */
	kMax,										/* 最大値 */
};

/* Updateタイプ */
enum class MostUpdateType : uint32_t {
	kNotifyCallbackRse,							/* RSEからのコールバック通知 */
	kSetProperty,								/* プロパティ設定 */
};

/*****************************************************************************
 クラス名称：CVhalMiconMostCmdItemSourceActivity
 処理概要  ：通信コマンドアイテム管理を行う。
            SourceActivity.StartResult
*****************************************************************************/
class CVhalMiconMostCmdItemSourceActivity : public CVhalNetCommandItem {
public:

	CVhalMiconMostCmdItemSourceActivity(const uint32_t command_id, const std::string& rear_rse_disp_path) noexcept;
	~CVhalMiconMostCmdItemSourceActivity(void) override = default;
  	CVhalMiconMostCmdItemSourceActivity(const CVhalMiconMostCmdItemSourceActivity& src) = delete;
	CVhalMiconMostCmdItemSourceActivity& operator=(const CVhalMiconMostCmdItemSourceActivity& src) & = default;
	CVhalMiconMostCmdItemSourceActivity(CVhalMiconMostCmdItemSourceActivity&& src) = delete;
	CVhalMiconMostCmdItemSourceActivity& operator=(CVhalMiconMostCmdItemSourceActivity&& src) & = delete;

	/* 送信データ構築 */
	void Build(std::vector<uint8_t> &send_data) const noexcept override;
	/* 応答コマンドあり/なし種別取得 */
	bool GetResponseType(void) const noexcept override;

private:

	/* 送信データ バイト番号 */
	static constexpr uint32_t	kPosSourceNr{0U};					/* SourceNr */
	static constexpr uint32_t	kPosActivityType{1U};				/* Activity設定 */

	static constexpr uint8_t	kActivityTypeOFF{0U};				/* Activity設定:OFF */
	static constexpr uint8_t	kActivityTypeON{2U};				/* Activity設定:ON */

	std::string		video_rear_rse_disp_path_;
};

/*****************************************************************************
 クラス名称：CVhalMiconMostCmdItemCurrentVideoSource
 処理概要  ：通信コマンドアイテム管理を行う。
            CurrentVideoSource.Status
*****************************************************************************/
class CVhalMiconMostCmdItemCurrentVideoSource : public CVhalNetCommandItem {
public:

	CVhalMiconMostCmdItemCurrentVideoSource(const uint32_t command_id, const std::string& path, const std::string& rear_rse_disp_path) noexcept;
	~CVhalMiconMostCmdItemCurrentVideoSource(void) override = default;
  	CVhalMiconMostCmdItemCurrentVideoSource(const CVhalMiconMostCmdItemCurrentVideoSource& src) = delete;
	CVhalMiconMostCmdItemCurrentVideoSource& operator=(const CVhalMiconMostCmdItemCurrentVideoSource& src) & = default;
	CVhalMiconMostCmdItemCurrentVideoSource(CVhalMiconMostCmdItemCurrentVideoSource&& src) = delete;
	CVhalMiconMostCmdItemCurrentVideoSource& operator=(CVhalMiconMostCmdItemCurrentVideoSource&& src) & = delete;

	/* 送信データ構築 */
	void Build(std::vector<uint8_t> &send_data) const noexcept override;
	/* 応答コマンドあり/なし種別取得 */
	bool GetResponseType(void) const noexcept override;

private:

	/* 送信データ バイト番号 */
	static constexpr uint32_t	kPosFblock{0U};						/* FblockID */
	static constexpr uint32_t	kPosInstID{1U};						/* InstID */
	static constexpr uint32_t	kPosSourceNr{2U};					/* SourceNr */

	std::string		video_path_;
	std::string		video_rear_rse_disp_path_;
};

/*****************************************************************************
 クラス名称：CVhalMiconMostCmdItemVideoMode
 処理概要  ：通信コマンドアイテム管理を行う。
            VideoMode.StartResult
*****************************************************************************/
class CVhalMiconMostCmdItemVideoMode : public CVhalNetCommandItem {
public:

	CVhalMiconMostCmdItemVideoMode(const uint32_t command_id, const std::string& rear_path, const std::string& rear_rse_disp_path, const int32_t w, const int32_t h) noexcept;
	~CVhalMiconMostCmdItemVideoMode(void) override = default;
  	CVhalMiconMostCmdItemVideoMode(const CVhalMiconMostCmdItemVideoMode& src) = delete;
	CVhalMiconMostCmdItemVideoMode& operator=(const CVhalMiconMostCmdItemVideoMode& src) & = default;
	CVhalMiconMostCmdItemVideoMode(CVhalMiconMostCmdItemVideoMode&& src) = delete;
	CVhalMiconMostCmdItemVideoMode& operator=(CVhalMiconMostCmdItemVideoMode&& src) & = delete;

	/* 送信データ構築 */
	void Build(std::vector<uint8_t> &send_data) const noexcept override;
	/* 応答コマンドあり/なし種別取得 */
	bool GetResponseType(void) const noexcept override;

private:

	/* 送信データ バイト番号 */
	static constexpr uint32_t	kPosFblock{0U};						/* FblockID */
	static constexpr uint32_t	kPosInstID{1U};						/* InstID */
	static constexpr uint32_t	kPosCompositeFblock{2U};			/* 合成映像.FblockID */
	static constexpr uint32_t	kPosCompositeInstID{3U};			/* 合成映像.InstID */
	static constexpr uint32_t	kPosScreenWHigh{12U};				/* ディスプレイサイズ幅 - 上位 */
	static constexpr uint32_t	kPosScreenWLow{13U};				/* ディスプレイサイズ幅 - 下位 */
	static constexpr uint32_t	kPosScreenHHigh{14U};				/* ディスプレイサイズ高さ - 上位 */
	static constexpr uint32_t	kPosScreenHLow{15U};				/* ディスプレイサイズ高さ - 下位 */

	std::string		video_rear_path_;
	std::string		video_rear_rse_disp_path_;

	/* 後席スクリーンサイズ */
	int32_t			rear_screen_w_;
	int32_t			rear_screen_h_;
};

/*****************************************************************************
 クラス名称：CVhalMiconMostCmdItemStatusVpathRearNotification
 処理概要  ：通信コマンドアイテム管理を行う。
            StatusVpathRearNotification
*****************************************************************************/
class CVhalMiconMostCmdItemStatusVpathRearNotification : public CVhalNetCommandItem {
public:

	using CVhalNetCommandItem::CVhalNetCommandItem;
	~CVhalMiconMostCmdItemStatusVpathRearNotification(void) override = default;
  	CVhalMiconMostCmdItemStatusVpathRearNotification(const CVhalMiconMostCmdItemStatusVpathRearNotification& src) = delete;
	CVhalMiconMostCmdItemStatusVpathRearNotification& operator=(const CVhalMiconMostCmdItemStatusVpathRearNotification& src) & = default;
	CVhalMiconMostCmdItemStatusVpathRearNotification(CVhalMiconMostCmdItemStatusVpathRearNotification&& src) = delete;
	CVhalMiconMostCmdItemStatusVpathRearNotification& operator=(CVhalMiconMostCmdItemStatusVpathRearNotification&& src) & = delete;

	/* 送信データ構築 */
	void Build(std::vector<uint8_t> &send_data) const noexcept override;
	/* 応答コマンドあり/なし種別取得 */
	bool GetResponseType(void) const noexcept override;

private:

};

/*****************************************************************************
 クラス名称：CVhalMiconMostCmdItemVideoMute
 処理概要  ：通信コマンドアイテム管理を行う。
            VideoMute.Set
*****************************************************************************/
class CVhalMiconMostCmdItemVideoMute : public CVhalNetCommandItem {
public:

	CVhalMiconMostCmdItemVideoMute(const uint32_t command_id, const bool mute) noexcept;
	~CVhalMiconMostCmdItemVideoMute(void) override = default;
  	CVhalMiconMostCmdItemVideoMute(const CVhalMiconMostCmdItemVideoMute& src) = delete;
	CVhalMiconMostCmdItemVideoMute& operator=(const CVhalMiconMostCmdItemVideoMute& src) & = default;
	CVhalMiconMostCmdItemVideoMute(CVhalMiconMostCmdItemVideoMute&& src) = delete;
	CVhalMiconMostCmdItemVideoMute& operator=(CVhalMiconMostCmdItemVideoMute&& src) & = delete;

	/* 送信データ構築 */
	void Build(std::vector<uint8_t> &send_data) const noexcept override;
	/* 応答コマンドあり/なし種別取得 */
	bool GetResponseType(void) const noexcept override;

private:

	/* 送信データ バイト番号 */
	static constexpr uint32_t	kPosMute{0U};							/* Mute設定 */

	bool			mute_rear_disp_;
};

/*****************************************************************************
 クラス名称：CVhalMiconMostCmdItemStatusMuteRearDisp
 処理概要  ：通信コマンドアイテム管理を行う。
            StatusMuteRearDisp
*****************************************************************************/
class CVhalMiconMostCmdItemStatusMuteRearDisp : public CVhalNetCommandItem {
public:

	CVhalMiconMostCmdItemStatusMuteRearDisp(const uint32_t command_id, const bool mute) noexcept;
	~CVhalMiconMostCmdItemStatusMuteRearDisp(void) override = default;
  	CVhalMiconMostCmdItemStatusMuteRearDisp(const CVhalMiconMostCmdItemStatusMuteRearDisp& src) = delete;
	CVhalMiconMostCmdItemStatusMuteRearDisp& operator=(const CVhalMiconMostCmdItemStatusMuteRearDisp& src) & = default;
	CVhalMiconMostCmdItemStatusMuteRearDisp(CVhalMiconMostCmdItemStatusMuteRearDisp&& src) = delete;
	CVhalMiconMostCmdItemStatusMuteRearDisp& operator=(CVhalMiconMostCmdItemStatusMuteRearDisp&& src) & = delete;

	/* 送信データ構築 */
	void Build(std::vector<uint8_t> &send_data) const noexcept override;
	/* 応答コマンドあり/なし種別取得 */
	bool GetResponseType(void) const noexcept override;
	/* Mute取得 */
	bool GetMute(void) const noexcept;

private:

	bool			mute_rear_disp_;
};

/*****************************************************************************
 クラス名称：CVhalMiconMostSendItemVideoInfo
 処理概要  ：MOST映像情報通知送信アイテム
*****************************************************************************/
class CVhalMiconMostSendItemVideoInfo : public CVhalMiconSendItem {
public:

	CVhalMiconMostSendItemVideoInfo(void) noexcept = default;
	~CVhalMiconMostSendItemVideoInfo(void) override = default;
  	CVhalMiconMostSendItemVideoInfo(const CVhalMiconMostSendItemVideoInfo& src) = delete;
	CVhalMiconMostSendItemVideoInfo& operator=(const CVhalMiconMostSendItemVideoInfo& src) & = default;
	CVhalMiconMostSendItemVideoInfo(CVhalMiconMostSendItemVideoInfo&& src) = delete;
	CVhalMiconMostSendItemVideoInfo& operator=(CVhalMiconMostSendItemVideoInfo&& src) & = delete;

	/* 送信データ構築 */
	int32_t Build(std::vector<uint8_t> &send_data) const noexcept override;

	/* 送信アイテム種別取得 */
	SendItemType GetItemType(void) const noexcept override;

	/* コマンドアイテム設定 */
	void SetCommandItem(CVhalNetCommandItem* const item) noexcept;

private:

	CVhalNetCommandItem*	p_command_item_{nullptr};
};

/*****************************************************************************
 クラス名称：CVhalMiconMostReceiveEventListenerBase
 処理概要  ：VhalMiconMostReceiveイベントリスナベース。
*****************************************************************************/
class CVhalMiconMostReceiveEventListenerBase {
public:
	CVhalMiconMostReceiveEventListenerBase(void) noexcept = default;
	virtual ~CVhalMiconMostReceiveEventListenerBase(void) = default;
  	CVhalMiconMostReceiveEventListenerBase(const CVhalMiconMostReceiveEventListenerBase& src) = delete;
	CVhalMiconMostReceiveEventListenerBase(CVhalMiconMostReceiveEventListenerBase&& src) = delete;

	virtual void NotifyReceiveMostRsePathResult(const uint32_t result, const enum MostUpdateType update_type) const noexcept = 0;
	virtual void NotifyReceiveMostMuteRearDispResult(const bool mute, const enum MostUpdateType update_type) const noexcept = 0;
private:
	CVhalMiconMostReceiveEventListenerBase& operator=(const CVhalMiconMostReceiveEventListenerBase& src) & = delete;
	CVhalMiconMostReceiveEventListenerBase& operator=(CVhalMiconMostReceiveEventListenerBase&& src) & = delete;
};

/*****************************************************************************
 クラス名称：CVhalMiconMostVideoInfo
 処理概要  ：MOST映像情報送信を行う。
*****************************************************************************/
class CVhalMiconMostVideoInfo final {
public:
	CVhalMiconMostVideoInfo(void) noexcept;
	~CVhalMiconMostVideoInfo(void);
  	CVhalMiconMostVideoInfo(const CVhalMiconMostVideoInfo& src) = delete;
	CVhalMiconMostVideoInfo& operator=(const CVhalMiconMostVideoInfo& src) & = default;
	CVhalMiconMostVideoInfo(CVhalMiconMostVideoInfo&& src) = delete;
	CVhalMiconMostVideoInfo& operator=(CVhalMiconMostVideoInfo&& src) & = delete;

	int32_t Initialize(CVhalMiconCommControl* const p_micon_comm_control, CVhalMiconMostReceiveEventListenerBase* const p_listener);
	void Finalize(void);
	void ReInit(void) noexcept;

	/* 前席映像パス設定 */
	int32_t SetFrontPath(const std::string& path);
	/* 後席映像パス設定 */
	void SetRearPath(const std::string& path);
	/* 後席専用映像パス設定 */
	int32_t SetRearRseDispPath(const std::string& path);
	/* 後席映像反映通知 */
	int32_t Reflect(void);
	/* RSE接続通知 */
	int32_t SetConnectedRse(const enum MostConnectedState connected_state) noexcept;
	/* 後席ディスプレイ出力矩形設定 */
	void SetRearDisplayRectangle(const int32_t width, const int32_t height) noexcept;
	/* 後席ディスプレイ全体のMUTE設定 */
	void SetMuteRearDisp(const bool mute) noexcept;
	/* コマンド送信（メイン） */
	int32_t SendCmd(const enum MostUpdateType update_type) noexcept;
	/* カレントシーケンスアイテムクリア */
	std::unique_ptr<CVhalNetCommandItem> ClearCurrentSequenceItem(void) noexcept;

private:

	/* シーケンスアイテム送信 */
	int32_t SendSequenceItem(const enum MostSequenceType sequence_id) noexcept;
	/* シーケンスアイテム生成 */
	std::unique_ptr<CVhalNetSequenceItem> CreateSequenceItem(const enum MostSequenceType sequence_id) noexcept;
	/* コマンドアイテム生成 */
	int32_t CreateCommandItem(const enum MostSequenceType sequence_id, CVhalNetSequenceItem* const p_sequence_item) noexcept;
	/* コマンド送信（未応答リトライあり） */
	int32_t SendCmdInterval(CVhalNetCommandItem * const p_command_item) noexcept;
	/* コマンド送信（未応答リトライなし） */
	int32_t SendCmdOneshot(CVhalNetCommandItem * const p_command_item) noexcept;

	std::string		video_front_path_;
	std::string		current_video_rear_path_;
	std::string		current_video_rear_rse_disp_path_;

	/* 前回値保持（後席/後席専用は一括適用のため、設定タイミングと適用タイミングが異なる） */
	std::string		prev_video_rear_path_;
	std::string		prev_video_rear_rse_disp_path_;

	/* 後席スクリーンサイズ */
	int32_t			rear_screen_w_;
	int32_t			rear_screen_h_;
	/* 後席ディスプレイ全体MUTE設定 */
	bool			mute_rear_disp_;

	CVhalMiconCommControl*				p_micon_ctrl_;
	std::unique_ptr<CVhalMiconMostSendItemVideoInfo>	p_most_send_item_videoinfo_;
	std::unique_ptr<CVhalMiconMostVideoInfoReceiver>	p_most_video_info_receiver_;
	std::unique_ptr<CVhalNetSequenceManager>			p_net_sequence_manager_;
	std::unique_ptr<CVhalNetSequenceItem>				p_current_sequence_item_;
};

/*****************************************************************************
 クラス名称：CVhalMiconMostVideoInfoReceiver
 処理概要  ：MOST通信イベントクラス
*****************************************************************************/
class CVhalMiconMostVideoInfoReceiver : public CVhalMiconReceiveItem {
public:

	CVhalMiconMostVideoInfoReceiver(CVhalMiconMostVideoInfo* const p_most_video_info, CVhalMiconCommControl* const p_micon_comm_control, CVhalMiconMostSendItemVideoInfo* const p_send_item) noexcept;
	CVhalMiconMostVideoInfoReceiver(void) = default;
	~CVhalMiconMostVideoInfoReceiver(void) override;
  	CVhalMiconMostVideoInfoReceiver(const CVhalMiconMostVideoInfoReceiver& src) = delete;
	CVhalMiconMostVideoInfoReceiver& operator=(const CVhalMiconMostVideoInfoReceiver& src) & = default;
	CVhalMiconMostVideoInfoReceiver(CVhalMiconMostVideoInfoReceiver&& src) = delete;
	CVhalMiconMostVideoInfoReceiver& operator=(CVhalMiconMostVideoInfoReceiver&& src) & = delete;

	/* データ受信（VideoHALイベントスレッドからのコール） */
	void Receive(const std::vector<uint8_t>& data) override;

	/* データ受信事前通知（通信モジュールスレッドからのコール） */
	void ReceivePreNotify(const std::vector<uint8_t>& data) noexcept override;

	/* 受信アイテム種別取得 */
	ReceiveItemType GetItemType(void) const noexcept override;

	/* イベントリスナー登録 */
	int32_t RegisterEventListener(CVhalMiconMostReceiveEventListenerBase* const p_listener);
	/* イベントリスナー解除 */
	void ClearEventListener(void) noexcept;
	void ReceiveEvent(const uint32_t result, CVhalNetCommandItem* const p_command_item, const enum MostUpdateType update_type) const noexcept;

private:
	CVhalMiconMostReceiveEventListenerBase*		p_most_video_info_listener_;
	CVhalMiconMostVideoInfoReceiver*			p_most_video_info_receiver_;
	CVhalMiconMostVideoInfo*					p_most_videopath_;
	CVhalMiconCommControl*						p_micon_ctrl_;
	CVhalMiconMostSendItemVideoInfo*			p_most_video_info_send_item_;

	void ReceiveLs3Data(const Ls3CtlApiDat &recv_data);
};

} /* namespace videohal */

#endif	/* #ifndef	VHAL_MICON_MOST_VIDEO_INFO_H */
