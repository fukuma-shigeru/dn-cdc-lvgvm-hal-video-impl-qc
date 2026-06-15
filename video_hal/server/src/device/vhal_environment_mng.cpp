/*******************************************************************************
    機能名称    ：  環境制御モジュール
    ファイル名称：  vhal_environment_mng.cpp
*******************************************************************************/
#include "vhal_environment_mng.h"

#include "vhal_define.h"
#include "vhal_log.h"

namespace videohal
{

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalEnvironmentManager::CVhalEnvironmentManager(void)
{
	VHAL_LOGV("CVhalEnvironmentManager is created. this=%p", this);
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalEnvironmentManager::~CVhalEnvironmentManager(void)
{
	VHAL_LOGV("CVhalEnvironmentManager is deleted. this=%p", this);
}

/*****************************************************************************
 処理概要：	初期化処理（内部リソースの確保）
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalEnvironmentManager::Initialize(void)
{
	std::ifstream ifs{"/proc/device-tree/model"};
	std::string model{""};
	if (ifs.is_open())
	{
		(void)getline(ifs, model);

		VHAL_LOGI("board model number (%s)", model.c_str());
		
		/* ボード型番判定 */
		if (std::string::npos != model.find("SA81"))
		{
			board_model_ = BoardModelNumber::kSa81;
		}
		else if(std::string::npos != model.find("SA61"))
		{
			board_model_ = BoardModelNumber::kSa61;
		}
		else
		{
			board_model_ = BoardModelNumber::kUnknown;
		}
	}
	else
	{
		VHAL_LOGE("model file open failure");
		board_model_ = BoardModelNumber::kUnknown;
	}
	
	ifs.close();
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	ボード型番判定結果取得
 引数    ：	なし
 戻り値  ：	ボード型番判定結果
*****************************************************************************/
BoardModelNumber CVhalEnvironmentManager::GetBoardModelNumber(void) const noexcept
{
	return board_model_;
}

} /* namespace videohal */

