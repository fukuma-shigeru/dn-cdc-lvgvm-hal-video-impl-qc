/*******************************************************************************
    機能名称    ：  画面キャプチャ制御モジュール
    ファイル名称：  vhal_screen_shot_micon.cpp
*******************************************************************************/
#include <cstdint>
#include <fstream>
#include <array>
#include <cerrno>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <map>

#include "vhal_define.h"
#include "vhal_log.h"
#include "vhal_layout_mng.h"
#include "vhal_screen_shot_micon.h"
#include "vhal_main_control.h"
#include "vhal_event_item_screen_shot_event_micon.h"
#include "vhal_debug_system.h"

namespace videohal
{

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalScreenShotMicon::CVhalScreenShotMicon(void) noexcept
	: p_main_{nullptr}
	, p_layout_{nullptr}
	, p_micon_comm_{nullptr}
	, p_route_{nullptr}
	, initialize_failed_{false}
	, dest_filepath_{}
	, p_screenshot_listener_{nullptr}
{
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalScreenShotMicon::~CVhalScreenShotMicon(void)
{
	Finalize();
}

/*****************************************************************************
 処理概要：	初期化処理（内部リソースの確保）
 引数    ：	CVhalMainControl * const p_main_control				(i)メインコントロールインスタンスポインタ
          	CVhalLayoutManager * const p_layout_mng				(i)レイアウト制御インスタンスポインタ
			CVhalMiconCommControl * const p_micon_comm_control	(i)マイコン間通信制御インスタンスポインタ
 戻り値  ：	処理結果
           		VHAL_ERR_****		エラー
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalScreenShotMicon::Initialize(CVhalMainControl * const p_main_control, CVhalLayoutManager * const p_layout_mng, CVhalMiconCommControl * const p_micon_comm_control)
{
	int32_t ret{VHAL_SUCCESS};

	VHAL_LOGV_IN();

	/* 引数ポインタ妥当性チェック */
	if ((nullptr == p_main_control) || (nullptr == p_layout_mng) || (nullptr == p_micon_comm_control))
	{
		VHAL_LOGE("parameter error. null pointer detected");
		ret = VHAL_ERR_PARAM;
	}
	else
	{
		p_main_ = p_main_control;
		p_layout_ = p_layout_mng;
		p_micon_comm_ = p_micon_comm_control;
		
		p_route_ = std::make_unique<CVhalEventRoute>();
		if (nullptr == p_route_)
		{
			ret = VHAL_ERR_MEMORY;
			VHAL_LOGE("new CVhalEventRoute null.");
		}
		else
		{
			ret = p_route_->Initialize();
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("CVhalEventRoute Initialize error. ret=%d", ret);
			}
			else
			{
				ret = p_main_->RegisterEventSource(p_route_.get());
				if (VHAL_SUCCESS != ret)
				{
					VHAL_LOGE("RegisterEventSource error. ret=%d", ret);
				}
				else
				{
					/* スクリーンショット応答待ちタイマ初期化 */
					p_screenshot_send_timer_ = std::make_unique<CVhalScreenShotTimer>(this);
					if (nullptr == p_screenshot_send_timer_)
					{
						ret = VHAL_ERR_MEMORY;
						VHAL_LOGE("new CVhalScreenShotTimer null.");
					}
					else
					{
						ret = p_screenshot_send_timer_->Initialize(p_route_.get(), response_interval, response_cycle);
						if (VHAL_SUCCESS != ret)
						{
							VHAL_LOGE("p_screenshot_send_timer_->Initialize error. ret=%d", ret);
						}
						else
						{
							/* スクリーンショット格納先ファイルパスクリア */
							SetDestFilePath("");
						}
					}
				}
			}
		}
	}

	/* 初期化失敗フラグ設定 */
	if (VHAL_SUCCESS != ret)
	{
		initialize_failed_ = true;
	}

	VHAL_LOGV_OUT("ret=%d", ret);
	return ret;
}

/*****************************************************************************
 処理概要：	終了処理（内部リソースの解放）
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalScreenShotMicon::Finalize(void)
{
	VHAL_LOGV_IN();

	/* タイマ終了 */
	if (nullptr != p_screenshot_send_timer_)
	{
		p_screenshot_send_timer_->Finalize();
		p_screenshot_send_timer_ = nullptr;
	}

	/* イベントリスナー解除 */
	if (nullptr != p_screenshot_listener_)
	{
		ClearEventListener();
		p_screenshot_listener_ = nullptr;
	}

	/* イベントソース解除 */
	if ((nullptr != p_route_) && (nullptr != p_main_))
	{
		p_main_->ClearEventSource(p_route_.get());
		p_route_ = nullptr;
	}

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	イベントリスナー登録
 引数    ：	CVhalScreenShotReceiveEventListenerBase* const p_listener		(i)リスナーポインタ
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM			パラメータ不正
           		VHAL_SUCCESS			正常終了
****************************************************************************/
int32_t CVhalScreenShotMicon::RegisterEventListener(CVhalScreenShotReceiveEventListenerBase* const p_listener)
{
	int32_t ret{VHAL_SUCCESS};

	if (nullptr == p_listener)
	{
		VHAL_LOGE("p_listener is null");
		ret = VHAL_ERR_PARAM;
	}
	else
	{
		p_screenshot_listener_ = p_listener;
	}

	return ret;
}

/*****************************************************************************
 処理概要：	イベントリスナー解除
 引数    ：	なし
 戻り値  ：	なし
****************************************************************************/
void CVhalScreenShotMicon::ClearEventListener(void) noexcept
{
	p_screenshot_listener_ = nullptr;
}

/*****************************************************************************
 処理概要：	画面キャプチャ要求送信
 引数    ：	const int32_t ivi_id		(i)画面キャプチャ対象のIVI ID
			const std::string& filepath	(i)スクリーンショット保存先ファイルパス
 戻り値  ：	処理結果
		   		VHAL_ERR_****		エラー
		   		VHAL_SUCCESS		正常終了
****************************************************************************/	
int32_t CVhalScreenShotMicon::SendScreenShotRequest(const int32_t ivi_id, const std::string& filepath) noexcept
{
	int32_t ret{VHAL_SUCCESS};

	VHAL_LOGV_IN();

	/* 初期化チェック */
	if ((true == initialize_failed_) || (nullptr == p_layout_))
	{
		VHAL_LOGE("error. initialize_failed_=%d p_layout_=%p", static_cast<int32_t>(initialize_failed_), p_layout_);
		ret = VHAL_ERR_NOT_INITIALIZED;
	}
	else
	{
		/* スクリーン有効判定 */
		const bool available{ p_layout_->IsScreenAvailable(ivi_id) };
		if (false == available)
		{
			VHAL_LOGE("IsScreenAvailable returned false.");
			ret = VHAL_ERR_PARAM;
		}
		else
		{
			/* ivi_idからスクリーン種別変換 */
			uint8_t screen_type{0U};
			ret = ConvertScreenType(ivi_id, screen_type);
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("ConvertScreenType error. ret=%d", ret);
				ret = VHAL_ERR_PARAM;
			}
			else
			{
				/* スクリーンショット要求送信 */
				CVhalScreenShotSendItem send_item{};
				send_item.SetScreenType(screen_type);	/* スクリーン種別 */

				/* スクリーンショット格納先ファイルパスチェック */
				ret = CheckDestPath(filepath);
				if (VHAL_SUCCESS != ret)
				{
					VHAL_LOGE("ScreenShotFilePath error. %s", filepath.c_str());
					ret = VHAL_ERR_PARAM;
				}
				else
				{
					/* スクリーンショット要求送信前に、スクリーンショット格納先ファイルパス設定 */
					SetDestFilePath(filepath);
					/* スクリーンショット要求送信 */
					ret = p_micon_comm_->Send(send_item);
					if (VHAL_SUCCESS != ret)
					{
						VHAL_LOGE("Send() error. ret=%d", ret);
						ret = VHAL_ERR;
					}
					else
					{
						/* スクリーンショット応答待ちタイマスタート */
						ret = StartTimer();
						if (VHAL_SUCCESS != ret)
						{
							VHAL_LOGE("StartTimer() error. ret=%d", ret);
							ret = VHAL_ERR_TIMER;
						}
						else
						{
							VHAL_LOGI("ScreenShot Request sent successfully.");
						}
					}
				}
			}
		}
	}

	/* エラー発生時は次回要求ブロック防止のため格納先ファイルパスをクリアする */
	if (VHAL_SUCCESS != ret)
	{
		SetDestFilePath("");
	}

	VHAL_LOGV_OUT("ret=%d", ret);
	return ret;
}

/*****************************************************************************
 処理概要：	格納先ファイルパスチェック
 引数    ：	const std::string& filepath	(i)格納先ファイルパス
 戻り値  ：	処理結果
		   		VHAL_ERR_PARAM		パラメータ不正
		   		VHAL_SUCCESS		正常終了
****************************************************************************/	
int32_t CVhalScreenShotMicon::CheckDestPath(const std::string& filepath) const noexcept
{
	int32_t ret{VHAL_ERR_PARAM};

	VHAL_LOGV_IN();

	if (true == filepath.empty())
	{
		VHAL_LOGE("FilePath is empty.");
	}
	else
	{
		const uint8_t first_char{ static_cast<uint8_t>(filepath.front()) };
		if (static_cast<uint8_t>('/') != first_char)
		{
			/* 絶対パス以外は不許可 */
			VHAL_LOGE("FilePath is not absolute path.");
		}
		else
		{
			/* ディレクトリ確認 */
			const size_t pos{static_cast<size_t>(filepath.find_last_of('/'))};
			/* パスからディレクトリ部分を抽出 */
			std::string folder{};
			if (0U == pos)
			{
				/* ルートディレクトリの場合は"/"をセット */
				folder = std::string("/");
			}
			else
			{
				folder = filepath.substr(0U, pos);
			}
			struct stat st{};
			/* 存在確認 */
			if (0 != ::stat(folder.c_str(), &st))
			{
				VHAL_LOGE("Folder not found: %s", folder.c_str());
			}
			else if (static_cast<mode_t>(S_IFDIR) != (st.st_mode & static_cast<mode_t>(S_IFMT)))
			{
				/* ディレクトリでない */
				VHAL_LOGE("Path exists but is not a directory: %s", folder.c_str());
			}
			else
			{
				/* 成功 */
				ret = VHAL_SUCCESS;
			}
		}
	}

	VHAL_LOGV_OUT("ret=%d", ret);
	return ret;
}

/*****************************************************************************
 処理概要：	画面キャプチャ応答通知
 引数    ：	const VhalScreenShotResult	response		(i)	スクリーンショット取得結果
 戻り値  ：	なし
*****************************************************************************/	
void CVhalScreenShotMicon::NotifyScreenShotMiconResponse(const VhalScreenShotResult response) noexcept
{
	VHAL_LOGV_IN();

	VHAL_LOGI("Received ScreenShot response. result=%d", static_cast<uint8_t>(response));

	/* スクリーンショット格納先ファイルパス取得 */
	std::string filepath{};
	GetDestFilePath(filepath);
	if (false == filepath.empty())
	{
		int32_t result{VHAL_CAPTURE_STS_FAILED};

		/* タイマーストップ */
		StopTimer();

		/* スクリーンショット取得結果から画面キャプチャ結果を設定 */
		if (VhalScreenShotResult::success == response)
		{
			int32_t ret{VHAL_SUCCESS};

			/* スクリーンショットファイルコピー */
			/* 5回、100ms間隔でリトライ */
			const std::string nfs_screenshot_filepath{"/tmp/dc-ivi-pf/nfs/video/display_snapshot.bmp"};
			for(uint32_t cnt{0U}; cnt < copy_retry_count; ++cnt)
			{
				ret = CopyScreenShot(nfs_screenshot_filepath, filepath);
				if (VHAL_SUCCESS == ret)
				{
					result = VHAL_CAPTURE_STS_SUCCESS;
					VHAL_LOGD("CopyScreenShot() success. cnt=%d", cnt);
					break;
				}
				else
				{
					if (cnt < (copy_retry_count - 1U))
					{
						/* 次回リトライまでwait */
						sif_mdelay(copy_retry_wait);
					}
				}
			} 
		}
		/* イベントリスナーに画面キャプチャ結果通知 */
		VHAL_LOGD("result=%d", result);
		if (nullptr != p_screenshot_listener_)
		{
			p_screenshot_listener_->NotifyScreenShotMiconResult(result);
		}
		else
		{
			VHAL_LOGE("p_screenshot_listener_ is null.");
		}
	}
	else
	{
		/* タイマー監視以外に画面キャプチャ応答通知を受信 */
		/* 1) 画面キャプチャ応答タイムアウト後に画面キャプチャ応答通知を受信 */
		/* 2) 画面キャプチャ要求していない状態で画面キャプチャ応答通知を受信 */
		/* 3) 画面キャプチャ要求送信キャンセル(要求待ち状態キャンセル)後に画面キャプチャ応答通知を受信 */
		/* 上記の場合は無視する */
		VHAL_LOGW("ScreenShot response is ignored because no request is in progress.");
	}

	/* スクリーンショット格納先ファイルパスクリア */
	SetDestFilePath("");

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	画面キャプチャ要求送信キャンセル(要求待ち状態キャンセル)
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalScreenShotMicon::CancelScreenShotRequest(void) noexcept
{
	VHAL_LOGV_IN();

	std::string filepath{};
	GetDestFilePath(filepath);
	if (!filepath.empty())
	{
		/* 以降のQNX応答/タイムアウトを必ず無視できるよう、先に要求状態を無効化する */
		SetDestFilePath("");
		/* タイマーストップ */
		StopTimer();
	}

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	ivi_idからスクリーン種別変換
 引数    ：	const int32_t 	ivi_id		(i)ivi_id
		   	uint8_t&		type		(o)スクリーン種別				
 戻り値  ：	処理結果
           		VHAL_ERR			処理エラー
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalScreenShotMicon::ConvertScreenType(const int32_t ivi_id, uint8_t& type) noexcept
{
	int32_t ret{VHAL_ERR};

	VHAL_LOGV_IN();

	/* ivi_idとスクリーン種別の組み合わせ確認 */
	static const std::map<int32_t, VhalScreenShotType> kScreenshotTypes{
		{0, VhalScreenShotType::cid},
		{1, VhalScreenShotType::rse},
		{2, VhalScreenShotType::met},
		{3, VhalScreenShotType::hud}
	};

	const std::map<int32_t, VhalScreenShotType>::const_iterator it{kScreenshotTypes.find(ivi_id)};
	if (it != kScreenshotTypes.end())
	{
		ret = VHAL_SUCCESS;
		type = static_cast<uint8_t>(it->second);
	}

	VHAL_LOGV_OUT("ret=%d", ret);
	return ret;
}

/*****************************************************************************
 処理概要：	タイマスタート
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_ERR_TIMER		Timer処理エラー
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalScreenShotMicon::StartTimer(void)
{
	int32_t ret{VHAL_ERR_TIMER};

	VHAL_LOGV_IN();

	if (nullptr != p_screenshot_send_timer_)
	{
		/* タイマスタート */
		ret = p_screenshot_send_timer_->StartTimer();
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("p_screenshot_send_timer_->StartTimer() error. ret=%d", ret);
		}
	}
	else
	{
		VHAL_LOGE("p_screenshot_send_timer_ is null.");
	}

	VHAL_LOGV_OUT();
	return ret;
}

/*****************************************************************************
 処理概要：	タイマストップ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalScreenShotMicon::StopTimer(void)
{

	VHAL_LOGV_IN();

	if (nullptr != p_screenshot_send_timer_)
	{
		/* タイマ終了 */
		p_screenshot_send_timer_->EndTimer();
	}
	else
	{
		VHAL_LOGE("p_screenshot_send_timer_ is null.");
	}

	VHAL_LOGV_OUT();
}

/*****************************************************************************
 処理概要：	スクリーンショット格納先ファイルパス取得
 引数    ：	std::string& filepath		(o)スクリーンショット保存先ファイルパス	
 戻り値  ：	なし
*****************************************************************************/
void CVhalScreenShotMicon::GetDestFilePath(std::string& filepath) const noexcept
{
	const std::lock_guard<std::mutex> lock_data{mtx_starting_};

	filepath = dest_filepath_;
}

/*****************************************************************************
 処理概要：	スクリーンショット格納先ファイルパス設定
 引数    ：	const std::string& filepath		(i)スクリーンショット保存先ファイルパス	
 戻り値  ：	なし
*****************************************************************************/
void CVhalScreenShotMicon::SetDestFilePath(const std::string& filepath) noexcept
{
	const std::lock_guard<std::mutex> lock_data{mtx_starting_};

	dest_filepath_ = filepath;
}

/*****************************************************************************
 処理概要：	スクリーンショットファイルコピー
 引数    ：	const std::string &src			 (i)コピー元ファイルパス
			const std::string &dest			(i)コピー先ファイルパス
 戻り値  ：	処理結果
           		VHAL_ERR			処理エラー
           		VHAL_SUCCESS		正常終了
 フェールセーフNo  ：	 F-VHAL-R-350
						F-VHAL-R-351
						F-VHAL-R-352
						F-VHAL-R-353
******************************************************************************/
int32_t CVhalScreenShotMicon::CopyScreenShot(const std::string &src, const std::string &dest) noexcept
{
    int32_t ret{VHAL_SUCCESS};

    VHAL_LOGV_IN();

	struct stat st{};
	/* コピー元ファイル存在確認 */
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	int32_t fail_ret{0};
//	int32_t src_file_stat{::stat(src.c_str(), &st)};
//	bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-R-350A",fail_ret)};
//	if(true == fail)
//	{
//		src_file_stat = fail_ret;
//	}
//#else
	errno = 0;
	const int32_t src_file_stat{::stat(src.c_str(), &st)};
//#endif
	if (0 != src_file_stat)
	{
		ret = VHAL_ERR;
		VHAL_LOGE("Screenshot file stat error. errno=%d", errno);
	}
	else
	{
		/* コピー元ファイルサイズチェック */
		const off_t src_file_size{st.st_size};
		if (0 >= src_file_size)
		{
			ret = VHAL_ERR;
			VHAL_LOGE("Screenshot file size is zero.");
		}
		else
		{
			errno = 0;
			std::ifstream src_file{src, std::ios::binary};

			/* ファイルオープン */
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//			fail_ret = 0;
//			bool src_open{src_file.is_open()};
//			bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-R-350B",fail_ret)};
//			if(true == fail)
//			{
//				if (1 == fail_ret)
//				{
//					src_open = false;
//				}
//			}
//#else
			const bool src_open{src_file.is_open()};
			const int32_t src_open_errno{src_open ? 0 : errno};
//#endif
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//			fail_ret = 0;
//			std::ofstream dest_file{dest, std::ios::binary};
//			bool dest_open{dest_file.is_open()};
//			const int32_t src_open_errno{0};
//			const int32_t dest_open_errno{0};
//			fail = CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-R-350C",fail_ret);
//			if(true == fail)
//			{
//				if (1 == fail_ret)
//				{
//					dest_open = false;
//				}
//			}
//#else
			errno = 0;
			std::ofstream dest_file{dest, std::ios::binary};
			const bool dest_open{dest_file.is_open()};
			const int32_t dest_open_errno{dest_open ? 0 : errno};
//#endif
			if ((true != src_open) || (true != dest_open))
			{
				ret = VHAL_ERR;
				VHAL_LOGE("File open failed. src_open=%d errno=%d, dest_open=%d errno=%d",
					src_open,
					src_open_errno,
					dest_open,
					dest_open_errno);
			}
			else
			{
				/* ファイルコピー用のバッファ確保 */
				constexpr int32_t buffer_size{1024};
				std::array<char, buffer_size> file_buffer{};
				constexpr std::streamsize read_size{static_cast<std::streamsize>(buffer_size)};

				std::streamsize read_bytes{0};
				while (VHAL_SUCCESS == ret)
				{
					/* コピー元ファイル読み込み */
					errno = 0;
					static_cast<void>(src_file.read(file_buffer.data(), read_size));
					/* 読み込みエラー（EOF 以外）チェック */
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//					fail_ret = 0;
//					bool src_file_fail{src_file.fail()};
//					bool src_file_eof{src_file.eof()};
//					bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-R-351",fail_ret)};
//					if(true == fail)
//					{
//						if (1 == fail_ret)
//						{
//							src_file_fail = true;
//							src_file_eof = false;
//						}
//					}
//#else
					const bool src_file_fail{src_file.fail()};
					const bool src_file_eof{src_file.eof()};
//#endif
					if ((true == src_file_fail) && (true != src_file_eof))	/* 読み込み結果の確認 */
					{
						ret = VHAL_ERR;
						VHAL_LOGE("Error occurred during file read. fail=%d eof=%d errno=%d",
							static_cast<int32_t>(src_file_fail),
							static_cast<int32_t>(src_file_eof),
							errno);
					}
					else
					{
						/* 読み込みバイト数のチェック */
						read_bytes = src_file.gcount();
						if (0 == read_bytes)
						{
							/* 読み込むデータが無ければ正常終了 */
							break;
						}
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//						int32_t fail_ret{0};
//						static_cast<void>(dest_file.write(file_buffer.data(), read_bytes));
//						bool file_fail{dest_file.fail()};
//						bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-R-352",fail_ret)};
//						if(true == fail)
//						{
//							if (1 == fail_ret)
//							{
//								file_fail = true;
//							}
//						}
//#else	
						/* コピー先ファイル書き込み */
						errno = 0;
						static_cast<void>(dest_file.write(file_buffer.data(), read_bytes));
						const bool file_fail{dest_file.fail()};
//#endif
						if (true == file_fail)	/* 書き込み結果の確認 */
						{
							ret = VHAL_ERR;
							VHAL_LOGE("Error occurred during file write. fail=%d bytes=%ld errno=%d",
								static_cast<int32_t>(file_fail),
								static_cast<int64_t>(read_bytes),
								errno);
						}
					}
				}
				src_file.close();
				dest_file.close();
				if (VHAL_SUCCESS != ret)
				{
					static_cast<void>(::unlink(dest.c_str()));
				}
			}
		}
	}

	if (VHAL_SUCCESS == ret)
	{
		/* fsync を実行してディスクへ反映する */
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//		int32_t fd{::open(dest.c_str(), O_WRONLY)};
//		int32_t fail_ret{0};
//		bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-R-353A",fail_ret)};
//		if(true == fail)
//		{
//			if (0 > fail_ret)
//			{
//				fd = fail_ret;
//			}
//		}
//#else
		const int32_t fd{::open(dest.c_str(), O_WRONLY)};	
//#endif	
		if (0 <= fd)
		{
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//			int32_t fsync_ret{::fsync(fd)};
//			fail_ret = 0;
//			bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-R-353B",fail_ret)};
//			if(true == fail)
//			{
//				if (0 != fail_ret)
//				{
//					fsync_ret = fail_ret;
//				}
//			}
//#else
			const int32_t fsync_ret{::fsync(fd)};
//#endif
			if (0 != fsync_ret)
			{
				ret = VHAL_ERR;
				static_cast<void>(::close(fd));
				static_cast<void>(::unlink(dest.c_str()));
				VHAL_LOGE("fsync failed");
			}
			else
			{
				static_cast<void>(::close(fd));
			}
		}
		else
		{
			ret = VHAL_ERR;
			static_cast<void>(::unlink(dest.c_str()));
			VHAL_LOGE("open() for fsync failed. ");
		}
	}

	VHAL_LOGV_OUT("ret=%d", ret);
	return ret;
}

} /* namespace videohal */

