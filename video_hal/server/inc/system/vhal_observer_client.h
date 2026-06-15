/*******************************************************************************
    機能名称    ：  状態監視制御クライアントモジュール
    ファイル名称：  vhal_observer_client.h
*******************************************************************************/
#ifndef	VHAL_OBSERVER_CLIENT_H
#define	VHAL_OBSERVER_CLIENT_H

namespace videohal
{

/*****************************************************************************
 クラス名称：CVhalObserverClient
 処理概要  ：状態通知イベントアイテムのベースクラス
*****************************************************************************/
class CVhalObserverClient {
public:
	CVhalObserverClient(void) noexcept = default;
	virtual ~CVhalObserverClient(void) = default;
  	CVhalObserverClient(const CVhalObserverClient& src) = delete;
	CVhalObserverClient(CVhalObserverClient&& src) = delete;

	/* 状態通知イベント処理の実施 */
	virtual int32_t Notify(void) = 0;

protected:
	CVhalObserverClient& operator=(const CVhalObserverClient& src) & = delete;
	CVhalObserverClient& operator=(CVhalObserverClient&& src) & = delete;

private:

};

} /* namespace videohal */

#endif	/* #ifndef	VHAL_OBSERVER_CLIENT_H */
