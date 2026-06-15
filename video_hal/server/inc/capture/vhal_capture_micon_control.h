/*******************************************************************************
    機能名称    ：  マイコン間通信処理モジュール(キャプチャ)
    ファイル名称：  vhal_capture_micon_control.h
*******************************************************************************/
#ifndef	VHAL_CAPTURE_MICON_CONTROL_H
#define	VHAL_CAPTURE_MICON_CONTROL_H

#include <cstdint>
#include <vector>
#include <map>
#include <unordered_map>

#include "vhal_micon_receive_item.h"
#include "vhal_micon_send_item.h"

namespace videohal
{

/* カメラ同期 */
enum class VhalTabCamSync : uint8_t {
	kSyncNo  = 0x00,	/* No Sync */
	kSync    = 0x01,	/* Sync */
	kUnknown = 0xff		/* Pending or Invalid */
};

/*****************************************************************************
 クラス名称：CVhalCaptureTabReceiveEventListenerBase
 処理概要  ：VhalCaptureTabReceiveイベントリスナベース。
*****************************************************************************/
class CVhalCaptureTabReceiveEventListenerBase {
public:
	CVhalCaptureTabReceiveEventListenerBase(void) noexcept = default;
	virtual ~CVhalCaptureTabReceiveEventListenerBase(void) = default;
  	CVhalCaptureTabReceiveEventListenerBase(const CVhalCaptureTabReceiveEventListenerBase& src) = delete;
	CVhalCaptureTabReceiveEventListenerBase(CVhalCaptureTabReceiveEventListenerBase&& src) = delete;


	virtual void NotifyTabReceiveCameraSync(const VhalTabCamSync sync) const noexcept = 0;
	virtual void NotifyTabSetCameraInputSize(const int32_t disp) const noexcept = 0;
private:
	CVhalCaptureTabReceiveEventListenerBase& operator=(const CVhalCaptureTabReceiveEventListenerBase& src) & = delete;
	CVhalCaptureTabReceiveEventListenerBase& operator=(CVhalCaptureTabReceiveEventListenerBase&& src) & = delete;
};

/*****************************************************************************
 クラス名称：CVhalCaptureTabReceiver
 処理概要  ：TAB通信イベントクラス
*****************************************************************************/
class CVhalCaptureTabReceiver : public CVhalMiconReceiveItem {
public:

	CVhalCaptureTabReceiver(void);
	~CVhalCaptureTabReceiver(void) override;
	CVhalCaptureTabReceiver(const CVhalCaptureTabReceiver& src) = delete;
	CVhalCaptureTabReceiver& operator=(const CVhalCaptureTabReceiver& src) & = delete;
	CVhalCaptureTabReceiver(CVhalCaptureTabReceiver&& src) = delete;
	CVhalCaptureTabReceiver& operator=(CVhalCaptureTabReceiver&& src) & = delete;

	/* 再初期化処理 */
	void ReInit(void) noexcept;

	/* データ受信（VideoHALイベントスレッドからのコール） */
	void Receive(const std::vector<uint8_t>& data) override;

	/* データ受信事前通知（通信モジュールスレッドからのコール） */
	void ReceivePreNotify(const std::vector<uint8_t>& data) noexcept override;

	/* 受信アイテム種別取得 */
	ReceiveItemType GetItemType(void) const noexcept override;

	/* イベントリスナー登録 */
	int32_t RegisterEventListener(CVhalCaptureTabReceiveEventListenerBase* const p_listener);
	/* イベントリスナー解除 */
	void ClearEventListener(void) noexcept;

	/* カメラ映像パス設定時の同期状態更新 */
	bool UpdateCameraPathSync(const VhalTabCamSync state);
	/* カメラ同期の取得 */
	VhalTabCamSync GetCameraSync(void) const noexcept;

	/* 表示用映像サイズの取得 */
	int32_t GetCameraInputSize(const int32_t disp, int32_t &w, int32_t &h) const;
	/* カメラ映像表示要求状況(センターディスプレイ)の取得 */
	bool GetCameraCenterDisplayState(void) const;

private:
	static constexpr uint32_t	kTabRecvDataSizeMin{2U};	/* data_type+sub_type */
	static constexpr uint32_t	kTabRecvDataOpc{0U};

	/* 同期検知・経路情報通知(BEYE/FEYE) */
	// MISRA C++-2008 Rule 0-1-4 static constexpr uint32_t	kTabRecvDataSizeCamSync{5U};
	static constexpr uint32_t	kTabRecvDataCamSync{1U};
	static constexpr uint32_t	kTabRecvDataVicStatus2{5U};

	/* カメラ種別判別通知 */
	static constexpr uint32_t	kTabRecvDataSizeCamType{6U};
	// MISRA C++-2008 Rule 0-1-4 static constexpr uint32_t	kTabRecvDataCamType{1U};
	static constexpr uint32_t	kTabRecvDataCenterCamSize{2U};
	// MISRA C++-2008 Rule 0-1-4 static constexpr uint32_t	kTabRecvDataMeterCamSize{3U};
	static constexpr uint32_t	kTabRecvDataCenterCamReq{4U};
	// MISRA C++-2008 Rule 0-1-4 static constexpr uint32_t	kTabRecvDataMeterCamReq{5U};
	/* カメラ映像表示要求状況 */
	static constexpr uint8_t	kTabCamReqUnknown{0U};
	// MISRA C++-2008 Rule 0-1-4 static constexpr uint8_t	kTabCamReqOff{1U};
	static constexpr uint8_t	kTabCamReqOn{2U};
	/* V-IC-STATUS2信号初期値 */
	static constexpr uint8_t	kTabRecvDataVicStatusInit{0xFFU};

	/* 表示用映像サイズ(TAB2側で定義された場合は置き換える) */
	enum class CameraSizeMode : uint8_t {
		CAM_SIZE_UNKNOWN	= 0x00,	/* 0h:カメラシステム未判別 */
		CAM_SIZE_1920_1080	= 0x01,	/* 1h:1920*1080出力 */
		CAM_SIZE_1920_954	= 0x02,	/* 2h:1920*954出力  */
		CAM_SIZE_1920_900	= 0x03,	/* 3h:1920*900出力  */
		CAM_SIZE_1920_720	= 0x04,	/* 4h:1920*720出力  */
		CAM_SIZE_1280_846	= 0x05,	/* 5h:1280*846出力  */
		CAM_SIZE_1280_720	= 0x06,	/* 6h:1280*720出力  */
		CAM_SIZE_1280_621	= 0x07,	/* 7h:1280*621出力  */
		CAM_SIZE_1696_954	= 0x08,	/* 8h:1696*954出力  */
		CAM_SIZE_1104_621	= 0x09,	/* 9h:1104*621出力  */
		CAM_SIZE_1672_720	= 0x0A,	/* Ah:1672*720出力  */
		CAM_SIZE_MAX        = 0x0B,
	};

	struct VhalCameraSizeData {
		int32_t	width;
		int32_t	height;
	};

	static const std::unordered_map<CameraSizeMode, struct VhalCameraSizeData> cam_size_entries_;

	VhalTabCamSync			camera_path_sync_status_;	/*カメラ映像パス設定時の同期状態 */
	VhalTabCamSync			camera_sync_status_;		/*カメラ同期状態 */
	uint8_t					camera_vic_status_;			/*V-IC-STATUS2信号 */
	uint8_t					center_display_status_;
	std::vector<uint8_t>	camera_size_;
	std::vector<uint8_t>	prev_camera_type_;

	CVhalCaptureTabReceiveEventListenerBase	*p_capture_tab_recv_listener_;
	CVhalCaptureTabReceiver					*p_capture_tab_receiver_;

	/* カメラ同期通知 */
	void SetCameraSync(const std::vector<uint8_t>& data) noexcept;
	/* カメラ種別判別通知 */
	void SetCameraType(const std::vector<uint8_t>& data);
};

/*****************************************************************************
 クラス名称：CVhalCaptureTabSendItem
 処理概要  ：カメラ種別判別要求送信クラス
*****************************************************************************/
class CVhalCaptureTabSendItem : public CVhalMiconSendItem {
public:

	CVhalCaptureTabSendItem(void);
	~CVhalCaptureTabSendItem(void) override = default;
  	CVhalCaptureTabSendItem(const CVhalCaptureTabSendItem& src) = delete;
	CVhalCaptureTabSendItem& operator=(const CVhalCaptureTabSendItem& src) & = delete;
	CVhalCaptureTabSendItem(CVhalCaptureTabSendItem&& src) = delete;
	CVhalCaptureTabSendItem& operator=(CVhalCaptureTabSendItem&& src) & = delete;

	/* 送信データ構築 */
	int32_t Build(std::vector<uint8_t> &send_data) const override;

	/* 送信アイテム種別取得 */
	SendItemType GetItemType(void) const noexcept override;

	/* カメラ種別判別通知要求 */
	void SetTabCmdCameraTypeNotify(void);

private:
	uint8_t					opc_;
	std::vector<uint8_t>	data_;

	/* OPC設定 */
	void SetTabCmdCode(const uint8_t opc) noexcept;
	/* 送信データ設定 */
	void SetTabData(std::vector<uint8_t> &data);
};

/*****************************************************************************
 クラス名称：CVhalCameraModeSendItem
 処理概要  ：カメラ映像モード通知送信クラス
*****************************************************************************/
class CVhalCameraModeSendItem : public CVhalMiconSendItem {
public:

	CVhalCameraModeSendItem(void);
	~CVhalCameraModeSendItem(void) override = default;
  	CVhalCameraModeSendItem(const CVhalCameraModeSendItem& src) = delete;
	CVhalCameraModeSendItem& operator=(const CVhalCameraModeSendItem& src) & = delete;
	CVhalCameraModeSendItem(CVhalCameraModeSendItem&& src) = delete;
	CVhalCameraModeSendItem& operator=(CVhalCameraModeSendItem&& src) & = delete;

	/* 送信データ構築 */
	int32_t Build(std::vector<uint8_t> &send_data) const override;

	/* 送信アイテム種別取得 */
	SendItemType GetItemType(void) const noexcept override;

	/* カメラ映像モード通知要求 */
	void SendCameraVisualMode(const uint8_t mode);

private:
	uint8_t					opc_;
	std::vector<uint8_t>	data_;

	void SetMiscCmdCode(const uint8_t opc) noexcept;
	void SetMiscData(std::vector<uint8_t> &data);
};


} /* namespace videohal */


#endif	/* #ifndef	VHAL_CAPTURE_MICON_CONTROL_H */
