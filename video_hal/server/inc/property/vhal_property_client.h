/*******************************************************************************
    機能名称    ：  VideoHAL接続クライアントモジュール
    ファイル名称：  vhal_property_client.h
*******************************************************************************/
#ifndef	VHAL_PROPERTY_CLIENT_H
#define	VHAL_PROPERTY_CLIENT_H

#include <set>
#include <string>
#include <vector>

#include "halcomm_protected.h"

#include "vhal_event_source.h"

namespace videohal
{

class CVhalPropertyControl;
class CVhalMainControl;

/*****************************************************************************
 クラス名称：CVhalPropertyClient
 処理概要  ：VideoHALに接続しているクライアントからのリクエストを処理する
*****************************************************************************/
class CVhalPropertyClient : public CVhalEventSource {
public:
	CVhalPropertyClient(void);
	~CVhalPropertyClient(void) override;
  	CVhalPropertyClient(const CVhalPropertyClient& src) = delete;
	CVhalPropertyClient& operator=(const CVhalPropertyClient& src) & = default;
	CVhalPropertyClient(CVhalPropertyClient&& src) = delete;
	CVhalPropertyClient& operator=(CVhalPropertyClient&& src) & = delete;

	int32_t Initialize(void) noexcept override;
	int32_t Initialize(CVhalMainControl * const p_main);
	void Finalize(void);

	/* イベント受信用fdの取得 */
	int32_t GetSourceFd(void) const override;
	/* イベントの読み込みと実行 */
	int32_t ExecEvent(const uint32_t source_event) override;

	/* HalCommonクライアントオブジェクトの取得 */
	friend halcomm::HalCommObj_t *GetHalCommClient(CVhalPropertyClient * const p_client) noexcept;

	/* SetValue処理の実行 */
	int32_t SetValue(const halcomm::HalCommSetReq_t* const p_req, halcomm::HalCommSetAck_t* const p_ack);
	/* GetValue処理の実行 */
	int32_t GetValue(const halcomm::HalCommGetReq_t* const p_req, halcomm::HalCommGetAck_t* const p_ack);
	/* Subscribe処理の実行 */
	int32_t Subscribe(halcomm::HalCommSubReq_t* const p_req, halcomm::HalCommSubAck_t* const p_ack);
	/* UnSubscribe処理の実行 */
	int32_t UnSubscribe(halcomm::HalCommUnSubReq_t* const p_req, halcomm::HalCommUnSubAck_t* const p_ack);

	/* プロパティ更新通知を行う */
	int32_t Publish(std::vector<std::string> &property_names);

protected:

private:
	CVhalMainControl *p_main_;
	CVhalPropertyControl *p_prop_;
	halcomm::HalCommObj_t halcomm_obj_;
	std::set<std::string> subscriptions_;

	halcomm::HalCommMsg_t reqMsg_;	// サイズが大きすぎるけど・・・ひとつ3.5MBくらい
	halcomm::HalCommMsg_t ackMsg_;
};

} /* namespace videohal */

#endif	/* #ifndef	VHAL_PROPERTY_CLIENT_H */
