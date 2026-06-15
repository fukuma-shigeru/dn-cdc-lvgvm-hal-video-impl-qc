/*******************************************************************************
    機能名称    ：  通信サーバーモジュール
    ファイル名称：  vhal_server.cpp
*******************************************************************************/
#include "vhal_server.h"

#include <algorithm>
#include <string>
#include <iostream>
#include <memory>
#include <sys/stat.h>

#include "vhal_define.h"
#include "vhal_log.h"
#include "vhal_property_client.h"
#include "vhal_main_control.h"
#include "vhal_debug_system.h"

namespace videohal
{

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalServer::CVhalServer(void)
	:p_main_(nullptr)
	,halcomm_obj_()
{
	VHAL_LOGV("CVhalServer is created. this=%p", this);
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalServer::~CVhalServer(void)
{
	VHAL_LOGV("CVhalServer is deleted. this=%p", this);
	Finalize();
}

/*****************************************************************************
 処理概要：	初期化処理（内部リソースの確保）
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalServer::Initialize(void) noexcept
{
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	初期化処理（内部リソースの確保）
 引数    ：	CVhalMainControl	*p_main		(i)メインコントロールインスタンスポインタ
 戻り値  ：	処理結果
           		VHAL_ERR_HALCOMM	HalCommエラー
           		VHAL_SUCCESS		正常終了
 フェールセーフNo  ：	F-VHAL-R-001A
                     	F-VHAL-C-001B
*****************************************************************************/
int32_t CVhalServer::Initialize(CVhalMainControl * const p_main)
{
	int32_t result{VHAL_SUCCESS};
	const char *p_dir{nullptr};
	const char *p_file{nullptr};
	size_t len{0U};
	char envdir[VHAL_ENV_VAL_MAX]{0};
	char envfile[VHAL_ENV_VAL_MAX]{0};

	p_main_ = p_main;

	p_dir = secure_getenv("COCKPIT_RUNTIME_DIR_HAL_VIDEO");
	if (nullptr == p_dir)
	{
		p_dir = "/run/arene/share/hal/video";
	}
	len = strnlen(p_dir, VHAL_ENV_VAL_MAX);
	(void)strncpy(&envdir[0], p_dir, len);

	p_file = secure_getenv("COCKPIT_HAL_SOCKFILE");
	if (nullptr == p_file)
	{
		p_file = "sock";
	}
	len = strnlen(p_file, VHAL_ENV_VAL_MAX);
	(void)strncpy(&envfile[0], p_file, len);

	result = MkdirPath(&envdir[0], 0755U);
	if (VHAL_SUCCESS != result)
	{
		VHAL_LOGE("MkdirPath(%s) error. ret=%d", &envdir[0], result);
	}
	else
	{
		uint32_t count{0U};
		int32_t ret_hal{HALCOMM_RET_OK};
		while (true)
		{
			bool loop_end{false};
			ret_hal = HalCommOpen(&halcomm_obj_, &envdir[0], &envfile[0], true);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//			int32_t fail_ret{0};
//			bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-R-001A",fail_ret)};
//			if(true == fail)
//			{
//				ret_hal = fail_ret;
//			}
//#endif
			if (HALCOMM_RET_OK == ret_hal)
			{
				loop_end = true;
			}
			else if ((HALCOMM_RET_ERR_SYS == ret_hal) 
				|| (HALCOMM_RET_ERR_BIND == ret_hal) 
				|| (HALCOMM_RET_ERR_LISTEN == ret_hal) 
				|| (HALCOMM_RET_ERR_NO_OBJ == ret_hal))
			{
				count++;
				
				constexpr uint32_t kHalComOpenRetryCount{120U}; /* Hal通信初期化リトライ回数 */
				if (kHalComOpenRetryCount <= count)
				{
					VHAL_LOGE("HalCommOpen error. ret=%d, retry count=%d", ret_hal , count);
					result = VHAL_ERR_HALCOMM;
					loop_end = true;
				}
			}
			else
			{
				VHAL_LOGE("HalCommOpen error. ret=%d, Not retry. count=%d", ret_hal, count);
				result = VHAL_ERR_HALCOMM;
				loop_end = true;
			}
			if (true == loop_end)
			{
				break;
			}

			constexpr uint32_t kHalComOpenRetryWait{500U}; /* Hal通信初期化リトライ待ち時間(ms) */
			(void)usleep(kHalComOpenRetryWait * 1000U);
		}
		VHAL_LOGV("HalCommOpen ret=%d, retry count=%d", ret_hal , count);

		if (HALCOMM_RET_OK == ret_hal)
		{
			result = p_main->RegisterEventSource(this);
			if (VHAL_SUCCESS != result)
			{
				VHAL_LOGE("RegisterEventSource error. ret=%d", result);
			}
			else
			{
				result = Initialize();
			}
		}
	}

	return result;
}

/*****************************************************************************
 処理概要：	終了処理（内部リソースの解放）
 引数    ：	なし
 戻り値  ：	なし
 フェールセーフNo  ：	F-VHAL-C-007
*****************************************************************************/
void CVhalServer::Finalize(void)
{
	p_main_->ClearEventSource(this, halcomm_obj_.fd);

	clients_.clear();

	if (0 != halcomm_obj_.fd)
	{
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//		int32_t ret{HalCommClose(&halcomm_obj_)};
//		int32_t fail_ret{0};
//		bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-C-007",fail_ret)};
//		if(true == fail)
//		{
//			ret = fail_ret;
//		}
//#else
		const int32_t ret{HalCommClose(&halcomm_obj_)};
//#endif
		if (HALCOMM_RET_OK != ret)
		{
			VHAL_LOGW("HalCommClose error. ret=%d", ret);
		}
	}
}

/*****************************************************************************
 処理概要：	ディレクトリパス作成
 引数    ：	const char	*p_dir		(i)ディレクトリパス名
         ：	mode_t mode				(i)アクセスモード
 戻り値  ：	処理結果
           		VHAL_ERR_RUNTIME	ランタイムエラー（システムコール等）
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalServer::MkdirPath(const char * const p_dir, const mode_t mode)
{
	int32_t	ret;
	size_t	dir_len;
	constexpr size_t max_len{256U};

	dir_len = strnlen(p_dir, max_len);
	if ((0U == dir_len) || (max_len <= dir_len))
	{
		VHAL_LOGE("parameter error. dir_len=%ld", dir_len);
		return VHAL_ERR_PARAM;
	}

	const std::unique_ptr<char[]>	dir_buf{std::make_unique<char[]>(dir_len+1U)};
	(void)strncpy(&dir_buf[0], p_dir, dir_len);
	dir_buf[dir_len] = '\0';

	if(dir_buf[dir_len-1U] == '/')
	{
		dir_buf[dir_len-1U] = '\0';
	}

	for (uint32_t i{1U}; i < dir_len; i++)
	{
		if (dir_buf[i] == '\0')
		{
			break;
		}
		else if (dir_buf[i] == '/')
		{
			dir_buf[i] = '\0';
			errno = 0;
			ret = mkdir(&dir_buf[0], mode);
			if ((ret < 0) && (errno != EEXIST))
			{
				VHAL_LOGE("mkdir error. ret=%d, errno=%d", ret, errno);
				return (VHAL_ERR_RUNTIME);
			}
			dir_buf[i] = '/';
		}
		else
		{
			/* 処理なし */
		}
	}

	errno = 0;
	ret = mkdir(&dir_buf[0], mode);
	if ((ret < 0) && (errno != EEXIST))
	{
		VHAL_LOGE("mkdir error. ret=%d, errno=%d", ret, errno);
		return (VHAL_ERR_RUNTIME);
	}

	return (VHAL_SUCCESS);
}

/*****************************************************************************
 処理概要：	イベント受信用fdの取得
 引数    ：	なし
 戻り値  ：	イベント受信用fd
*****************************************************************************/
int32_t CVhalServer::GetSourceFd(void) const
{
	if (0 == halcomm_obj_.fd)
	{
		VHAL_LOGW("fd is 0. HalComm is not initialized successfully.");
	}

	return halcomm_obj_.fd;
}

/*****************************************************************************
 処理概要：	イベントの読み込みと実行
 引数    ：	uint32_t	source_event		(i)イベントフラグ
 戻り値  ：	処理結果
           		VHAL_ERR_SOCKET		socket処理エラー
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalServer::ExecEvent(const uint32_t source_event)
{
	int32_t	result{VHAL_SUCCESS};
	VHAL_LOGV_IN();

	if (0U != (source_event & (kSourceEventHungup | kSourceEventError)))
	{
		/* クライアント接続待ち受けソケットでエラー発生。 */
		VHAL_LOGE("CVhalServer: received fd error event. source_event=%d", source_event);
		result = VHAL_ERR_SOCKET;
	}
	else
	{
		if (0U != (source_event & kSourceEventReadable))
		{
			std::unique_ptr<CVhalPropertyClient> p_client{AcceptClient()};
			if (nullptr == p_client)
			{
				VHAL_LOGW("AcceptClient error.");
				result = VHAL_ERR_HALCOMM;
			}
			else
			{
				clients_.push_back(std::move(p_client));
			}
		}
	}

	VHAL_LOGV_OUT();
	return result;
}

/*****************************************************************************
 処理概要：	更新プロパティリストを全クライアントに通知する
 引数    ：	std::vector<std::string>	&property_names		(i)更新プロパティリスト
 戻り値  ：	処理結果
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalServer::Publish(std::vector<std::string> &property_names)
{
	VHAL_LOGV_IN();

	for (auto itr_client = clients_.begin(); itr_client != clients_.end(); ++itr_client)
	{
		CVhalPropertyClient * const p_client{itr_client->get()};
		const int32_t ret{p_client->Publish(property_names)};
		if (0 > ret)
		{
			VHAL_LOGW("client Publish error. ret=%d", ret);	
		}
	}

	VHAL_LOGV_OUT();
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	接続してきたクライアントのaccept処理
 引数    ：	なし
 戻り値  ：	クライアントインスタンスポインタ
 フェールセーフNo  ：	F-VHAL-R-002A
                     	F-VHAL-C-002B
*****************************************************************************/
std::unique_ptr<CVhalPropertyClient> CVhalServer::AcceptClient(void)
{
	std::unique_ptr<CVhalPropertyClient> p_client{nullptr};
	halcomm::HalCommObj_t* p_halcomm_client{nullptr};

	VHAL_LOGV_IN();

	/* クライアントインスタンスの作成 */
	p_client = std::make_unique<CVhalPropertyClient>();

	p_halcomm_client = GetHalCommClient(p_client.get());
	if (nullptr == p_halcomm_client)
	{
		VHAL_LOGE("CVhalPropertyClient::getHalCommClient error.");
		p_client = nullptr;
	}
	else
	{
		/* accept処理 */
		uint32_t count{0U};
		int32_t ret_hal{HALCOMM_RET_OK};
		while (true)
		{
			bool loop_end{false};
			ret_hal = HalCommAccept(&halcomm_obj_, p_halcomm_client);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//			int32_t fail_ret{0};
//			bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-R-002A",fail_ret)};
//			if(true == fail)
//			{
//				ret_hal = fail_ret;
//			}
//#endif
			if (HALCOMM_RET_OK == ret_hal)
			{
				loop_end = true;
			}
			else if ((HALCOMM_RET_ERR_SYS == ret_hal) || (HALCOMM_RET_ERR_NO_OBJ == ret_hal))
			{
				count++;
				constexpr uint32_t kHalComAcceptRetryCount{15U};	/* Hal通信通信確立リトライ回数 */
				if (kHalComAcceptRetryCount <= count)
				{
					VHAL_LOGE("HalCommAccept error. ret=%d, retry count=%d", ret_hal , count);
					p_client = nullptr;
					loop_end = true;
				}
			}
			else
			{
				VHAL_LOGE("HalCommAccept error. ret=%d, Not retry. count=%d", ret_hal, count);
				p_client = nullptr;
				loop_end = true;
			}
			if (true == loop_end)
			{
				break;
			}

			constexpr uint32_t kHalComAcceptRetryWait{100U};	/* Hal通信通信確立リトライ待ち時間(ms) */
			(void)usleep(kHalComAcceptRetryWait * 1000U);
		}
		VHAL_LOGV("HalCommAccept ret=%d, retry count=%d", ret_hal , count);

		if (HALCOMM_RET_OK == ret_hal)
		{
			const int32_t	ret{p_client->Initialize(p_main_)};
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("CVhalPropertyClient::Initialize error. ret=%d", ret);
				p_client = nullptr;
			}
		}
	}

	VHAL_LOGV_OUT();
	return p_client;
}

/*****************************************************************************
 処理概要：	クライアント切断通知
 引数    ：	CVhalEventSource	*p_source		(i)イベントソース（プロパティクライアント）インスタンスポインタ
 戻り値  ：	なし
*****************************************************************************/
void CVhalServer::ClientDisconnected(CVhalEventSource * const p_source)
{
	VHAL_LOGV_IN();

	const auto itr_client = std::find_if(clients_.cbegin(), clients_.cend(), 
		[p_source] (const std::unique_ptr<CVhalPropertyClient> &client) noexcept {return client.get() == p_source;});

	if (itr_client == clients_.cend())
	{
		VHAL_LOGW("client is not found.");
	}
	else
	{
		(void)clients_.erase(itr_client);
	}

	VHAL_LOGV_OUT();
}

} /* namespace videohal */

