/*******************************************************************************
    機能名称    ：  通信シーケンス管理モジュール
    ファイル名称：  vhal_net_sequence_manager.h
*******************************************************************************/
#ifndef	VHAL_NET_SEQUENCE_MANAGER_H
#define	VHAL_NET_SEQUENCE_MANAGER_H

#include <algorithm>
#include <cstdint>
#include <mutex>
#include <string>
#include <list>
#include <memory>
#include <vector>

namespace videohal
{

/*****************************************************************************
 クラス名称：CVhalNetCommandItem
 処理概要  ：通信コマンドアイテム管理を行う。
*****************************************************************************/
class CVhalNetCommandItem {
public:

	explicit CVhalNetCommandItem(const uint32_t command_id) noexcept;
	CVhalNetCommandItem(void) = default;
	virtual ~CVhalNetCommandItem(void) = default;
  	CVhalNetCommandItem(const CVhalNetCommandItem& src) = delete;
	CVhalNetCommandItem(CVhalNetCommandItem&& src) = delete;

	/* コマンドID取得 */
	uint32_t GetOpc(void) const noexcept;
	/* 送信データ構築 */
	virtual void Build(std::vector<uint8_t> &send_data) const noexcept = 0;
	/* 応答コマンドあり/なし種別取得（StartResultコマンドの場合はtrue） */
	virtual bool GetResponseType(void) const noexcept = 0;

private:

	CVhalNetCommandItem& operator=(const CVhalNetCommandItem& src) & = delete;
	CVhalNetCommandItem& operator=(CVhalNetCommandItem&& src) & = delete;

	uint32_t		opc_;
};


/*****************************************************************************
 クラス名称：CVhalNetSequenceItem
 処理概要  ：通信コマンド管理を行う。
*****************************************************************************/
class CVhalNetSequenceItem {
public:

	explicit CVhalNetSequenceItem(const uint32_t id) noexcept;
	CVhalNetSequenceItem(void) = default;
	virtual ~CVhalNetSequenceItem(void);
  	CVhalNetSequenceItem(const CVhalNetSequenceItem& src) = delete;
	CVhalNetSequenceItem& operator=(const CVhalNetSequenceItem& src) & = default;
	CVhalNetSequenceItem(CVhalNetSequenceItem&& src) = delete;
	CVhalNetSequenceItem& operator=(CVhalNetSequenceItem&& src) & = delete;

	void Finalize(void) noexcept;

	void QueueCommand(std::unique_ptr<CVhalNetCommandItem> p_command_item) noexcept;		/* コマンドアイテム追加 */
	std::unique_ptr<CVhalNetCommandItem> DequeCommand(void) noexcept;						/* コマンドアイテム取得/削除 */
	uint32_t GetSequenceId(void) const noexcept;										/* シーケンスID取得 */

private:

	mutable std::mutex	mtx_item_;

	uint32_t		sequence_id_;

	/* コマンドアイテムリスト */
	std::list< std::unique_ptr<CVhalNetCommandItem> >	command_list_;
};

/*****************************************************************************
 クラス名称：CVhalNetSequenceManager
 処理概要  ：通信シーケンス管理を行う。
*****************************************************************************/
class CVhalNetSequenceManager final {
public:

	CVhalNetSequenceManager(void) noexcept;
	~CVhalNetSequenceManager(void);
  	CVhalNetSequenceManager(const CVhalNetSequenceManager& src) = delete;
	CVhalNetSequenceManager& operator=(const CVhalNetSequenceManager& src) & = default;
	CVhalNetSequenceManager(CVhalNetSequenceManager&& src) = delete;
	CVhalNetSequenceManager& operator=(CVhalNetSequenceManager&& src) & = delete;

	void Finalize(void) noexcept;

	void QueueSequence(std::unique_ptr<CVhalNetSequenceItem> p_sequence_item) noexcept;		/* シーケンスアイテム追加 */
	std::unique_ptr<CVhalNetSequenceItem> DequeSequence(void) noexcept;						/* シーケンスアイテム取得/削除 */

private:

	void EraseSequence(const uint32_t sequence_id) noexcept;								/* シーケンスアイテム削除（間引き用） */

	mutable std::mutex	mtx_item_;

	/* シーケンスアイテムリスト */
	std::list< std::unique_ptr<CVhalNetSequenceItem> >	sequence_list_;
};

} /* namespace videohal */


#endif	/* #ifndef	VHAL_NET_SEQUENCE_MANAGER_H */
