/*******************************************************************************
    機能名称    ：  内部イベント送信ルートモジュール
    ファイル名称：  vhal_event_route.cpp
*******************************************************************************/
#include "vhal_event_route.h"

#include <string>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "vhal_define.h"
#include "vhal_log.h"
#include "vhal_str_mng.h"
#include "vhal_debug_system.h"

extern "C"
{
#include "sif_util.h"
}

namespace videohal
{

namespace
{
constexpr uint32_t kVhalEventRetryCount{15U};	/* EventRouteのイベントリトライ回数 */
constexpr uint32_t kVhalEventRetryWait{100U};	/* EventRouteのイベントリトライ待ち時間(ms) */
}

int32_t CVhalEventRoute::route_counter_{0};

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalEventRoute::CVhalEventRoute(void)
{
	VHAL_LOGV("CVhalEventRoute is created. this=%p", this);
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalEventRoute::~CVhalEventRoute(void)
{
	VHAL_LOGV("CVhalEventRoute is deleted. this=%p", this);
	Finalize();
}

/*****************************************************************************
 処理概要：	初期化処理（内部リソースの確保）
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_ERR_SOCKET		socket処理エラー
           		VHAL_SUCCESS		正常終了
 フェールセーフNo  ：	F-VHAL-C-014
*****************************************************************************/
int32_t CVhalEventRoute::Initialize(void)
{
	int32_t ret;
	if (INT32_MAX == route_counter_)
	{
		VHAL_LOGW("route_counter_ reached max.");
		route_counter_ = 0;
	}
	route_id_ = route_counter_ ++;

	ret = socketpair(AF_UNIX, static_cast<int>(SOCK_STREAM), 0, pair_fd_);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	int32_t fail_ret{0};
//	bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-C-014",fail_ret)};
//	if(true == fail)
//	{
//		ret = fail_ret;
//	}
//#endif
	if (0 > ret)
	{
		VHAL_LOGE("sockerpair error. ret=%d", ret);
		return VHAL_ERR_SOCKET;
	}

	VHAL_LOGD("sockerpair created. fd=%d,%d", pair_fd_[0], pair_fd_[1]);
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	終了処理（内部リソースの解放）
 引数    ：	なし
 戻り値  ：	なし
 フェールセーフNo  ：	F-VHAL-C-017
*****************************************************************************/
void CVhalEventRoute::Finalize(void)
{
	if (0 != pair_fd_[kSocketPairRead])
	{
		errno = 0;
		const int32_t ret{close(pair_fd_[kSocketPairRead])};
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//		int32_t fail_ret{ret};
//		CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-C-017-1",fail_ret);
//		int32_t* p_fail_ret{const_cast<int32_t*>(&ret)};
//		*p_fail_ret = fail_ret;
//#endif
		if(0 > ret)
		{
			VHAL_LOGE("close error errno=%d", errno);
		}
		pair_fd_[kSocketPairRead] = 0;
	}

	if (0 != pair_fd_[kSocketPairWrite])
	{
		errno = 0;
		const int32_t ret{close(pair_fd_[kSocketPairWrite])};
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//		int32_t fail_ret{ret};
//		CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-C-017-2",fail_ret);
//		int32_t* p_fail_ret{const_cast<int32_t*>(&ret)};
//		*p_fail_ret = fail_ret;
//#endif
		if(0 > ret)
		{
			VHAL_LOGE("close error errno=%d", errno);
		}
		pair_fd_[kSocketPairWrite] = 0;
	}
}

/*****************************************************************************
 処理概要：	イベント受信用fdの取得
 引数    ：	なし
 戻り値  ：	イベント受信用fd
*****************************************************************************/
int32_t CVhalEventRoute::GetSourceFd(void) const noexcept
{
	return pair_fd_[kSocketPairRead];
}

/*****************************************************************************
 処理概要：	イベントの読み込みと実行
 引数    ：	uint32_t	source_event	(i)イベントフラグ
 戻り値  ：	処理結果
           		VHAL_ERR_SOCKET		socket処理エラー
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalEventRoute::ExecEvent(const uint32_t source_event)
{
	VHAL_LOGV_IN();

	if (0U != (source_event & (kSourceEventHungup | kSourceEventError)))
	{
		/* 内部イベント待ち受けソケットでエラー発生。 */
		VHAL_LOGW("CVhalEventRoute: received fd error event. source_event=%d", source_event);
		return VHAL_ERR_SOCKET;
	}

	if (0U != (source_event & kSourceEventReadable))
	{
		CVhalEventItemBase * const p_item{ReadEvent()};
		if (nullptr != p_item)
		{
			/* サスペンド状態の場合は破棄 */
			if (videohal::CVhalStrManager::GetSuspend())
			{
				VHAL_LOGV("ExecEvent(route) discard item.");
			}
			else
			{
				const int32_t ret{p_item->Exec()};
				if (VHAL_SUCCESS != ret)
				{
					VHAL_LOGE("p_item->Exec ret=%d", ret);
				}
			}
			delete p_item;
		}
	}

	VHAL_LOGV_OUT();
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	イベントアイテムの書き込み
 引数    ：	CVhalEventItemBase	*p_item		(i)イベントアイテムインスタンスポインタ
 戻り値  ：	処理結果
           		VHAL_ERR_****		異常終了
           		VHAL_SUCCESS		正常終了
 フェールセーフNo  ：	F-VHAL-R-015A
                     	F-VHAL-R-015B
*****************************************************************************/
int32_t CVhalEventRoute::WriteEvent(CVhalEventItemBase *p_item) const
{
	int32_t	result{VHAL_SUCCESS};
	VHAL_LOGV_IN();

	/* サスペンド状態の場合は破棄 */
	if (videohal::CVhalStrManager::GetSuspend())
	{
		VHAL_LOGV("WriteEvent(route) discard item.");
		return VHAL_ERR_SUSPEND;
	}

	for (uint32_t i{0U}; i<kVhalEventRetryCount; ++i)
	{
		bool loop_end{false};
		errno = 0;
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//		ssize_t ret{0};
//		int32_t fail_ret{0};
//		bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-R-015",fail_ret)};
//		if(true == fail)
//		{
//			ret = -1;
//			errno = fail_ret;
//		}
//		else
//		{
//			ret = write(pair_fd_[kSocketPairWrite], &p_item, sizeof(p_item));
//		}
//#else
		const ssize_t ret{write(pair_fd_[kSocketPairWrite], &p_item, sizeof(p_item))};
//#endif
		if (0 > ret)
		{
			if((EINTR == errno) || (EIO == errno))
			{
				/* リトライ */
				if (kVhalEventRetryCount <= (i+1U))
				{
					VHAL_LOGE("write error errno=%d, retry over=%d",errno ,i+1U);
					result = VHAL_ERR_SOCKET;
					loop_end = true;
				}
				else
				{
					sif_mdelay(kVhalEventRetryWait);
				}
			}
			else
			{
				/* 失敗 */
				VHAL_LOGE("write error errno=%d.",errno);
				result = VHAL_ERR_SOCKET;
				loop_end = true;
			}
		}
		else
		{
			loop_end = true;
		}
		if (true == loop_end)
		{
			break;
		}
	}

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	イベントアイテムの読み出し
 引数    ：	なし
 戻り値  ：	イベントアイテムインスタンスポインタ
 フェールセーフNo  ：	F-VHAL-R-016A
                     	F-VHAL-R-016B
*****************************************************************************/
CVhalEventItemBase *CVhalEventRoute::ReadEvent(void) const
{
	CVhalEventItemBase *p_item{nullptr};
	ssize_t read_size{};

	VHAL_LOGV_IN();

	for (uint32_t i{0U}; i<kVhalEventRetryCount; ++i)
	{
		errno = 0;
		read_size = read(pair_fd_[kSocketPairRead], &p_item, sizeof(p_item));
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//		int32_t fail_ret{0};
//		bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-R-016",fail_ret)};
//		if(true == fail)
//		{
//			read_size = -1;
//			errno = fail_ret;
//		}
//#endif
		if (0 > read_size)
		{
			if ((EINTR == errno) || (EIO == errno))
			{
				/* リトライ */
				if (kVhalEventRetryCount <= (i+1U))
				{
					VHAL_LOGE("read(%ld) error errno=%d, retry over=%d",sizeof(p_item), errno, i+1U);
					return nullptr;
				}
				sif_mdelay(kVhalEventRetryWait);
			}
			else
			{
				/* 失敗 */
				VHAL_LOGE("read(%ld) error. errno=%d, read_size=%ld", sizeof(p_item), errno, read_size);
				return nullptr;
			}
		}
		else
		{
			break;
		}
	}

	if (static_cast<size_t>(read_size) != sizeof(p_item))
	{
		VHAL_LOGE("read size mismatch. read=%ld, expected=%ld", read_size, sizeof(p_item));
		return nullptr;
	}

	VHAL_LOGV_OUT();
	return p_item;
}

} /* namespace videohal */

