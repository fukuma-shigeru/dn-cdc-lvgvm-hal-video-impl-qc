/*******************************************************************************
    機能名称    ：  Suspend To Ram状態管理モジュール
    ファイル名称：  vhal_str_mng.h
*******************************************************************************/
#ifndef	VHAL_STR_MNG_H
#define	VHAL_STR_MNG_H

#include <cstdint>
#include <array>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <memory>
#include "vhal_define.h"

namespace videohal
{

/* 待ちイベント種別 */
enum class StrWaitState : uint32_t
{
	SuspendApps = 0U,	/* SysDB検出待ち */
	SuspendVhal,		/* VideoHAL終了処理待ち */
	ResumeSystem,		/* システム復帰待ち */
	ResumeVhal,			/* VideoHAL開始処理待ち */
	StrWaitStateMax,	/* 最大値 */
};

/*****************************************************************************
 クラス名称：CVhalStrManager
 処理概要  ：STR(Suspend To Ram)状態を管理する
 ****************************************************************************/
class CVhalStrManager final {

public:

	/* 実体取得(シングルトン) */
	static CVhalStrManager& GetInstance(void);

	/* STRモード設定 */
	static void SetStrMode(const bool param);

	/* サスペンド状態設定 */
	static void SetSuspend(const bool param);

	/* VM STR移行ステータス設定 */
	void SetVmstrValue(const int32_t value) noexcept;

	/* STRモード取得 */
	static bool GetStrMode(void);

	/* サスペンド状態取得 */
	static bool GetSuspend(void);

	/* イベント待ち */
	int32_t Wait(const StrWaitState wait_state);

	/* イベント待ち解除 */
	void NotifyOne(const StrWaitState wait_state);

private:
	CVhalStrManager(void) = default;
	~CVhalStrManager(void) noexcept = default;
  	CVhalStrManager(const CVhalStrManager& src) = delete;
	CVhalStrManager& operator=(const CVhalStrManager& src) & = delete;
	CVhalStrManager(CVhalStrManager&& src) = delete;
	CVhalStrManager& operator=(CVhalStrManager&& src) & = delete;

	/* StrWaitStateからの数値キャスト */
	static constexpr uint32_t EnumToU32(const StrWaitState wait_state) noexcept;

	static constexpr uint32_t								kStrWaitStateMax{static_cast<uint32_t>(StrWaitState::StrWaitStateMax)};
	static constexpr uint32_t								kStrTimeout{18000U};			/* 18秒（ミリ秒単位）*/

	std::atomic<bool>										str_mode_{false};				/* PFMサスペンド開始～VHALレジューム完了までtrue */
	std::atomic<bool>										suspend_{false};				/* VHALサスペンド開始～PFMレジューム開始までtrue */
	std::atomic<int32_t>									vmstr_value_{VHAL_SUCCESS};		/* VM STR移行ステータス */
	std::array<bool, kStrWaitStateMax>						flag_{};						/* 待ち状態管理フラグ */
	std::array<std::mutex, kStrWaitStateMax>				sync_mtx_;						/* 待ち状態管理ミューテックス */
	std::array<std::condition_variable, kStrWaitStateMax>	sync_cond_;						/* 待ち状態管理条件変数 */
};

} /* namespace videohal */

#endif	/* #ifndef	VHAL_STR_MNG_H */
