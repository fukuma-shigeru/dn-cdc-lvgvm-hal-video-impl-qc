/*******************************************************************************
    機能名称    ：  内部イベント送信ルートモジュール
    ファイル名称：  vhal_event_route.h
*******************************************************************************/
#ifndef	VHAL_EVENT_ROUTE_H
#define	VHAL_EVENT_ROUTE_H

#include "vhal_event_item_base.h"
#include "vhal_event_source.h"

namespace videohal
{

/*****************************************************************************
 クラス名称：CVhalEventRoute
 処理概要  ：内部イベントの送信ルートを構成する
*****************************************************************************/
class CVhalEventRoute : public CVhalEventSource {
public:

	CVhalEventRoute(void);
	~CVhalEventRoute(void) override;
  	CVhalEventRoute(const CVhalEventRoute& src) = delete;
	CVhalEventRoute& operator=(const CVhalEventRoute& src) & = default;
	CVhalEventRoute(CVhalEventRoute&& src) = delete;
	CVhalEventRoute& operator=(CVhalEventRoute&& src) & = delete;

	int32_t Initialize(void) override;
	void Finalize(void);

	/* イベント受信用fdの取得 */
	int32_t GetSourceFd(void) const noexcept override;
	/* イベントの読み込みと実行 */
	int32_t ExecEvent(const uint32_t source_event) override;

	/* イベントアイテムの書き込み */
	int32_t WriteEvent(CVhalEventItemBase *p_item) const;
	/* イベントアイテムの読み出し */
	CVhalEventItemBase *ReadEvent(void) const;

protected:

private:
	static constexpr int32_t kSocketPairRead{0};
	static constexpr int32_t kSocketPairWrite{1};
	static int32_t route_counter_;

	int32_t route_id_{};
	int32_t pair_fd_[2];	/* socketpairで生成したfdふたつ */
};

} /* namespace videohal */

#endif	/* #ifndef	VHAL_EVENT_ROUTE_H */
