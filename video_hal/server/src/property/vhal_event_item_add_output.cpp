/*******************************************************************************
    機能名称    ：  出力デバイス追加イベントモジュール
    ファイル名称：  vhal_event_item_add_output.cpp
*******************************************************************************/
#include "vhal_event_item_add_output.h"

#include "vhal_define.h"
#include "vhal_log.h"
#include "vhal_layout_mng.h"

namespace videohal
{

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalEventItemAddOutput::CVhalEventItemAddOutput(void)
	:p_layout_mng_(nullptr)
	,width_{}
	,height_{}
{
	VHAL_LOGV("CVhalEventItemAddOutput is created. this=%p", this);
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalEventItemAddOutput::~CVhalEventItemAddOutput(void)
{
	VHAL_LOGV("CVhalEventItemAddOutput is deleted. this=%p", this);
}

/*****************************************************************************
 処理概要：	イベント処理の実施
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalEventItemAddOutput::Exec(void) const
{
	/* iviイベントを受信したときの処理 */
	VHAL_LOGD("CVhalEventItemAddOutput::Exec called.");

	if ( nullptr != p_layout_mng_ )
	{
		p_layout_mng_->NotifyOutputAdded(width_, height_, model_, make_);
	}
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	データ設定
 引数    ：	CVhalLayoutManager*		p_layout_mng	(i) レイアウト管理インスタンスポインタ
          	int32_t 				width			(i) 出力デバイス先の画面幅
          	int32_t 				height			(i) 出力デバイス先の画面高さ
          	const std::string& 		model			(i) 出力デバイス先のモデル名
          	const std::string& 		make			(i) 出力デバイス先の製造名
 戻り値  ：	なし
*****************************************************************************/
void CVhalEventItemAddOutput::SetData(CVhalLayoutManager* const p_layout_mng, const int32_t width, const int32_t height, const std::string& model, const std::string& make)
{
	p_layout_mng_ = p_layout_mng;
	width_  = width;
	height_ = height;
	model_  = model;
	make_   = make;
}

} /* namespace videohal */

