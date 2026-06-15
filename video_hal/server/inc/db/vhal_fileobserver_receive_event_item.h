/*******************************************************************************
    機能名称    ：  FileObserver イベントモジュール
    ファイル名称：  vhal_fileobserver_receive_event_item.h
*******************************************************************************/
#ifndef	VHAL_FILEOBSERVER_RECEIVE_EVENT_ITEM_H
#define	VHAL_FILEOBSERVER_RECEIVE_EVENT_ITEM_H

#include "vhal_event_item_base.h"
#include "vhal_fileobserver_control.h"

namespace videohal
{

/*****************************************************************************
 クラス名称：CVhalFileObserverReceiveEventItem
 処理概要  ：FileObserver イベントクラス
*****************************************************************************/
class CVhalFileObserverReceiveEventItem : public CVhalEventItemBase {
public:

	CVhalFileObserverReceiveEventItem(void);
	~CVhalFileObserverReceiveEventItem(void) override;
	CVhalFileObserverReceiveEventItem(const CVhalFileObserverReceiveEventItem& src) = delete;
	CVhalFileObserverReceiveEventItem& operator=(const CVhalFileObserverReceiveEventItem& src) & = delete;
	CVhalFileObserverReceiveEventItem(CVhalFileObserverReceiveEventItem&& src) = delete;
	CVhalFileObserverReceiveEventItem& operator=(CVhalFileObserverReceiveEventItem&& src) & = delete;


	/* イベント処理の実施 */
	int32_t Exec(void) const override;

	/* データ設定 */
	void SetData(CVhalFileObserverControl* const p_fileObserverctrl, const std::string& property_monitor_path, const uint32_t value) noexcept;

private:
	CVhalFileObserverControl*	p_fileob_;
	std::string					monitor_path_;
	uint32_t					value_;
};

} /* namespace videohal */

#endif	/* #ifndef	VHAL_FILEOBSERVER_RECEIVE_EVENT_ITEM_H */
