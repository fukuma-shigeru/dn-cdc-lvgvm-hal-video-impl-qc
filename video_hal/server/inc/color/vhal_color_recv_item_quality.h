/*******************************************************************************
    機能名称    ：  MISC画質モード応答受信アイテムモジュール
    ファイル名称：  vhal_color_recv_item_quality.h
*******************************************************************************/
#ifndef	VHAL_COLOR_RECV_ITEM_QUALITY_H
#define	VHAL_COLOR_RECV_ITEM_QUALITY_H

#include "vhal_color_mng.h"
#include "vhal_color_db.h"
#include "vhal_round_cast.h"
#include "vhal_micon_misc_control.h"

namespace videohal
{

/*****************************************************************************
 クラス名称：CVhalColorAdjustMiscReceiver
 処理概要  ：MISC画質モード応答受信アイテム
*****************************************************************************/
class CVhalColorAdjustMiscReceiver : public CVhalMiconReceiveItem {
public:

	CVhalColorAdjustMiscReceiver(void);
	~CVhalColorAdjustMiscReceiver(void) override ;
  	CVhalColorAdjustMiscReceiver(const CVhalColorAdjustMiscReceiver& src) = delete;
	CVhalColorAdjustMiscReceiver& operator=(const CVhalColorAdjustMiscReceiver& src) & = delete;
	CVhalColorAdjustMiscReceiver(CVhalColorAdjustMiscReceiver&& src) = delete;
	CVhalColorAdjustMiscReceiver& operator=(CVhalColorAdjustMiscReceiver&& src) & = delete;

	/* データ受信（VideoHALイベントスレッドからのコール） */
	void Receive(const std::vector<uint8_t>& data) override;

	/* データ受信事前通知（通信モジュールスレッドからのコール） */
	void ReceivePreNotify(const std::vector<uint8_t>& data) noexcept override;

	/* 受信アイテム種別取得 */
	ReceiveItemType GetItemType(void) const noexcept override;
	
	/* 画質モード応答 */
	void ReceiveColorAdjustStatus(const std::vector<uint8_t>& recv_data);

	/* 現在のバックライト設定（propertyによる設定値） */
	void SetCurrentBackLight(const bool light);
	/* 現在の前席ディスプレイMUTE設定（propertyによる設定値） */
	void SetCurrentMuteFrontDisp(const bool mute);

private:
	static constexpr uint32_t	kColorRcvMiscDataSizeMin{2U};	/* data_type+sub_type */
	static constexpr uint32_t	kColorRcvDataOpc{0U};
	uint8_t GetByteData(const uint8_t in_data, const uint8_t enable_bit, const uint8_t rightShift_bit) const;

	bool current_back_light_;			/* 現在のバックライト設定 */
	bool current_mute_front_disp_;		/* 現在の前席ディスプレイMUTE設定 */
};

} /* namespace videohal */

#endif	/* #ifndef	VHAL_COLOR_RECV_ITEM_QUALITY_H */
