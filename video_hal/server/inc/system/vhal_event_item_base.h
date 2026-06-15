/*******************************************************************************
    機能名称    ：  内部イベントアイテムベースモジュール
    ファイル名称：  vhal_event_item_base.h
*******************************************************************************/
#ifndef	VHAL_EVENT_ITEM_BASE_H
#define	VHAL_EVENT_ITEM_BASE_H

#include <string>

namespace videohal
{

/*****************************************************************************
 クラス名称：CVhalEventItemBase
 処理概要  ：内部イベントアイテムのベースクラス
*****************************************************************************/
class CVhalEventItemBase {
public:
	CVhalEventItemBase(void) noexcept;
	virtual ~CVhalEventItemBase(void) = default;
  	CVhalEventItemBase(const CVhalEventItemBase& src) = delete;
	CVhalEventItemBase(CVhalEventItemBase&& src) = delete;

	/* イベント名の設定 */
	void SetName(const std::string &event_name);

	/* イベント処理の実施 */
	virtual int32_t Exec(void) const = 0;


protected:
	CVhalEventItemBase& operator=(const CVhalEventItemBase& src) & = delete;
	CVhalEventItemBase& operator=(CVhalEventItemBase&& src) & = delete;

private:
	std::string event_name_;	/* イベント名（ログ用） */

};

} /* namespace videohal */

#endif	/* #ifndef	VHAL_EVENT_ITEM_BASE_H */
