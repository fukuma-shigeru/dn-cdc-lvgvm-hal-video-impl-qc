/*******************************************************************************
    機能名称    ：  バックアップ制御モジュール
    ファイル名称：  vhal_backup_control.cpp
*******************************************************************************/
#include "vhal_backup_control.h"

#include "vhal_define.h"
#include "vhal_log.h"
#include "vhal_round_cast.h"
#include "vhal_debug_system.h"

extern "C"
{
#include "sif_util.h"
}

namespace videohal
{

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalBackupControl::CVhalBackupControl(void)
{
	VHAL_LOGV("CVhalBackupControl is created. this=%p", this);
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalBackupControl::~CVhalBackupControl(void)
{
	VHAL_LOGV("CVhalBackupControl is deleted. this=%p", this);
}

/*****************************************************************************
 処理概要：	データ属性の取得
 引数    ：	const std::string &name					(i)リストア対象のデータ識別文字列
           	SysBkupMngAttr &attr					(o)バックアップデータ属性構造体
 戻り値  ：	処理結果
           		VHAL_ERR			異常終了
           		VHAL_SUCCESS		正常終了
 フェールセーフNo  ：	F-VHAL-N-129
                     	F-VHAL-R-130
*****************************************************************************/
int32_t CVhalBackupControl::GetAttr(const std::string &name, SysBkupMngAttr &attr)
{
	int32_t	ret{VHAL_SUCCESS};
	int32_t ret_bkup{SYS_BKUP_MNG_RET_OK};
	uint32_t count{0U};

	/* 起動直後、SysBkupMngGetAttrからエラーが返るため、リトライで回避 */
	constexpr uint32_t kRetryMaxStartup{100U};		/* SysBkupMngアクセスリトライ回数 */
	bool done{false};
	while((!done) && (count < kRetryMaxStartup))
	{
		++count;
		/* データ属性の取得 */
		ret_bkup = SysBkupMngGetAttr(name.c_str(), &attr);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//		CVhalDebugSystem::GetInstance().UpdateFailData("F-VHAL-R-130",ret_bkup, count);
//#endif
		if (SYS_BKUP_MNG_RET_OK == ret_bkup)
		{
			ret = VHAL_SUCCESS;
			done = true;
		}
		else if (SYS_BKUP_MNG_RET_ER_STAT == ret_bkup)
		{
			if (kRetryMaxStartup <= count)
			{
				ret = VHAL_ERR;
				done = true;
			}
			else
			{
				constexpr uint32_t kRetryWaitStartup{100U};	/* SysBkupMngアクセスリトライ待ち時間(ms) */
				sif_mdelay(kRetryWaitStartup);
			}
		}
		else
		{
			ret = VHAL_ERR;
			done = true;
		}
	}

	if (VHAL_SUCCESS == ret)
	{
		VHAL_LOGI("SysBkupMngGetAttr(%s) count=%d/%d ret_bkup=%d", name.c_str(), count, kRetryMaxStartup, ret_bkup);
	}
	else
	{
		VHAL_LOGE("SysBkupMngGetAttr(%s) count=%d/%d ret_bkup=%d", name.c_str(), count, kRetryMaxStartup, ret_bkup);
	}

	return ret;
}

/*****************************************************************************
 処理概要：	データリストア
 引数    ：	const std::string &name					(i)リストア対象のデータ識別文字列
           	std::vector<uint8_t> &backup_data		(o)リストアするデータを格納するバッファ
 戻り値  ：	処理結果
           		VHAL_ERR			異常終了
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_SUCCESS		正常終了
 フェールセーフNo  ：	F-VHAL-N-132
*****************************************************************************/
int32_t CVhalBackupControl::Load(const std::string &name, std::vector<uint8_t> &backup_data)
{
	int32_t	ret{VHAL_SUCCESS};

	/* 引数チェック */
	if ((true == name.empty()) || (true == backup_data.empty()))
	{
		VHAL_LOGE("parameter error. name=%s, backup_data.size=%lu", name.c_str(), backup_data.size());
		ret = VHAL_ERR_PARAM;
	}
	else
	{
		/* データ属性の取得 */
		SysBkupMngAttr attr{0U};
		ret = GetAttr(name, attr);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("GetAttr(%s) error. ret=%d", name.c_str(), ret);
			ret = VHAL_ERR;
		}
		else
		{
			/* サイズチェック */
			if (backup_data.size() != attr.size)
			{
				VHAL_LOGE("SysBkupMngGetAttr(%s) unmatch attr.size=%d, request size=%lu", name.c_str(), attr.size, backup_data.size());
				ret = VHAL_ERR;
			}
			else
			{
				/* リストア */
				int32_t ret_bkup{SYS_BKUP_MNG_RET_OK};
				ret_bkup = SysBkupMngLoad(name.c_str(), backup_data.data(), SizeToUI32(static_cast<std::size_t>(backup_data.size())));
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//				int32_t fail_ret{0};
//				bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-132",fail_ret)};
//				if(true == fail)
//				{
//					ret_bkup = fail_ret;
//				}
//#endif
				if (SYS_BKUP_MNG_RET_OK != ret_bkup)
				{
					VHAL_LOGE("SysBkupMngLoad(%s) error. ret=%d", name.c_str(), ret_bkup);
					ret = VHAL_ERR;
				}
				else
				{
					ret = VHAL_SUCCESS;
				}
			}
		}
	}

	return ret;
}

/*****************************************************************************
 処理概要：	データバックアップ
 引数    ：	const std::string &name					(i)バックアップ対象のデータ識別文字列
           	std::vector<uint8_t> &backup_data		(i)バックアップするデータを格納するバッファ
           	const bool checkAttr					(i)データ属性チェック あり/なし
 戻り値  ：	処理結果
           		VHAL_ERR			異常終了
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_SUCCESS		正常終了
 フェールセーフNo  ：	F-VHAL-N-131
*****************************************************************************/
int32_t CVhalBackupControl::Save(const std::string &name, std::vector<uint8_t> &backup_data, const bool checkAttr)
{
	int32_t	ret{VHAL_SUCCESS};

	/* 引数チェック */
	if ((true == name.empty()) || (true == backup_data.empty()))
	{
		VHAL_LOGE("parameter error. name=%s, backup_data.size=%lu", name.c_str(), backup_data.size());
		ret = VHAL_ERR_PARAM;
	}
	else
	{
		int32_t ret_bkup{SYS_BKUP_MNG_RET_OK};
		SysBkupMngAttr attr{0U};
		if (true == checkAttr)
		{
			/* データ属性の取得 */
			ret_bkup = SysBkupMngGetAttr(name.c_str(), &attr);
		}

		if ((false == checkAttr) ||
			((SYS_BKUP_MNG_RET_OK == ret_bkup) && (backup_data.size() == attr.size)))
		{
			/* バックアップ */
			ret_bkup = SysBkupMngSave(name.c_str(), backup_data.data(), SizeToUI32(static_cast<std::size_t>(backup_data.size())));
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//			int32_t fail_ret{0};
//			bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-131",fail_ret)};
//			if(true == fail)
//			{
//				ret_bkup = fail_ret;
//			}
//#endif
			if (SYS_BKUP_MNG_RET_OK != ret_bkup)
			{
				VHAL_LOGE("SysBkupMngSave(%s) error. ret=%d", name.c_str(), ret_bkup);
				ret = VHAL_ERR;
			}
			else
			{
				ret = VHAL_SUCCESS;
			}
		}
		else
		{
			VHAL_LOGE("SysBkupMngGetAttr(%s) error. ret=%d, attr.size=%d, request size=%lu", name.c_str(), ret_bkup, attr.size, backup_data.size());
			ret = VHAL_ERR;
		}
	}

	return ret;
}

} /* namespace videohal */
