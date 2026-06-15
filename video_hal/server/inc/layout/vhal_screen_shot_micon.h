/*******************************************************************************
    機能名称    ：  画面キャプチャ制御モジュール
    ファイル名称：  vhal_screen_shot_micon.h
*******************************************************************************/
#ifndef	VHAL_SCREEN_SHOT_MICON_H
#define	VHAL_SCREEN_SHOT_MICON_H

#include <string>
#include <memory>
#include <mutex>

#include "vhal_event_route.h"
#include "vhal_micon_receive_item.h"
#include "vhal_micon_send_item.h"
#include "vhal_micon_comm_control.h"
#include "vhal_event_item_screen_shot_event_micon.h"

namespace videohal
{
class CVhalMainControl;
class CVhalLayoutManager;

/*****************************************************************************
 クラス名称：CVhalScreenShotMicon
 処理概要  ：スクリーンショット制御処理
*****************************************************************************/
class CVhalScreenShotMicon {
public:
	CVhalScreenShotMicon(void) noexcept;
	~CVhalScreenShotMicon(void);
  	CVhalScreenShotMicon(const CVhalScreenShotMicon& src) = delete;
	CVhalScreenShotMicon& operator=(const CVhalScreenShotMicon& src) & = delete;
	CVhalScreenShotMicon(CVhalScreenShotMicon&& src) = delete;
	CVhalScreenShotMicon& operator=(CVhalScreenShotMicon&& src) & = delete;

	/* 初期化処理（内部リソースの確保） */
	int32_t Initialize(CVhalMainControl * const p_main_control, CVhalLayoutManager * const p_layout_mng, CVhalMiconCommControl * const p_micon_comm_control);

	/* 終了処理（内部リソースの解放） */
	void Finalize(void);

	/* 画面キャプチャ要求送信 */
	int32_t SendScreenShotRequest(const int32_t ivi_id, const std::string& filepath) noexcept;

	/* スクリーンショット応答通知(from QNX) */
	void NotifyScreenShotMiconResponse(const VhalScreenShotResult response) noexcept;

	/* イベントリスナー登録 */
	int32_t RegisterEventListener(CVhalScreenShotReceiveEventListenerBase* const p_listener);

	/* イベントリスナー解除 */
	void ClearEventListener(void) noexcept;

	/* スクリーンショット格納先ファイルパス取得 */
	void GetDestFilePath(std::string& filepath) const noexcept;

	/* 画面キャプチャ要求送信キャンセル */
	void CancelScreenShotRequest(void) noexcept;

private:
	CVhalMainControl*							p_main_;
	CVhalLayoutManager*							p_layout_;
	CVhalMiconCommControl*						p_micon_comm_;
	std::unique_ptr<CVhalEventRoute>			p_route_;

	std::unique_ptr<CVhalScreenShotTimer>		p_screenshot_send_timer_;
	bool										initialize_failed_;
	mutable std::mutex							mtx_starting_;
	std::string									dest_filepath_;

	CVhalScreenShotReceiveEventListenerBase		*p_screenshot_listener_;

	/* スクリーンショット応答タイムアウト周期(ms) */
	static constexpr uint32_t response_interval{20000U};

	/* スクリーンショット応答待ちサイクル数 */
	static constexpr uint32_t response_cycle{1U};

	/* スクリーンショットファイルコピーリトライ回数 */
	static constexpr uint32_t copy_retry_count{5U};

	/* スクリーンショットファイルコピーリトライwait(ms) */
	static constexpr uint32_t copy_retry_wait{100U};

	/* 格納先ファイルパスチェック */
	int32_t CheckDestPath(const std::string& filepath) const noexcept;

	/* タイマースタート */
	int32_t StartTimer(void);

	/* タイマーストップ */
	void StopTimer(void);

	/* スクリーンショット格納先ファイルパス設定 */
	void SetDestFilePath(const std::string& filepath) noexcept;

	/* ivi_idからスクリーン種別変換 */
	static int32_t ConvertScreenType(const int32_t ivi_id, uint8_t& type) noexcept;

	/* スクリーンショットファイルコピー */
	static int32_t CopyScreenShot(const std::string &src, const std::string &dest) noexcept;

};

} /* namespace videohal */

#endif	/* #ifndef	VHAL_SCREEN_SHOT_MICON_H */
