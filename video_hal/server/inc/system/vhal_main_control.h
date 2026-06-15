/*******************************************************************************
    機能名称    ：  メインコントロールモジュール
    ファイル名称：  vhal_main_control.h
*******************************************************************************/
#ifndef	VHAL_MAIN_CONTROL_H
#define	VHAL_MAIN_CONTROL_H

#include <vector>

#include "vhal_event_source.h"
#include "vhal_event_route.h"
#include "vhal_server.h"
#include "vhal_property_control.h"

namespace videohal
{

/*****************************************************************************
 クラス名称：CVhalMainControl
 処理概要  ：VideoHALの全体制御を行う。
*****************************************************************************/
class CVhalMainControl final {
public:
	CVhalMainControl(void);
	~CVhalMainControl(void);
  	CVhalMainControl(const CVhalMainControl& src) = delete;
	CVhalMainControl& operator=(const CVhalMainControl& src) & = default;
	CVhalMainControl(CVhalMainControl&& src) = delete;
	CVhalMainControl& operator=(CVhalMainControl&& src) & = delete;

	int32_t Initialize(void);
	void Finalize(void);

	/* イベントソースの登録 */
	int32_t RegisterEventSource(CVhalEventSource * const p_source);
	/* イベントソースの解除 */
	void ClearEventSource(CVhalEventSource * const p_source);
	/* イベントソースの解除(仮想関数未使用) */
	void ClearEventSource(CVhalEventSource * const p_source, const int32_t fd);

	/* メインループ処理開始 */
	int32_t RunEventLoop(void);
	/* メインループ強制終了 */
	void ExitEventLoop(void) const;

	/* 更新されたプロパティ名の通知 */
	void UpdatedProperties(std::vector<std::string> &property_names);
	/* プロパティ制御オブジェクトの取得 */
	friend CVhalPropertyControl *GetPropertyControl(CVhalMainControl const * const p_main) noexcept;
	/* 内部イベント送信処理 */
	int32_t WriteEvent(CVhalEventItemBase* const p_item) const noexcept;
	/* 内部イベント破棄処理 */
	void DropEvents(void);

protected:

private:
	std::vector<CVhalEventSource *> event_sources_;

	std::unique_ptr<CVhalEventRoute> p_route_;
	std::unique_ptr<CVhalPropertyControl> p_prop_;
	std::unique_ptr<CVhalServer> p_server_;

	bool running_;
	int32_t epoll_fd_;
};

} /* namespace videohal */

#endif	/* #ifndef	VHAL_MAIN_CONTROL_H */
