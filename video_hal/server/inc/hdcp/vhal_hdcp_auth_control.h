/*******************************************************************************
    機能名称    ：  HDCP認証制御モジュール
    ファイル名称：  vhal_hdcp_auth_control.h
*******************************************************************************/
#ifndef	VHAL_HDCP_AUTH_CONTROL_H
#define	VHAL_HDCP_AUTH_CONTROL_H

#include "vhal_define.h"
#include "vhal_event_item_hdcp_auth.h"
#include "vhal_micon_misc_opc.h"

#include <string>
#include <vector>
#include <mutex>
#include <memory>

namespace videohal
{
class CVhalMainControl;
class CVhalBackupControl;

/* HDCP認証対象種別 */
enum class hdcpAuthType : int32_t {
	HDCP_AUTH_TYPE_RSE = 0,
	HDCP_AUTH_TYPE_CDISP,
	HDCP_AUTH_TYPE_MAX
};

/*****************************************************************************
 クラス名称：CVhalHdcpAuthRsltData
	処理概要  ： HDCP認証結果情報 (HDCP認証キー)
*****************************************************************************/
class CVhalHdcpAuthRsltData {
public:
	CVhalHdcpAuthRsltData(void) noexcept { Clear(); }
	virtual ~CVhalHdcpAuthRsltData(void) = default;
  	CVhalHdcpAuthRsltData(const CVhalHdcpAuthRsltData& src) = delete;
	CVhalHdcpAuthRsltData& operator=(const CVhalHdcpAuthRsltData& src) & = default;
	CVhalHdcpAuthRsltData(CVhalHdcpAuthRsltData&& src) = delete;
	CVhalHdcpAuthRsltData& operator=(CVhalHdcpAuthRsltData&& src) & = delete;

	static constexpr uint32_t	khdcpReceiverIdSize{5U};	/* HDCP Receiver IDのByte数 */
	static constexpr uint64_t	khdcpByteToBit{8U};

	static constexpr uint8_t	kMaxDevsExceed{1U};
	static constexpr uint8_t	kMaxDeviceCount{32U};
	static constexpr uint8_t	kMaxCascadeExceed{1U};
	static constexpr uint8_t	kMaxDepth{4U};

	/* ==設定系== */

	/* 実施結果を設定 */
	void SetResult(const int32_t result) noexcept
	{
		result_ = result;
	}
	/* 種別を設定 */
	void SetSubType(const uint8_t sub_type) noexcept
	{
		sub_type_ = sub_type;
	}
	/* MAX_DEVS_EXCEEDを設定 */
	void SetMaxDevs(const uint8_t max_devs_exceeded) noexcept
	{
		max_devs_exceeded_ = max_devs_exceeded;
	}
	/* DEVICE_COUNTを設定 */
	void SetDevsCount(const uint8_t device_count) noexcept
	{
		device_count_ = device_count;
	}
	/* MAX_CASCADE_EXCEEDEDを設定 */
	void SetMaxCascade(const uint8_t max_cascade_exceeded) noexcept
	{
		max_cascade_exceeded_ = max_cascade_exceeded;
	}
	/* CASCADE_DEPTHを設定 */
	void SetCascadeDepth(const uint8_t depth) noexcept
	{
		depth_ = depth;
	}
	/* HDCP認証鍵を設定(一括) */
	void SetReceiverIds(const std::vector<uint64_t> receiver_ids) noexcept
	{
		receiver_ids_ = receiver_ids;
	}
	/* HDCP認証鍵を追加(バイト配列) */
	void AddReceiverIds(const std::vector<uint8_t>& receiver_id) noexcept;

	/* ==取得系== */

	/* 実施結果を取得 */
	int32_t GetResult(void) const noexcept
	{
		return result_;
	}
	/* 種別を取得 */
	uint8_t GetSubType(void) const noexcept
	{
		return sub_type_;
	}
	/* MAX_DEVS_EXCEEDを取得 */
	uint8_t GetMaxDevs(void) const noexcept
	{
		return max_devs_exceeded_;
	}
	/* DEVICE_COUNTを取得 */
	uint8_t GetDevsCount(void) const noexcept
	{
		return device_count_;
	}
	/* MAX_CASCADE_EXCEEDEDを取得 */
	uint8_t GetMaxCascade(void) const noexcept
	{
		return max_cascade_exceeded_;
	}
	/* CASCADE_DEPTHを取得 */
	uint8_t GetCascadeDepth(void) const noexcept
	{
		return depth_;
	}
	/* HDCP認証件数を取得 */
	uint32_t SizeReceiverIds(void) const noexcept
	{
		return static_cast<uint32_t>(receiver_ids_.size());
	}
	/* HDCP認証鍵を取得 */
	std::vector<uint64_t> GetReceiverIds(void) const noexcept
	{
		return receiver_ids_;
	}
	/* HDCP認証鍵を変換 */
	void ConvReceiverIds(const uint64_t i_receiver_id, std::vector<uint8_t>& o_receiver_id) const noexcept;

	/* シリアライズ */
	void Serialize(std::vector<uint8_t>& serial_data) const noexcept;

	/* デシリアライズ */
	void Deserialize(const std::vector<uint8_t>& serial_data) noexcept;

	/* HDCP認証キー情報クリア */
	void Clear(void) noexcept;

	/* HDCP認証キー比較 */
	bool Equal(const CVhalHdcpAuthRsltData& hdcp_key) const noexcept;

private:
	int32_t					result_;								/* HDCP認証結果 */
	uint8_t					sub_type_;								/* 種別(05h:C-Disp, 07h:RSE, 00h:クリア) */
	uint8_t					max_devs_exceeded_;						/* HDCPデバイスオーバー 0~1 */
	uint8_t					device_count_;							/* HDCPデバイス数 0~32 */
	uint8_t					max_cascade_exceeded_;					/* HDCP接続段数オーバー 0~1 */
	uint8_t					depth_;									/* HDCP接続段数 0~4 */
	std::vector<uint64_t>	receiver_ids_;							/* HDCPレシーバーID 0~32 */
};

/*****************************************************************************
 クラス名称：CVhalHdcpAuthSerialize
	処理概要  ： 保存用HDCP認証キー
*****************************************************************************/
struct CVhalHdcpAuthSerialize final {
public:
	/* 設定系 */
	void SetSubType(const uint8_t sub_type) noexcept
	{
		sub_type_ = sub_type;
		sub_type_end_ = sub_type;
	}
	void SetMaxDevs(const uint8_t max_devs_exceeded) noexcept
	{
		max_devs_exceeded_ = max_devs_exceeded;
	}
	void SetDevsCount(const uint8_t device_count) noexcept
	{
		device_count_ = device_count;
	}
	void SetMaxCascade(const uint8_t max_cascade_exceeded) noexcept
	{
		max_cascade_exceeded_ = max_cascade_exceeded;
	}
	void SetCascadeDepth(const uint8_t depth) noexcept
	{
		depth_ = depth;
	}
	void SetReceiverId(const uint8_t pos, const std::vector<uint8_t>& receiver_id) noexcept
	{
		constexpr uint32_t kReceiverIdSize{CVhalHdcpAuthRsltData::khdcpReceiverIdSize};
		if ((CVhalHdcpAuthRsltData::kMaxDeviceCount > pos) && (kReceiverIdSize <= receiver_id.size()))
		{
			for (uint32_t i{0U}; i < kReceiverIdSize; ++i)
			{
				receiver_ids_[pos][i] = receiver_id[i];
			}
		}
	}
	/* 取得系 */
	uint8_t GetSubType(void) const noexcept
	{
		return sub_type_;
	}
	uint8_t GetSubTypeEnd(void) const noexcept
	{
		return sub_type_end_;
	}
	uint8_t GetMaxDevs(void) const noexcept
	{
		return max_devs_exceeded_;
	}
	uint8_t GetDevsCount(void) const noexcept
	{
		return device_count_;
	}
	uint8_t GetMaxCascade(void) const noexcept
	{
		return max_cascade_exceeded_;
	}
	uint8_t GetCascadeDepth(void) const noexcept
	{
		return depth_;
	}
	void GetReceiverId(const uint8_t pos, std::vector<uint8_t>& receiver_id) const noexcept
	{
		constexpr uint32_t kReceiverIdSize{CVhalHdcpAuthRsltData::khdcpReceiverIdSize};
		receiver_id.resize(kReceiverIdSize);
		for (uint32_t i{0U}; i < kReceiverIdSize; ++i)
		{
			receiver_id[i] = receiver_ids_[pos][i];
		}
	}

private:
	uint8_t		sub_type_;					/* 開始識別子 */
	uint8_t		max_devs_exceeded_;			/* MAX_DEVS_EXCEEDED */
	uint8_t		device_count_;				/* DEVICE_COUNT */
	uint8_t		max_cascade_exceeded_;		/* MAX_CASCADE_EXCEEDED */
	uint8_t		depth_;						/* DEPTH */
	uint8_t		receiver_ids_[32][5];		/* Receiver_ID [0～31] */
	uint8_t		reserved_[90];				/* 予備 */
	uint8_t		sub_type_end_;				/* 終了識別子 */
};

/*****************************************************************************
 クラス名称：CVhalHdcpAuthRsltListenerBase
 処理概要  ：HDCP認証イベントリスナベース。
*****************************************************************************/
class CVhalHdcpAuthRsltListenerBase {
public:
	CVhalHdcpAuthRsltListenerBase(void) noexcept = default;
	virtual ~CVhalHdcpAuthRsltListenerBase(void) = default;
  	CVhalHdcpAuthRsltListenerBase(const CVhalHdcpAuthRsltListenerBase& src) = delete;
	CVhalHdcpAuthRsltListenerBase(CVhalHdcpAuthRsltListenerBase&& src) = delete;

	virtual void NotifyHdcpAuthResult(const hdcpAuthType type, const CVhalHdcpAuthRsltData &hdcp_result) const noexcept = 0;
	virtual void NotifyHdcpAuthClear(void) const noexcept = 0;

private:
	CVhalHdcpAuthRsltListenerBase& operator=(const CVhalHdcpAuthRsltListenerBase& src) & = delete;
	CVhalHdcpAuthRsltListenerBase& operator=(CVhalHdcpAuthRsltListenerBase&& src) & = delete;
};

/*****************************************************************************
 クラス名称：CVhalHdcpAuthControl
 処理概要  ：状態監視の制御を行う。
*****************************************************************************/
class CVhalHdcpAuthControl {
public:

	CVhalHdcpAuthControl(void);
	~CVhalHdcpAuthControl(void);
  	CVhalHdcpAuthControl(const CVhalHdcpAuthControl& src) = delete;
	CVhalHdcpAuthControl& operator=(const CVhalHdcpAuthControl& src) & = default;
	CVhalHdcpAuthControl(CVhalHdcpAuthControl&& src) = delete;
	CVhalHdcpAuthControl& operator=(CVhalHdcpAuthControl&& src) & = delete;

	int32_t Initialize(const hdcpAuthType auth_type);
	void Finalize(void);

	/* HDCP認証開始 */
	int32_t StartAuth(const int32_t type);
	/* HDCP認証停止 */
	int32_t StopAuth(void);
	/* 接続機器種別設定 */
	void SetConnectedDevice(const int32_t type) noexcept;
	/* HDCP認証キー取得 */
	void GetHdcpAuthKey(CVhalHdcpAuthRsltData& hdcp_key) const noexcept;
	/* HDCP認証キー更新 */
	int32_t UpdateHdcpAuthResult(const CVhalHdcpAuthRsltData& hdcp_key) noexcept;
	/* HDCP認証キークリア */
	void ClearHdcpAuthKey(void) noexcept;

private:
	/* 接続機器状態判定 */
	bool CheckConnectedDevice(void) const;
	/* HDCP認証キー正当性チェック */
	int32_t CheckHdcpAuthKey(const CVhalHdcpAuthRsltData& hdcp_key) const noexcept;
	/* HDCP認証キー読込 */
	void LoadHdcpAuthKey(CVhalHdcpAuthRsltData& hdcp_key) const noexcept;
	/* HDCP認証キー保存 */
	void SaveHdcpAuthKey(const CVhalHdcpAuthRsltData& hdcp_key) const noexcept;

	std::unique_ptr<CVhalBackupControl>	p_backup_control_{nullptr};
	CVhalHdcpAuthRsltData			hdcp_result_;				/* HDCP認証キー */

	hdcpAuthType					auth_type_{hdcpAuthType::HDCP_AUTH_TYPE_RSE};				/* HDCP認証対象種別 */
	bool							first_auth_{false};											/* HDCP認証開始済状態(TRUE:システム起動後一度でも認証開始済) */
	bool							auth_request_{false};										/* HDCP認証開始要求(TRUE：開始要求) */

	int32_t							dev_type_{0};												/* 接続機器種別 */

};

} /* namespace videohal */


#endif	/* #ifndef	VHAL_HDCP_AUTH_CONTROL_H */
