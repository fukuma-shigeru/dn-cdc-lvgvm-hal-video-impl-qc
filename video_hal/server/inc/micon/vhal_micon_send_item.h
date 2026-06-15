/*******************************************************************************
    機能名称    ：  マイコン間通信送信アイテムモジュール
    ファイル名称：  vhal_micon_send_item.h
*******************************************************************************/
#ifndef	VHAL_MICON_SEND_ITEM_H
#define	VHAL_MICON_SEND_ITEM_H


#include <vector>

#include "vhal_define.h"

namespace videohal
{

/*****************************************************************************
 クラス名称：CVhalMiconSendItem
 処理概要  ：マイコン間通信送信アイテム
*****************************************************************************/
class CVhalMiconSendItem {
public:

	enum class SendItemType : int32_t {
		SEND_ITEM_TYPE_MISC,
		SEND_ITEM_TYPE_MOST
	};


	CVhalMiconSendItem(void) = default;
	virtual ~CVhalMiconSendItem(void) = default;
  	CVhalMiconSendItem(const CVhalMiconSendItem& src) = delete;
	CVhalMiconSendItem(CVhalMiconSendItem&& src) = delete;

	/* 送信データ構築 */
	virtual int32_t Build(std::vector<uint8_t> &send_data) const=0;

	virtual SendItemType GetItemType(void) const=0;

private:

	CVhalMiconSendItem& operator=(const CVhalMiconSendItem& src) & = delete;
	CVhalMiconSendItem& operator=(CVhalMiconSendItem&& src) & = delete;
};

} /* namespace videohal */

#endif	/* #ifndef	VHAL_MICON_SEND_ITEM_H */
