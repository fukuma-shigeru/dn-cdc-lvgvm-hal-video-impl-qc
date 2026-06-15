/*******************************************************************************
    機能名称    ：  通信サーバーモジュール
    ファイル名称：  vhal_server.h
*******************************************************************************/
#ifndef	VHAL_SERVER_H
#define	VHAL_SERVER_H

#include <vector>
#include <string>

#include "halcomm_protected.h"

#include "vhal_property_client.h"
#include "vhal_event_source.h"

namespace videohal
{


/*****************************************************************************
 クラス名称：CVhalServer
 処理概要  ：HalCommを使った通信のサーバー側処理を行う。
*****************************************************************************/
class CVhalServer : public CVhalEventSource {
public:
	CVhalServer(void);
	~CVhalServer(void) override;
  	CVhalServer(const CVhalServer& src) = delete;
	CVhalServer& operator=(const CVhalServer& src) & = default;
	CVhalServer(CVhalServer&& src) = delete;
	CVhalServer& operator=(CVhalServer&& src) & = delete;

	int32_t Initialize(void) noexcept override;
	int32_t Initialize(CVhalMainControl * const p_main);
	void Finalize(void);

	/* ディレクトリパス作成 */
	static int32_t MkdirPath(const char * const p_dir, const mode_t mode);
	/* イベント受信用fdの取得 */
	int32_t GetSourceFd(void) const override;
	/* イベントの読み込みと実行 */
	int32_t ExecEvent(const uint32_t source_event) override;

	/* 更新プロパティリストを全クライアントに通知する */
	int32_t Publish(std::vector<std::string> &property_names);

	/* クライアント切断通知 */
	void ClientDisconnected(CVhalEventSource * const p_source);

protected:

private:
	CVhalMainControl *p_main_;
	halcomm::HalCommObj_t halcomm_obj_;
	std::vector<std::unique_ptr<CVhalPropertyClient>> clients_;

	/* 接続してきたクライアントのaccept処理 */
	std::unique_ptr<CVhalPropertyClient> AcceptClient(void);
};

} /* namespace videohal */

#endif	/* #ifndef	VHAL_SERVER_H */
