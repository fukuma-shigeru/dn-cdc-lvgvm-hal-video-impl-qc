/*******************************************************************************
    機能名称    ：  画質調整レコードモジュール
    ファイル名称：  vhal_color_record.cpp
*******************************************************************************/
#include "vhal_color_record.h"

#include "vhal_define.h"
#include "vhal_log.h"

namespace videohal
{

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalColorRecord::CVhalColorRecord(void)
{
	VHAL_LOGV("CVhalColorRecord is created. this=%p", this);
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalColorRecord::~CVhalColorRecord(void)
{
	VHAL_LOGV("CVhalColorRecord is deleted. this=%p", this);
}

/*****************************************************************************
 処理概要：	データコピー
 引数    ：	CVhalColorRecord &dest			(o)コピー先
            const CVhalColorRecord &src		(i)コピー元
 戻り値  ：	なし
*****************************************************************************/
void CVhalColorRecord::Copy(CVhalColorRecord &dest, const CVhalColorRecord &src)
{
	dest = src;
}

} /* namespace videohal */
