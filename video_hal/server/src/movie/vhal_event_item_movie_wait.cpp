/*******************************************************************************
    機能名称    ：  動画再生処理待ちイベントモジュール
    ファイル名称：  vhal_event_item_movie_wait.cpp
*******************************************************************************/
#include "vhal_event_item_movie_wait.h"

#include "vhal_define.h"
#include "vhal_log.h"
#include "vhal_movie_control.h"

namespace videohal
{

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalEventItemMovieWait::CVhalEventItemMovieWait(void)
{
	VHAL_LOGV("CVhalEventItemMovieWait is created. this=%p", this);
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalEventItemMovieWait::~CVhalEventItemMovieWait(void)
{
	VHAL_LOGV("CVhalEventItemMovieWait is deleted. this=%p", this);
}

/*****************************************************************************
 処理概要：	イベント処理の実施
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalEventItemMovieWait::Exec(void) const
{
	/* iviイベントを受信したときの処理 */
	VHAL_LOGD("CVhalEventItemMovieWait::Exec called.");

	if ( nullptr != p_movie_ )
	{
		p_movie_->NotifyMovieFinished(result_);
	}
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	データ設定
 引数    ：	CVhalMovieControl*	p_movie		(i) 動画制御インスタンスポインタ
 引数    ：	int32_t				result		(i) 動画再生結果
 戻り値  ：	なし
*****************************************************************************/
void CVhalEventItemMovieWait::SetData(CVhalMovieControl* const p_movie, const int32_t result) noexcept
{
	p_movie_ = p_movie;
	result_ = result;
}

} /* namespace videohal */

