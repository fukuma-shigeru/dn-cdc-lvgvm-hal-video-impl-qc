/*******************************************************************************
    機能名称    ：  Suspend To Ram状態管理モジュール
    ファイル名称：  vhal_str_mng.cpp
*******************************************************************************/
#include "vhal_str_mng.h"
#include "vhal_log.h"

namespace videohal
{

/*****************************************************************************
 処理概要：	実体取得(シングルトン)
 引数    ：	なし
 戻り値  ：	CVhalStrManager参照
 備考    ：	本関数はvhal_main()の開始時にインスタンス生成を行う
 ****************************************************************************/
CVhalStrManager& CVhalStrManager::GetInstance(void)
{
	static CVhalStrManager	instance{};
	return instance;
}

/*****************************************************************************
 処理概要：	STRモード設定
 引数    ：	bool 		param	(i)	true:STR開始, false:STR終了
 戻り値  ：	なし
*****************************************************************************/
void CVhalStrManager::SetStrMode(const bool param)
{
	CVhalStrManager&	instance{CVhalStrManager::GetInstance()};
	/* STR状態解除時 */
	if (!param)
	{
		for (uint32_t idx{EnumToU32(StrWaitState::SuspendApps)}; idx < kStrWaitStateMax; ++idx)
		{
			const std::lock_guard<std::mutex>	lock{instance.sync_mtx_[idx]};
			instance.flag_[idx] = false;
		}
	}
	instance.str_mode_.store(param, std::memory_order_release);
}

/*****************************************************************************
 処理概要：	サスペンド状態設定
 引数    ：	bool 		param	(i)	true:サスペンド状態, false:非サスペンド状態
 戻り値  ：	なし
*****************************************************************************/
void CVhalStrManager::SetSuspend(const bool param)
{
	CVhalStrManager&	instance{CVhalStrManager::GetInstance()};
	instance.suspend_.store(param, std::memory_order_release);
}

/*****************************************************************************
 処理概要：	VM STR移行ステータス設定
 引数    ：	int32_t		value	(i)	VM STR移行ステータス値
 戻り値  ：	なし
*****************************************************************************/
void CVhalStrManager::SetVmstrValue(const int32_t value) noexcept
{
	vmstr_value_.store(value, std::memory_order_release);
}

/*****************************************************************************
 処理概要：	STRモード取得
 引数    ：	なし
 戻り値  ：	bool 				true:STR開始, false:STR終了
*****************************************************************************/
bool CVhalStrManager::GetStrMode(void)
{
	CVhalStrManager&	instance{CVhalStrManager::GetInstance()};
	return instance.str_mode_.load(std::memory_order_acquire);
}

/*****************************************************************************
 処理概要：	サスペンド状態取得
 引数    ：	なし
 戻り値  ：	bool 				true:サスペンド状態, false:非サスペンド状態
*****************************************************************************/
bool CVhalStrManager::GetSuspend(void)
{
	CVhalStrManager&	instance{CVhalStrManager::GetInstance()};
	return instance.suspend_.load(std::memory_order_acquire);
}

/*****************************************************************************
 処理概要：	イベント待ち
 引数    ：	StrWaitState	wait_state	待ちイベント種別
 戻り値  ：	処理結果
           		VHAL_ERR				処理失敗
           		VHAL_ERR_TIMEOUT		タイムアウト発生
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalStrManager::Wait(const StrWaitState wait_state)
{
	int32_t				result{VHAL_SUCCESS};
	const uint32_t		idx{EnumToU32(wait_state)};

	/* 適切な引数 */
	if (kStrWaitStateMax > idx)
	{
		/* VideoHAL開始処理待ち以外の場合は、タイムアウト無しとする */
		/* 通知が無い場合、監視プロセス側でシステムダウンする仕様 */
		if (StrWaitState::ResumeVhal != wait_state)
		{
			/* 本waitはVideoHALメインスレッド(RunEventLoop内)で実行される。 */
			/* そのため、本wait中にvhal_main()が終了することは構造上あり得ない。 */
			std::unique_lock<std::mutex> lock_sync{sync_mtx_[idx]};
			/* coverity[cert_ctr50_cpp_violation] */
			sync_cond_[idx].wait(lock_sync, [this, idx] (void) noexcept -> bool{ return flag_[idx]; });

			if (StrWaitState::SuspendApps == wait_state)
			{
				/* memory_order_acquire: SetVmstrValue()による memory_order_release での書き込みが */
				/* 確実に可視化されることを保証。ReadAcquire/WriteRelease パターンの同期 */
				result = vmstr_value_.load(std::memory_order_acquire);
			}
		}
		/* VideoHAL開始処理待ち */
		else
		{
			std::unique_lock<std::mutex> lock_sync{sync_mtx_[idx]};
			/* ラムダ内での条件チェックにより、スプリアスウェイクアップ対応 */
			/* 18秒のタイムアウト待ち */
			const bool intime{sync_cond_[idx].wait_for(lock_sync, std::chrono::milliseconds(kStrTimeout),
				/* coverity[cert_ctr50_cpp_violation] */
				[this, idx] (void) noexcept -> bool{ return flag_[idx]; })};

			/* タイムアウト発生 */
			if (!intime)
			{
				result = VHAL_ERR_TIMEOUT;
			}
		}
	}
	/* 引数異常 */
	else
	{
		VHAL_LOGE("param err. idx=%u", idx);
		result = VHAL_ERR;
	}

	return result;
}

/*****************************************************************************
 処理概要：	イベント待ち解除
 引数    ：	StrWaitState	wait_state	待ちイベント種別
 戻り値  ：	なし
*****************************************************************************/
void CVhalStrManager::NotifyOne(const StrWaitState wait_state)
{
	const uint32_t		idx{EnumToU32(wait_state)};

	/* SysDB検出待ち */
	/* VideoHAL終了処理待ち */
	/* システム復帰待ち */
	/* VideoHAL開始処理待ち */
	if (kStrWaitStateMax > idx)
	{
		{
			const std::lock_guard<std::mutex> lock_sync{sync_mtx_[idx]};
			flag_[idx] = true;
		}
		sync_cond_[idx].notify_one();
	}
	/* 引数異常 */
	else
	{
		VHAL_LOGE("param err. idx=%u", idx);
	}
}

/*****************************************************************************
 処理概要：	StrWaitStateからの数値キャスト
 引数    ：	StrWaitState	wait_state	待ちイベント種別
 戻り値  ：	uint32_t					数値化
*****************************************************************************/
constexpr uint32_t CVhalStrManager::EnumToU32(const StrWaitState wait_state) noexcept
{
	const uint32_t idx{static_cast<uint32_t>(wait_state)};
	uint32_t result{idx};
	if (kStrWaitStateMax <= idx)
	{
		result = kStrWaitStateMax;
	}
	return result;

}

} /* namespace videohal */
