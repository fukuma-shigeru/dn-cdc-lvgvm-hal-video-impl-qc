/*******************************************************************************
    機能名称    ：  通信シーケンス管理モジュール
    ファイル名称：  vhal_net_sequence_manager.cpp
*******************************************************************************/
#include "vhal_define.h"
#include "vhal_log.h"
#include "vhal_net_sequence_manager.h"

namespace videohal
{

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	const uint32_t command_id		(i)コマンドID
 戻り値  ：	なし
*****************************************************************************/
CVhalNetCommandItem::CVhalNetCommandItem(const uint32_t command_id) noexcept
	:opc_{command_id}
{
	VHAL_LOGV("opc:%d", command_id);
}

/*****************************************************************************
 処理概要：	コマンドID取得
 引数    ：	なし
 戻り値  ：	コマンドID
*****************************************************************************/
uint32_t CVhalNetCommandItem::GetOpc(void) const noexcept
{
	VHAL_LOGV("opc=%d", opc_);
	return opc_;
}


/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	const uint32_t id			(i)シーケンスID
 戻り値  ：	なし
*****************************************************************************/
CVhalNetSequenceItem::CVhalNetSequenceItem(const uint32_t id) noexcept
	:sequence_id_{id}
	,command_list_{}
{
	VHAL_LOGV("CVhalNetSequenceItem is created. this=%p, sequence_id=%d", this, id);
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalNetSequenceItem::~CVhalNetSequenceItem(void)
{
	VHAL_LOGV("CVhalNetSequenceItem is deleted. this=%p", this);
	Finalize();
}

/*****************************************************************************
 処理概要：	終了処理
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalNetSequenceItem::Finalize(void) noexcept
{
	VHAL_LOGV_IN();
	const std::lock_guard<std::mutex> lock_data{mtx_item_};
	command_list_.clear();
	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	コマンドアイテム追加
          	FIFO管理のためリスト末尾へ追加する。
 引数    ：	CVhalNetCommandItem * const p_command_item	(i)コマンドアイテムポインタ
 戻り値  ：	なし
*****************************************************************************/
void CVhalNetSequenceItem::QueueCommand(std::unique_ptr<CVhalNetCommandItem> p_command_item) noexcept
{
	VHAL_LOGV_IN("p_command_item=%p", p_command_item.get());

	if (nullptr != p_command_item)
	{
		const std::lock_guard<std::mutex> lock_data{mtx_item_};
		/* コマンドアイテム追加 */
		VHAL_LOGD("queue. opc=%d", p_command_item->GetOpc());
		command_list_.push_back(std::move(p_command_item));
	}

	VHAL_LOGV_OUT("command_list_.size=%ld", command_list_.size());
	return;
}

/*****************************************************************************
 処理概要：	コマンドアイテム取得/削除
          	FIFO管理のためリスト先頭アイテムを取得/リストからは削除する。
 引数    ：	なし
 戻り値  ：	コマンドアイテムポインタ（アイテムなし時はnullptr）
*****************************************************************************/
std::unique_ptr<CVhalNetCommandItem> CVhalNetSequenceItem::DequeCommand(void) noexcept
{
	VHAL_LOGV_IN();
	const std::lock_guard<std::mutex> lock_data{mtx_item_};
	std::unique_ptr<CVhalNetCommandItem> command_item{nullptr};

	/* 先頭アイテムを削除 */
	if (false == command_list_.empty())
	{
		command_item = std::move(command_list_.front());
		VHAL_LOGD("deque. opc=%d", command_item->GetOpc());
		command_list_.pop_front();
	}
	else
	{
		VHAL_LOGD("list empty.");
	}

	VHAL_LOGV_OUT("command_list_.size=%ld", command_list_.size());
	return command_item;
}

/*****************************************************************************
 処理概要：	シーケンスID取得
 引数    ：	なし
 戻り値  ：	シーケンスID
*****************************************************************************/
uint32_t CVhalNetSequenceItem::GetSequenceId(void) const noexcept
{
	VHAL_LOGV("id=%d",sequence_id_);
	return sequence_id_;
}


/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalNetSequenceManager::CVhalNetSequenceManager(void) noexcept
	:sequence_list_{}
{
	VHAL_LOGV("CVhalNetSequenceManager is created. this=%p", this);
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalNetSequenceManager::~CVhalNetSequenceManager(void)
{
	VHAL_LOGV("CVhalNetSequenceManager is deleted. this=%p", this);
	Finalize();
}

/*****************************************************************************
 処理概要：	終了処理
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalNetSequenceManager::Finalize(void) noexcept
{
	VHAL_LOGV_IN("");
	const std::lock_guard<std::mutex> lock_data{mtx_item_};
	sequence_list_.clear();
	VHAL_LOGV_OUT("");
}

/*****************************************************************************
 処理概要：	シーケンスアイテム追加
          	FIFO管理のためリスト末尾へ追加する。
          	シーケンスID重複時、既に積まれているアイテムは削除する（間引き）。
 引数    ：	CVhalNetSequenceItem * const p_sequence_item	(i)シーケンスアイテムポインタ
 戻り値  ：	なし
*****************************************************************************/
void CVhalNetSequenceManager::QueueSequence(std::unique_ptr<CVhalNetSequenceItem> p_sequence_item) noexcept
{
	VHAL_LOGV_IN("");

	if (nullptr != p_sequence_item)
	{
		/* シーケンスアイテム削除（重複要求アイテムの間引き） */
		EraseSequence(p_sequence_item->GetSequenceId());

		/* シーケンスアイテム追加 */
		{
			const std::lock_guard<std::mutex> lock_data{mtx_item_};
			VHAL_LOGD("queue. id=%d", p_sequence_item->GetSequenceId());
			sequence_list_.push_back(std::move(p_sequence_item));
		}
	}

	VHAL_LOGV_OUT("sequence_list_.size=%ld", sequence_list_.size());
	return;
}

/*****************************************************************************
 処理概要：	シーケンスアイテム取得/削除
          	FIFO管理のためリスト先頭アイテムを取得/リストからは削除する。
 引数    ：	なし
 戻り値  ：	シーケンスアイテムポインタ（アイテムなし時はnullptr）
*****************************************************************************/
std::unique_ptr<CVhalNetSequenceItem> CVhalNetSequenceManager::DequeSequence(void) noexcept
{
	VHAL_LOGV_IN();
	const std::lock_guard<std::mutex> lock_data{mtx_item_};
	std::unique_ptr<CVhalNetSequenceItem> sequence_item{nullptr};

	/* 先頭アイテムを削除 */
	if (false == sequence_list_.empty())
	{
		sequence_item = std::move(sequence_list_.front());
		VHAL_LOGD("deque. id=%d", sequence_item->GetSequenceId());
		sequence_list_.pop_front();
	}
	else
	{
		VHAL_LOGD("list empty.");
	}

	VHAL_LOGV_OUT("sequence_list_.size=%ld", sequence_list_.size());
	return sequence_item;
}

/*****************************************************************************
 処理概要：	シーケンスアイテム削除（間引き用）
          	シーケンスIDが一致時、既に積まれているアイテムから削除する（間引き）。
 引数    ：	const int32_t sequence_id	(i)シーケンスID
 戻り値  ：	なし
*****************************************************************************/
void CVhalNetSequenceManager::EraseSequence(const uint32_t sequence_id) noexcept
{
	VHAL_LOGV_IN();
	const std::lock_guard<std::mutex> lock_data{mtx_item_};

	/* 指定シーケンスIDのアイテムを削除 */
	const auto it_find = std::find_if(sequence_list_.begin(), sequence_list_.end(),
		[sequence_id](const std::unique_ptr<CVhalNetSequenceItem> &p_sequence_item) noexcept -> bool { return sequence_id == p_sequence_item->GetSequenceId(); });
	if (it_find != sequence_list_.end())
	{
		VHAL_LOGD("erase. id=%d", (*it_find)->GetSequenceId());
		(void)sequence_list_.erase(it_find);
	}

	VHAL_LOGV_OUT();
}

} /* namespace videohal */
