/*******************************************************************************
    機能名称    ：  マイコン間通信受信アイテムモジュール
    ファイル名称：  vhal_micon_receive_item.h
*******************************************************************************/
#ifndef	VHAL_MICON_RECEIVE_ITEM_H
#define	VHAL_MICON_RECEIVE_ITEM_H

#include <vector>

#include "vhal_define.h"

namespace videohal
{

/*****************************************************************************
 クラス名称：CVhalMiconReceiveItem
 処理概要  ：マイコン間通信受信アイテム
*****************************************************************************/
class CVhalMiconReceiveItem {
public:

	enum class ReceiveItemType : int32_t {
		RECEIVE_ITEM_TYPE_MISC,
		RECEIVE_ITEM_TYPE_MOST
	};


	CVhalMiconReceiveItem(void) = default;
	virtual ~CVhalMiconReceiveItem(void) = default;
  	CVhalMiconReceiveItem(const CVhalMiconReceiveItem& src) = delete;
	CVhalMiconReceiveItem(CVhalMiconReceiveItem&& src) = delete;

	/* データ受信（VideoHALイベントスレッドからのコール） */
	virtual void Receive(const std::vector<uint8_t>& data) = 0;

	/* データ受信事前通知（通信モジュールスレッドからのコール） */
	virtual void ReceivePreNotify(const std::vector<uint8_t>& data) = 0;

	virtual ReceiveItemType GetItemType(void) const = 0;

private:

	CVhalMiconReceiveItem& operator=(const CVhalMiconReceiveItem& src) & = delete;
	CVhalMiconReceiveItem& operator=(CVhalMiconReceiveItem&& src) & = delete;
};

} /* namespace videohal */

#endif	/* #ifndef	VHAL_MICON_RECEIVE_ITEM_H */
