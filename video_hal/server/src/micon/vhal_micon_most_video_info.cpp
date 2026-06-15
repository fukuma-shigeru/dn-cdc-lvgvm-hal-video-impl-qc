/*******************************************************************************
    機能名称    ：  MOST映像情報通知送信モジュール
    ファイル名称：  vhal_micon_most_video_info.cpp
*******************************************************************************/
#include "vhal_define.h"
#include "vhal_log.h"

#include "vhal_micon_most_video_info.h"
#include "vhal_micon_comm_control.h"
#include "vhal_layout_video_setting.h"
#include "vhal_round_cast.h"

#include <vector>
#include <algorithm>

namespace videohal
{

namespace
{

}

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalMiconMostVideoInfo::CVhalMiconMostVideoInfo(void) noexcept
	:video_front_path_("")
	,current_video_rear_path_("")
	,current_video_rear_rse_disp_path_("")
	,prev_video_rear_path_("")
	,prev_video_rear_rse_disp_path_("")
	,rear_screen_w_(0)
	,rear_screen_h_(0)
	,mute_rear_disp_(false)
	,p_micon_ctrl_(nullptr)
	,p_most_send_item_videoinfo_(nullptr)
	,p_most_video_info_receiver_(nullptr)
	,p_net_sequence_manager_{nullptr}
	,p_current_sequence_item_{nullptr}
{
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalMiconMostVideoInfo::~CVhalMiconMostVideoInfo(void)
{
	Finalize();
}

/*****************************************************************************
 処理概要：	初期化
 引数    ：	CVhalMiconCommControl* p_micon_comm_control			(i)マイコン間通信制御モジュール
            CVhalFileObserverEventListenerBase* p_listener		(i)リスナーポインタ
 戻り値  ：	処理結果
           		VHAL_ERR				異常終了
           		VHAL_ERR_MEMORY			メモリ確保エラー
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalMiconMostVideoInfo::Initialize(CVhalMiconCommControl* const p_micon_comm_control, CVhalMiconMostReceiveEventListenerBase* const p_listener)
{
	int32_t	ret{VHAL_SUCCESS};

	do
	{
		p_micon_ctrl_ = p_micon_comm_control;

		/* 通信シーケンス管理クラス初期化 */
		p_net_sequence_manager_ = std::make_unique<CVhalNetSequenceManager>();
		if (nullptr == p_net_sequence_manager_)
		{
			VHAL_LOGE("Failed to create CVhalNetSequenceManager.");
			ret = VHAL_ERR_MEMORY;
			break;
		}

		/* MOST映像情報通知送信アイテム初期化 */
		p_most_send_item_videoinfo_ = std::make_unique<CVhalMiconMostSendItemVideoInfo>();
		if (nullptr != p_most_send_item_videoinfo_)
		{
			/* 内部イベントデータ受信CB登録 */
			p_most_video_info_receiver_ = std::make_unique<CVhalMiconMostVideoInfoReceiver>(this, p_micon_ctrl_, p_most_send_item_videoinfo_.get());
			if (nullptr != p_most_video_info_receiver_)
			{
				/* イベントリスナー登録 */
				ret = p_most_video_info_receiver_->RegisterEventListener(p_listener);
				if (VHAL_SUCCESS != ret)
				{
					VHAL_LOGW("CVhalMiconMostVideoInfoReceiver::RegisterEventListener() error. ret=%d", ret);
				}

				/* 内部イベントデータ受信CB登録 */
				ret = p_micon_ctrl_->RegistryReceiver(p_most_video_info_receiver_.get());
				if (VHAL_SUCCESS != ret)
				{
					VHAL_LOGW("CVhalMiconCommControl::RegistryReceiver() error. ret=%d", ret);
				}
			}
			else
			{
				VHAL_LOGE("Failed to create CVhalMiconMostVideoInfoReceiver.");
				ret = VHAL_ERR_MEMORY;
			}
		}
		else
		{
			VHAL_LOGE("Failed to create CVhalMiconMostSendItemVideoInfo.");
			ret = VHAL_ERR_MEMORY;
		}
	} while (false);

	return ret;
}

/*****************************************************************************
 処理概要：	終了処理
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalMiconMostVideoInfo::Finalize(void)
{
	VHAL_LOGD("CVhalMiconMostVideoInfo.");

	if (nullptr != p_current_sequence_item_)
	{
		p_current_sequence_item_ = nullptr;
	}

	if (nullptr != p_net_sequence_manager_)
	{
		p_net_sequence_manager_ = nullptr;
	}

	if (nullptr != p_most_send_item_videoinfo_)
	{
		p_most_send_item_videoinfo_ = nullptr;
	}

	if (nullptr != p_most_video_info_receiver_)
	{
		if (nullptr != p_micon_ctrl_)
		{
			const int32_t ret{p_micon_ctrl_->ClearReceiver(p_most_video_info_receiver_.get())};
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("CVhalMiconCommControl::ClearReceiver error. ret=%d", ret);
			}
		}
		p_most_video_info_receiver_ = nullptr;
	}

	if (nullptr != p_micon_ctrl_)
	{
		p_micon_ctrl_ = nullptr;
	}
}

/*****************************************************************************
 処理概要：	再初期化処理
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalMiconMostVideoInfo::ReInit(void) noexcept
{
	video_front_path_ = "";
	current_video_rear_path_ = "";
	current_video_rear_rse_disp_path_ = "";
	prev_video_rear_path_ = "";
	prev_video_rear_rse_disp_path_ = "";
	mute_rear_disp_ = false;

	/* rear_screen_w_とrear_screen_h_はlayout_mng経由からの設定情報の為クリアしない */
}

/*****************************************************************************
 処理概要：	前席映像パス設定
 引数    ：	const std::string& path		(i)映像パス
 戻り値  ：	処理結果
           		VHAL_ERR				異常終了
           		VHAL_ERR_PARAM			パラメータ不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalMiconMostVideoInfo::SetFrontPath(const std::string& path)
{
	int32_t	ret{VHAL_SUCCESS};

	VHAL_LOGD("path=%s", path.c_str());

	do
	{
		/* 設定値の変化チェック */
		if (0 == video_front_path_.compare(path))
		{
			VHAL_LOGE("not changed video path. front=%s", path.c_str());
			ret = VHAL_ERR_PARAM;
			break;
		}
		video_front_path_ = path;

		/* シーケンスアイテム送信 */
		ret = SendSequenceItem(MostSequenceType::kSetFrontPath);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("SendSequenceItem error. ret=%d", ret);
		}
	} while (false);

	return ret;
}

/*****************************************************************************
 処理概要：	後席映像パス設定
 引数    ：	const std::string& path		(i)映像パス
 戻り値  ：	なし
*****************************************************************************/
void CVhalMiconMostVideoInfo::SetRearPath(const std::string& path)
{
	VHAL_LOGD("path=%s", path.c_str());
	current_video_rear_path_ = path;
}

/*****************************************************************************
 処理概要：	後席専用映像パス設定
 引数    ：	const std::string& path		(i)映像パス
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM			パラメータ不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalMiconMostVideoInfo::SetRearRseDispPath(const std::string& path)
{
	int32_t	ret{VHAL_SUCCESS};

	VHAL_LOGD("path=%s", path.c_str());

	/* 後席専用映像パス */
	const std::vector<std::string> path_list_
	{
		"",
		"REAR-MIRACAST",
		"REAR-BROWSER",
		"REAR-VDSP",
		"REAR-AUDIOSEL",
		"REAR-HDMI",
		"REAR-HDMI2",
	};

	do
	{
		const auto itr_path = std::find(path_list_.begin(), path_list_.end(), path);
		if (itr_path != path_list_.end())
		{
			current_video_rear_rse_disp_path_ = path;
		}
		else
		{
			VHAL_LOGE("invalid video path=%s", path.c_str());
			ret = VHAL_ERR_PARAM;
		}
	} while (false);

	return ret;
}

/*****************************************************************************
 処理概要：	後席映像反映通知
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_ERR				異常終了
           		VHAL_ERR_PARAM			パラメータ不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalMiconMostVideoInfo::Reflect(void)
{
	VHAL_LOGD("");

	int32_t	result{VHAL_SUCCESS};

	do
	{
		/* 後席/後席専用映像パスに変化ない場合は無効 */
		if ((0 == prev_video_rear_path_.compare(current_video_rear_path_)) && (0 == prev_video_rear_rse_disp_path_.compare(current_video_rear_rse_disp_path_)))
		{
			VHAL_LOGE("not changed video path. rear=%s rse=%s", current_video_rear_path_.c_str(), current_video_rear_rse_disp_path_.c_str());
			result = VHAL_ERR_PARAM;
			break;
		}

		/* シーケンスIDを設定 */
		constexpr enum MostSequenceType sequence_id{MostSequenceType::kSetRsePathNotify};

		/* シーケンスアイテム送信 */
		/* 後席専用映像反映通知シーケンス終了後に結果通知するためエラーリターンしない */
		const int32_t	ret{SendSequenceItem(sequence_id)};
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("SendSequenceItem error. ret=%d, sequence_id=%d", ret, sequence_id);
		}
		/* 映像パス前回値保持 */
		prev_video_rear_path_ = current_video_rear_path_;
		prev_video_rear_rse_disp_path_ = current_video_rear_rse_disp_path_;
	} while (false);

	return result;
}


/*****************************************************************************
 処理概要：	RSE接続通知
           	RSE接続に伴う後席専用映像パス設定(MOST)を行う。
 引数    ：	const enum MostConnectedState connected_state	(i)RSE接続状態
 戻り値  ：	処理結果
           		VHAL_ERR_****			異常終了
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalMiconMostVideoInfo::SetConnectedRse(const enum MostConnectedState connected_state) noexcept
{
	VHAL_LOGV_IN("connected_state=%d", connected_state);
	int32_t	ret{VHAL_SUCCESS};

	do
	{
		if (MostConnectedState::kDisConnected != connected_state)
		{
			/* シーケンスIDを設定 */
			enum MostSequenceType sequence_id{MostSequenceType::kRecoveryRse};
			if (MostConnectedState::kConnectedFirst == connected_state)
			{
				/* RSE接続（初回） */
				sequence_id = MostSequenceType::kColdBoot;
				/* コールドブート起動時に、view off（映像パスクリア）を通知する */
				current_video_rear_path_ = "";
				current_video_rear_rse_disp_path_ = "";
			}

			/* シーケンスアイテム送信 */
			ret = SendSequenceItem(sequence_id);
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("SendSequenceItem error. ret=%d, sequence_id=%d", ret, sequence_id);
			}
		}
		else
		{
			/* RSE未接続/脱落 */
			/* カレントシーケンスアイテムをクリア */
			const std::unique_ptr<CVhalNetCommandItem> p_ret_item{ClearCurrentSequenceItem()};
			if (nullptr != p_ret_item)
			{
				/* 切替結果通知（失敗） */
				p_most_video_info_receiver_->ReceiveEvent(VHAL_VPATH_STS_FAILED, p_ret_item.get(), MostUpdateType::kSetProperty);
			}
			/* シーケンスアイテムをクリア */
			p_net_sequence_manager_->Finalize();
		}
	} while (false);

	VHAL_LOGV_OUT("");
	return ret;
}

/*****************************************************************************
 処理概要：	後席ディスプレイ出力矩形を設定
 引数    ：	const int32_t width					(i)幅
           	const int32_t height				(i)高さ
 戻り値  ：	なし
*****************************************************************************/
void CVhalMiconMostVideoInfo::SetRearDisplayRectangle(const int32_t width, const int32_t height) noexcept
{
	/* 後席スクリーンサイズ */
	VHAL_LOGD("%dx%d", width, height);
	rear_screen_w_ = width;
	rear_screen_h_ = height;
}

/*****************************************************************************
 処理概要：	後席ディスプレイ全体のMUTE設定
 引数    ：	const bool mute		(i)true MUTE有効/false MUTE無効
 戻り値  ：	なし
*****************************************************************************/
void CVhalMiconMostVideoInfo::SetMuteRearDisp(const bool mute) noexcept
{
	VHAL_LOGV("mute=%d", mute);
	mute_rear_disp_ = mute;

	/* シーケンスアイテム送信 */
	const int32_t ret{SendSequenceItem(MostSequenceType::kSetMuteRearDisp)};
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("SendSequenceItem error. ret=%d, sequence_id=%d, mute=%d", ret, MostSequenceType::kSetMuteRearDisp, mute);
	}
}

/*****************************************************************************
 処理概要：	シーケンスアイテム送信
 引数    ：	const enum MostSequenceType sequence_id	(i)シーケンスID
 戻り値  ：	処理結果
           		VHAL_ERR_****			異常終了
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalMiconMostVideoInfo::SendSequenceItem(const enum MostSequenceType sequence_id) noexcept
{
	VHAL_LOGV_IN("sequence_id=%d", sequence_id);
	int32_t	ret{VHAL_SUCCESS};

	do
	{
		if (MostSequenceType::kMax > sequence_id)
		{
			/* シーケンスアイテム生成 */
			std::unique_ptr<CVhalNetSequenceItem> p_sequence_item{CreateSequenceItem(sequence_id)};
			if (nullptr != p_sequence_item)
			{
				if (nullptr != p_current_sequence_item_)
				{
					/* カレントシーケンスアイテムがある場合は、 */
					/* シーケンスアイテムリストへ追加 */
					p_net_sequence_manager_->QueueSequence(std::move(p_sequence_item));
				}
				else
				{
					/* カレントシーケンスアイテムがない場合は、 */
					/* カレントシーケンスアイテムとしてコマンド送信開始 */
					p_current_sequence_item_ = std::move(p_sequence_item);
					ret = SendCmd(MostUpdateType::kSetProperty);
					if (VHAL_SUCCESS != ret)
					{
						VHAL_LOGE("SendCmd error. ret=%d, sequence_id=%d", ret, sequence_id);
					}
				}
			}
			else
			{
				VHAL_LOGE("p_sequence_item is null, sequence_id=%d", sequence_id);
				ret = VHAL_ERR;
			}
		}
		else
		{
			VHAL_LOGE("Invalid sequence_id=%d", sequence_id);
			ret = VHAL_ERR_PARAM;
		}
	} while (false);

	VHAL_LOGV_OUT("ret=%d", ret);
	return ret;
}

/*****************************************************************************
 処理概要：	シーケンスアイテム生成
 引数    ：	const enum MostSequenceType sequence_id	(i)シーケンスID
 戻り値  ：	シーケンスアイテムポインタ（失敗時はnullptr）
*****************************************************************************/
std::unique_ptr<CVhalNetSequenceItem> CVhalMiconMostVideoInfo::CreateSequenceItem(const enum MostSequenceType sequence_id) noexcept
{
	VHAL_LOGV_IN("sequence_id=%d", sequence_id);
	std::unique_ptr<CVhalNetSequenceItem> p_sequence_item{nullptr};

	do
	{
		/* シーケンスアイテム生成 */
		p_sequence_item = std::make_unique<CVhalNetSequenceItem>(static_cast<uint32_t>(sequence_id));
		if (nullptr != p_sequence_item)
		{
			/* コマンドアイテム生成 */
			const int32_t ret{CreateCommandItem(sequence_id, p_sequence_item.get())};
			if (VHAL_SUCCESS != ret)
			{
				p_sequence_item = nullptr;
				VHAL_LOGE("CreateCommandItem ret=%d, sequence_id=%d", ret, sequence_id);
			}
		}
		else
		{
			VHAL_LOGE("Failed to create CVhalNetSequenceItem. sequence_id=%d", sequence_id);
		}
	} while (false);

	VHAL_LOGV_OUT("p_sequence_item=%p", p_sequence_item.get());
	return p_sequence_item;
}

/*****************************************************************************
 処理概要：	コマンドアイテム生成
 引数    ：	const enum MostSequenceType sequence_id			(i)シーケンスID
          	CVhalNetSequenceItem* p_sequence_item			(o)シーケンスアイテムポインタ
 戻り値  ：	処理結果
           		VHAL_ERR_****			異常終了
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalMiconMostVideoInfo::CreateCommandItem(const enum MostSequenceType sequence_id, CVhalNetSequenceItem* const p_sequence_item) noexcept
{
	/* シーケンスID毎のコマンド情報 */
	using most_command_item_map = std::map<enum MostSequenceType, std::vector<uint32_t>>;
	const most_command_item_map command_item_table_
	{
		/* シーケンスID,									{コマンドID, ..}  */
		/* 前席映像パス切替設定 */
		{MostSequenceType::kSetFrontPath,					{MostCmdType::kFrontAvMasterCurrentVideoSource } },
		/* 後席専用映像反映通知 */
		{MostSequenceType::kSetRsePathNotify,				{MostCmdType::kPrevSourceActivity, MostCmdType::kNextSourceActivity, MostCmdType::kRearAvMasterCurrentVideoSource, MostCmdType::kRearDisplayVideoMode, MostCmdType::kStatusVpathRearNotification } },
		/* +B起動 */
		{MostSequenceType::kColdBoot,						{MostCmdType::kRearAvMasterCurrentVideoSource, MostCmdType::kRearDisplayVideoMode } },
		/* RSE脱落復帰 */
		{MostSequenceType::kRecoveryRse,					{MostCmdType::kNextSourceActivity, MostCmdType::kRearAvMasterCurrentVideoSource, MostCmdType::kRearDisplayVideoMode } },
		/* 後席ディスプレイ全体MUTE設定 */
		{MostSequenceType::kSetMuteRearDisp,				{MostCmdType::kRearDisplayVideoMute, MostCmdType::kStatusMuteRearDisp } },
	};

	VHAL_LOGV_IN("sequence_id=%d, p_sequence_item=%p", sequence_id, p_sequence_item);
	int32_t	ret{VHAL_SUCCESS};

	do
	{
		if (nullptr != p_sequence_item)
		{
			const auto iter = command_item_table_.find(sequence_id);
			if (iter != command_item_table_.end())
			{
				/* コマンドアイテム追加 */
				for (const auto iter_id: iter->second)
				{
					/* コマンド毎にデータ設定 */
					std::unique_ptr<CVhalNetCommandItem> p_command_item{nullptr};
					switch (iter_id)
					{
						case MostCmdType::kPrevSourceActivity:
							/* 後席専用映像パスが変化 かつ (前回)後席専用映像パスの設定ありの場合はSourceActivityを送信 */
							if ((0 != prev_video_rear_rse_disp_path_.compare(current_video_rear_rse_disp_path_)) && (0 != prev_video_rear_rse_disp_path_.compare("")))
							{
								p_command_item = std::make_unique<CVhalMiconMostCmdItemSourceActivity>(
													iter_id, prev_video_rear_rse_disp_path_);
							}
							else
							{
								/* SourceActivity送信なし（次データを確認） */
								continue;
							}
							break;
						case MostCmdType::kNextSourceActivity:
							if (MostSequenceType::kRecoveryRse == sequence_id)
							{
								/* RSE脱落復帰 */
								/* (現在)後席専用映像パスの設定ありの場合はSourceActivityを送信 */
								if (0 != current_video_rear_rse_disp_path_.compare(""))
								{
									p_command_item = std::make_unique<CVhalMiconMostCmdItemSourceActivity>(
														iter_id, current_video_rear_rse_disp_path_);
								}
								else
								{
									/* SourceActivity送信なし（次データを確認） */
									continue;
								}
							}
							else
							{
								/* RSE脱落復帰以外(後席専用映像反映通知) */
								/* 後席専用映像パスが変化 かつ (現在)後席専用映像パスの設定ありの場合はSourceActivityを送信 */
								if ((0 != prev_video_rear_rse_disp_path_.compare(current_video_rear_rse_disp_path_)) && (0 != current_video_rear_rse_disp_path_.compare("")))
								{
									p_command_item = std::make_unique<CVhalMiconMostCmdItemSourceActivity>(
														iter_id, current_video_rear_rse_disp_path_);
								}
								else
								{
									/* SourceActivity送信なし（次データを確認） */
									continue;
								}
							}								
							break;
						case MostCmdType::kFrontAvMasterCurrentVideoSource:
							p_command_item = std::make_unique<CVhalMiconMostCmdItemCurrentVideoSource>(
												iter_id, video_front_path_, "");
							break;							
						case MostCmdType::kRearAvMasterCurrentVideoSource:
							p_command_item = std::make_unique<CVhalMiconMostCmdItemCurrentVideoSource>(
												iter_id, current_video_rear_path_, current_video_rear_rse_disp_path_);
							break;
						case MostCmdType::kRearDisplayVideoMode:
							p_command_item = std::make_unique<CVhalMiconMostCmdItemVideoMode>(
												iter_id, current_video_rear_path_, current_video_rear_rse_disp_path_, rear_screen_w_, rear_screen_h_);
							break;
						case MostCmdType::kStatusVpathRearNotification:
							p_command_item = std::make_unique<CVhalMiconMostCmdItemStatusVpathRearNotification>(
												iter_id);
							break;
						case MostCmdType::kRearDisplayVideoMute:
							p_command_item = std::make_unique<CVhalMiconMostCmdItemVideoMute>(
												iter_id, mute_rear_disp_);
							break;
						case MostCmdType::kStatusMuteRearDisp:
							p_command_item = std::make_unique<CVhalMiconMostCmdItemStatusMuteRearDisp>(
												iter_id, mute_rear_disp_);
							break;
						default:
							/* 処理なし */
							break;
					}
					if (nullptr != p_command_item)
					{
						p_sequence_item->QueueCommand(std::move(p_command_item));
					}
					else
					{
						VHAL_LOGE("Failed to create CVhalNetCommandItem. command_id=%d", iter_id);
						ret = VHAL_ERR_MEMORY;
						break;
					}
				}
			}
			else
			{
				VHAL_LOGW("Unknown sequence_id=%d", sequence_id);
				ret = VHAL_ERR_PARAM;
			}
		}
		else
		{
			VHAL_LOGE("Invalid params. p_sequence_item is null");
			ret = VHAL_ERR_PARAM;
		}
	} while (false);

	VHAL_LOGV_OUT("ret=%d", ret);
	return ret;
}

/*****************************************************************************
 処理概要：	コマンド送信（未応答リトライあり）
 引数    ：	CVhalNetCommandItem * const p_command_item		(i)コマンドアイテム
 戻り値  ：	処理結果
           		VHAL_ERR_****			異常終了
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalMiconMostVideoInfo::SendCmdInterval(CVhalNetCommandItem * const p_command_item) noexcept
{
	VHAL_LOGV_IN("");
	int32_t	ret{VHAL_SUCCESS};

	do
	{
		/* コマンドアイテム設定 */
		p_most_send_item_videoinfo_->SetCommandItem(p_command_item);

		/* 映像情報通知 送信（未応答リトライ３回、200ms間隔） */
		constexpr uint32_t kMostCtrlApiCmdIntervalMax{200U};	/* MOSTコマンドリトライ時間(ms) */
		constexpr uint32_t kMostCtrlApiCmdSendCountMax{3U};		/* MOSTコマンドリトライ回数 */
		ret = p_micon_ctrl_->SendIntervalStart(p_most_send_item_videoinfo_.get(), kMostCtrlApiCmdIntervalMax, kMostCtrlApiCmdSendCountMax);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("SendIntervalStart error. ret=%d", ret);
		}
	} while (false);

	VHAL_LOGV_OUT("ret=%d", ret);
	return ret;
}

/*****************************************************************************
 処理概要：	コマンド送信（未応答リトライなし）
 引数    ：	CVhalNetCommandItem * const p_command_item		(i)コマンドアイテム
 戻り値  ：	処理結果
           		VHAL_ERR_****			異常終了
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalMiconMostVideoInfo::SendCmdOneshot(CVhalNetCommandItem * const p_command_item) noexcept
{
	VHAL_LOGV_IN("");
	int32_t	ret{VHAL_SUCCESS};

	do
	{
		/* コマンドアイテム設定 */
		p_most_send_item_videoinfo_->SetCommandItem(p_command_item);

		/* 映像情報通知 送信（未応答リトライなし） */
		ret = p_micon_ctrl_->Send(*p_most_send_item_videoinfo_);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("Send error. ret=%d", ret);
		}
	} while (false);

	VHAL_LOGV_OUT("ret=%d", ret);
	return ret;
}

/*****************************************************************************
 処理概要：	コマンド送信（メイン）
 引数    ：	const enum MostUpdateType update_type	(i)Updateタイプ
 戻り値  ：	処理結果
           		VHAL_ERR_****			異常終了
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalMiconMostVideoInfo::SendCmd(const enum MostUpdateType update_type) noexcept
{
	VHAL_LOGV_IN("");
	int32_t	ret{VHAL_SUCCESS};

	do
	{
		bool loop_end{false};

		/* カレントシーケンスアイテムが空になるまで */
		VHAL_LOGD("p_current_sequence_item_=%p", p_current_sequence_item_.get());
		if (nullptr == p_current_sequence_item_)
		{
			/* カレントシーケンスアイテムがない場合は、処理アイテムを取得 */
			/* （シーケンスアイテムリストの先頭アイテム） */
			p_current_sequence_item_ = p_net_sequence_manager_->DequeSequence();
		}

		if (nullptr == p_current_sequence_item_)
		{
			/* 処理アイテムなし（送信処理完了） */
			VHAL_LOGD("None send item");
			loop_end = true;
		}
		else
		{
			VHAL_LOGD("sequence_id=%d", p_current_sequence_item_->GetSequenceId());
			/* カレントコマンドアイテムを取得し、コマンドを送信 */
			/* （コマンドアイテムリストの先頭アイテム） */
			std::unique_ptr<CVhalNetCommandItem> p_command_item{p_current_sequence_item_->DequeCommand().release()};
			if (nullptr != p_command_item)
			{
				/* 処理アイテムあり */
				const uint32_t command_id{p_command_item->GetOpc()};
				VHAL_LOGD("command_id=%d", command_id);
				if (MostCmdType::kStatusType == (MostCmdType::kStatusType & command_id))
				{
					/* 切替結果通知（成功） */
					p_most_video_info_receiver_->ReceiveEvent(VHAL_VPATH_STS_SUCCESS, p_command_item.get(), update_type);
				}
				else
				{
					/* コマンド送信 */
					/* 応答コマンドあり/なし別 */
					if (true == p_command_item->GetResponseType())
					{
						/* 応答コマンドあり */
						ret = SendCmdInterval(p_command_item.get());
						if (VHAL_SUCCESS != ret)
						{
							VHAL_LOGE("SendCmdInterval error. ret=%d", ret);
							/* カレントシーケンスアイテムをクリア */
							const std::unique_ptr<CVhalNetCommandItem> p_ret_item{ClearCurrentSequenceItem()};
							if (nullptr != p_ret_item)
							{
								/* 切替結果通知（失敗） */
								p_most_video_info_receiver_->ReceiveEvent(VHAL_VPATH_STS_FAILED, p_ret_item.get(), update_type);
							}
						}
						else
						{
							(void)p_command_item.release();
							VHAL_LOGD("Wait command response");
						}
						loop_end = true;
					}
					else
					{
						/* 応答コマンドなし */
						ret = SendCmdOneshot(p_command_item.get());
						if (VHAL_SUCCESS != ret)
						{
							VHAL_LOGE("SendCmdOneshot error. ret=%d", ret);
							/* カレントシーケンスアイテムをクリア */
							const std::unique_ptr<CVhalNetCommandItem> p_ret_item{ClearCurrentSequenceItem()};
							if (nullptr != p_ret_item)
							{
								/* 切替結果通知（失敗） */
								p_most_video_info_receiver_->ReceiveEvent(VHAL_VPATH_STS_FAILED, p_ret_item.get(), update_type);
							}
						}
						else
						{
							(void)p_command_item.release();
						}
					}
				}
			}
			else
			{
				/* 処理アイテムなし */
				VHAL_LOGD("None command item");
				/* 次シーケンスアイテムへ */
				/* カレントシーケンスアイテムをクリア */
				(void)ClearCurrentSequenceItem();
			}
		}

		if (true == loop_end)
		{
			break;
		}
	} while (true);

	VHAL_LOGV_OUT("ret=%d", ret);
	return ret;
}

/*****************************************************************************
 処理概要：	カレントシーケンスアイテムクリア
 引数    ：	なし
 戻り値  ：	切替結果通知要否（最終コマンドアイテムポインタ）
            !nullptr	切替結果通知必要
            nullptr		切替結果通知不要
*****************************************************************************/
std::unique_ptr<CVhalNetCommandItem> CVhalMiconMostVideoInfo::ClearCurrentSequenceItem(void) noexcept
{
	std::unique_ptr<CVhalNetCommandItem> p_ret_item{nullptr};

	/* カレントシーケンスアイテムをクリア */
	VHAL_LOGV_IN("p_current_sequence_item_=%p", p_current_sequence_item_.get());
	if (nullptr != p_current_sequence_item_)
	{
		std::unique_ptr<CVhalNetCommandItem> p_command_item{nullptr};
		do
		{
			/* カレントコマンドアイテムを取得/削除 */
			p_command_item = p_current_sequence_item_->DequeCommand();
			if (nullptr != p_command_item)
			{
				/* 処理アイテムあり */
				const uint32_t command_id{p_command_item->GetOpc()};
				VHAL_LOGD("command_id=%d", command_id);
				if (MostCmdType::kStatusType == (MostCmdType::kStatusType & command_id))
				{
					/* コマンドアイテム（切替結果通知）を設定 */
					p_ret_item = std::move(p_command_item);
					/* break不要。p_command_itemが空になるまで。 */
				}
			}
		} while (nullptr != p_command_item);
		p_current_sequence_item_ = nullptr;
	}

	VHAL_LOGV_OUT("");
	return p_ret_item;
}

/*****************************************************************************
 処理概要：	送信データ構築
 引数    ：	std::vector<uint8_t> &send_data		(o)送信データ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalMiconMostSendItemVideoInfo::Build(std::vector<uint8_t> &send_data) const noexcept
{
	/* 送信データ構築 */
	send_data.clear();

	/* コマンド毎にデータ設定 */
	switch (p_command_item_->GetOpc())
	{
		case MostCmdType::kPrevSourceActivity:
		case MostCmdType::kNextSourceActivity:
			{
				const CVhalMiconMostCmdItemSourceActivity* const p_item{static_cast<CVhalMiconMostCmdItemSourceActivity*>(p_command_item_)};
				if (nullptr != p_item)
				{
					p_item->Build(send_data);
				}
			}
			break;

		case MostCmdType::kRearAvMasterCurrentVideoSource:
		case MostCmdType::kFrontAvMasterCurrentVideoSource:
			{
				const CVhalMiconMostCmdItemCurrentVideoSource* const p_item{static_cast<CVhalMiconMostCmdItemCurrentVideoSource*>(p_command_item_)};
				if (nullptr != p_item)
				{
					p_item->Build(send_data);
				}
			}
			break;

		case MostCmdType::kRearDisplayVideoMode:
			{
				const CVhalMiconMostCmdItemVideoMode* const p_item{static_cast<CVhalMiconMostCmdItemVideoMode*>(p_command_item_)};
				if (nullptr != p_item)
				{
					p_item->Build(send_data);
				}
			}
			break;

		case MostCmdType::kStatusVpathRearNotification:
			{
				const CVhalMiconMostCmdItemStatusVpathRearNotification* const p_item{static_cast<CVhalMiconMostCmdItemStatusVpathRearNotification*>(p_command_item_)};
				if (nullptr != p_item)
				{
					p_item->Build(send_data);
				}
			}
			break;

		case MostCmdType::kRearDisplayVideoMute:
			{
				const CVhalMiconMostCmdItemVideoMute* const p_item{static_cast<CVhalMiconMostCmdItemVideoMute*>(p_command_item_)};
				if (nullptr != p_item)
				{
					p_item->Build(send_data);
				}
			}
			break;

		case MostCmdType::kStatusMuteRearDisp:
			{
				const CVhalMiconMostCmdItemStatusMuteRearDisp* const p_item{static_cast<CVhalMiconMostCmdItemStatusMuteRearDisp*>(p_command_item_)};
				if (nullptr != p_item)
				{
					p_item->Build(send_data);
				}
			}
			break;

		default:
			/* 処理なし */
			break;
	}

	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	送信アイテム種別取得
 引数    ：	なし
 戻り値  ：	送信アイテム種別
*****************************************************************************/
CVhalMiconSendItem::SendItemType CVhalMiconMostSendItemVideoInfo::GetItemType(void) const noexcept
{
	return CVhalMiconSendItem::SendItemType::SEND_ITEM_TYPE_MOST;
}

/*****************************************************************************
 処理概要：	コマンドアイテム設定
 引数    ：	CVhalNetCommandItem* const item		(i)コマンドアイテムポインタ
 戻り値  ：	なし
*****************************************************************************/
void CVhalMiconMostSendItemVideoInfo::SetCommandItem(CVhalNetCommandItem* const item) noexcept
{
	p_command_item_ = item;
}

/*****************************************************************************
 処理概要：	コンストラク
 引数    ：	CVhalMiconMostVideoInfo* const p_most_video_info		(i)MOST映像情報通知送信モジュールインスタンスポインタ
           	CVhalMiconCommControl* const p_micon_comm_control		(i)マイコン間通信制御モジュールインスタンスポインタ
           	CVhalMiconMostSendItemVideoInfo* const p_send_item		(i)送信データインスタンスポインタ
 戻り値  ：	なし
*****************************************************************************/
CVhalMiconMostVideoInfoReceiver::CVhalMiconMostVideoInfoReceiver(CVhalMiconMostVideoInfo* const p_most_video_info, CVhalMiconCommControl* const p_micon_comm_control, CVhalMiconMostSendItemVideoInfo* const p_send_item) noexcept
{
	VHAL_LOGV("CVhalMiconMostVideoInfoReceiver is created. this=%p", this);
	p_most_video_info_receiver_= this;
	p_most_videopath_ = p_most_video_info;
	p_micon_ctrl_ = p_micon_comm_control;
	p_most_video_info_send_item_ = p_send_item;
	p_most_video_info_listener_ = nullptr;
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalMiconMostVideoInfoReceiver::~CVhalMiconMostVideoInfoReceiver(void)
{
	VHAL_LOGV("CVhalMiconMostVideoInfoReceiver is deleted. this=%p", this);
	p_most_video_info_receiver_= nullptr;
	p_most_videopath_ = nullptr;
	p_micon_ctrl_ = nullptr;
	p_most_video_info_send_item_ = nullptr;

	ClearEventListener();
}

/*****************************************************************************
 処理概要：	MOSTコマンド通知受信処理（VideoHALイベントスレッドからのコール）
 引数    ：	const std::vector<uint8_t>& data	(i)	コマンド情報ポインタ
 戻り値  ：	なし
*****************************************************************************/
void CVhalMiconMostVideoInfoReceiver::Receive(const std::vector<uint8_t>& data)
{
	if ((0U < data.size()) && (MostRecvEvent::kRetryCountEnd >= data[0]))
	{
		const uint8_t recv_event{static_cast<uint8_t>(data[0])};
		VHAL_LOGI("recv_event=%d, data.size=%ld", recv_event, data.size());

		if (MostRecvEvent::kLs3Data == recv_event)
		{
			/* LS3CTL Callbackデータ */
			constexpr std::size_t event_size{sizeof(uint8_t)};
			if (sizeof(Ls3CtlApiDat) == (data.size() - event_size))
			{
				Ls3CtlApiDat recv_data{};
				(void)memcpy(static_cast<void*>(&recv_data), static_cast<const void*>(&data[event_size]), sizeof(Ls3CtlApiDat));
				ReceiveLs3Data(recv_data);
			}
		}
		else if ((MostRecvEvent::kResponsTimeout == recv_event) ||
				 (MostRecvEvent::kRetryCountEnd == recv_event))
		{
			int32_t ret{VHAL_SUCCESS};
			if (MostRecvEvent::kRetryCountEnd == recv_event)
			{
				/* コマンド送信リトライ終了（初回合わせて3回リトライ） */
				ret = p_micon_ctrl_->SendIntervalEnd(p_most_video_info_send_item_);
				if (VHAL_SUCCESS != ret)
				{
					VHAL_LOGE("SendIntervalEnd error. ret=%d", ret);
				}
			}

			/* カレントシーケンスアイテムをクリア */
			const std::unique_ptr<CVhalNetCommandItem> p_ret_item{p_most_videopath_->ClearCurrentSequenceItem()};
			if (nullptr != p_ret_item)
			{
				/* 切替結果通知（失敗） */
				ReceiveEvent(VHAL_VPATH_STS_FAILED, p_ret_item.get(), MostUpdateType::kNotifyCallbackRse);
			}

			/* 次コマンド送信 */
			ret = p_most_videopath_->SendCmd(MostUpdateType::kNotifyCallbackRse);
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("SendCmd error. ret=%d", ret);
			}
		}
		else
		{
			/* 処理なし */
		}
	}
}

/*****************************************************************************
 処理概要：	MOSTコマンド通知受信事前通知処理（通信モジュールスレッドからのコール）
 引数    ：	const std::vector<uint8_t>& data	(i)	コマンド情報ポインタ
 戻り値  ：	なし
*****************************************************************************/
void CVhalMiconMostVideoInfoReceiver::ReceivePreNotify(const std::vector<uint8_t>& data) noexcept
{
	/* 処理なし（Receiveで処理実施） */
}

/*****************************************************************************
 処理概要：	受信アイテム種別取得
 引数    ：	なし
 戻り値  ：	受信アイテム種別
           		RECEIVE_ITEM_TYPE_MOST
*****************************************************************************/
CVhalMiconReceiveItem::ReceiveItemType CVhalMiconMostVideoInfoReceiver::GetItemType(void) const noexcept
{
	return CVhalMiconReceiveItem::ReceiveItemType::RECEIVE_ITEM_TYPE_MOST;
}

/*****************************************************************************
 処理概要：	イベントリスナー登録
 引数    ：	CVhalFileObserverEventListenerBase* p_listener		(i)リスナーポインタ
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM			パラメータ不正
           		VHAL_SUCCESS			正常終了
****************************************************************************/
int32_t CVhalMiconMostVideoInfoReceiver::RegisterEventListener(CVhalMiconMostReceiveEventListenerBase* const p_listener)
{
	int32_t	result{VHAL_SUCCESS};

	if (nullptr == p_listener)
	{
		VHAL_LOGE("p_listener is null");
		result = VHAL_ERR_PARAM;
	}
	else
	{
		p_most_video_info_listener_ = p_listener;
	}

	return result;
}

/*****************************************************************************
 処理概要：	イベントリスナー解除
 引数    ：	なし
 戻り値  ：	なし
****************************************************************************/
void CVhalMiconMostVideoInfoReceiver::ClearEventListener(void) noexcept
{
	p_most_video_info_listener_ = nullptr;
}

/*****************************************************************************
 処理概要：	LS3CTL Callbackデータ受信
 引数    ：	Ls3CtlApiDat &recv_data		(i)受信データ
 戻り値  ：	なし
*****************************************************************************/
void CVhalMiconMostVideoInfoReceiver::ReceiveLs3Data(const Ls3CtlApiDat &recv_data)
{
	/* コマンド送信リトライ終了（初回合わせて3回リトライ） */
	int32_t ret{p_micon_ctrl_->SendIntervalEnd(p_most_video_info_send_item_)};
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("SendIntervalEnd error. ret=%d", ret);
	}

	switch (recv_data.OPType)
	{
		case static_cast<uint8_t>(MostOPType::kResult):
			/* 次コマンド送信 */
			ret = p_most_videopath_->SendCmd(MostUpdateType::kNotifyCallbackRse);
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("SendCmd error. ret=%d", ret);
			}
			break;
		
		case static_cast<uint8_t>(MostOPType::kError):
			{
				VHAL_LOGE("recv_data.OPType=kError");
				/* カレントシーケンスアイテムをクリア */
				const std::unique_ptr<CVhalNetCommandItem> p_ret_item{p_most_videopath_->ClearCurrentSequenceItem()};
				if (nullptr != p_ret_item)
				{
					/* RSE映像パス切替結果通知（失敗） */
					ReceiveEvent(VHAL_VPATH_STS_FAILED, p_ret_item.get(), MostUpdateType::kNotifyCallbackRse);
				}
			}

			/* 次コマンド送信 */
			ret = p_most_videopath_->SendCmd(MostUpdateType::kNotifyCallbackRse);
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("SendCmd error. ret=%d", ret);
			}
			break;

		default:
			/* 処理なし */
			break;
	}
}

/*****************************************************************************
 処理概要：	LS3CTL 切替結果通知
 引数    ：	const uint32_t	result	(i)切替結果
            CVhalNetCommandItem* const p_command_item	(i)コマンドアイテムポインタ（切替結果通知あり）
            const enum MostUpdateType update_type		(i)Updateタイプ
 戻り値  ：	なし
*****************************************************************************/
void CVhalMiconMostVideoInfoReceiver::ReceiveEvent(const uint32_t result, CVhalNetCommandItem* const p_command_item, const enum MostUpdateType update_type) const noexcept
{
	/* 切替結果通知 */
	if (nullptr != p_command_item)
	{
		switch (p_command_item->GetOpc())
		{
			case MostCmdType::kStatusVpathRearNotification:		/* 切替結果通知あり（後席専用映像反映通知） */
				{
					/* 切替結果を設定 */
					if (nullptr != p_most_video_info_listener_)
					{
						p_most_video_info_listener_->NotifyReceiveMostRsePathResult(result, update_type);
					}
				}
				break;

			case MostCmdType::kStatusMuteRearDisp:				/* 切替結果通知あり（後席ディスプレイ全体MUTE） */
				{
					/* Mute設定を設定 */
					if (nullptr != p_most_video_info_listener_)
					{
						const CVhalMiconMostCmdItemStatusMuteRearDisp* const p_item{static_cast<CVhalMiconMostCmdItemStatusMuteRearDisp*>(p_command_item)};
						p_most_video_info_listener_->NotifyReceiveMostMuteRearDispResult(p_item->GetMute(), update_type);
					}
				}
				break;

			default:
				/* 処理なし */
				break;
		}
	}
}

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	const uint32_t command_id						(i)コマンドID
         ：	const std::string& rear_rse_disp_path			(i)後席専用映像パス
 戻り値  ：	なし
*****************************************************************************/
CVhalMiconMostCmdItemSourceActivity::CVhalMiconMostCmdItemSourceActivity(const uint32_t command_id, const std::string& rear_rse_disp_path) noexcept
	:CVhalNetCommandItem(command_id)
	,video_rear_rse_disp_path_(rear_rse_disp_path)
{
	VHAL_LOGV("Item is created. this=%p, command_id=%d, rear_rse_disp_path=%s", this, command_id, rear_rse_disp_path.c_str());
}

/*****************************************************************************
 処理概要：	送信データ構築
 引数    ：	std::vector<uint8_t> &send_data		(o)送信データ
 戻り値  ：	なし
*****************************************************************************/
void CVhalMiconMostCmdItemSourceActivity::Build(std::vector<uint8_t> &send_data) const noexcept
{
	const uint32_t command_id{GetOpc()};
	const auto iter_cmd = cmd_head_table_.find(command_id);
	if (iter_cmd != cmd_head_table_.end())
	{
		Ls3CtlApiDat dat{};
		constexpr std::size_t data_size{sizeof(dat)};
		(void)memset(&dat, 0, data_size);

		dat.DeviceID	= iter_cmd->second.deviceid_;
		dat.FktID		= static_cast<uint16_t>(iter_cmd->second.fktid_);
		dat.OPType		= static_cast<uint8_t>(iter_cmd->second.optype_);
		dat.DataType	= iter_cmd->second.datatype_;
		dat.Length		= static_cast<uint16_t>(iter_cmd->second.length_);

		/* 後席専用映像パス情報を設定 */
		const auto iter = most_rear_path_table_.find(video_rear_rse_disp_path_);
		if (iter != most_rear_path_table_.end())
		{
			dat.FBlockID		   = static_cast<uint8_t>(iter->second.fblockid_);
			dat.InstID			   = static_cast<uint8_t>(iter->second.instid_);
			dat.Data[kPosSourceNr] = I32ToUI8(UI32ToI32(iter->second.sourcenr_));
			if (MostCmdType::kPrevSourceActivity == command_id)
			{
				dat.Data[kPosActivityType] = kActivityTypeOFF;
			}
			else
			{
				dat.Data[kPosActivityType] = kActivityTypeON;
			}
		}

		/* 送信データ生成 */
		send_data.resize(data_size);
		(void)memcpy(static_cast<void*>(send_data.data()), static_cast<void*>(&dat), data_size);

		/* 送信データログ */
		VHAL_LOGI("most SourceActivity dev=0x%04x fbl=0x%02x ins=0x%02x fkt=0x%04x opt=0x%02x dat=%d len=%d Data:srcnr=0x%02x Act=%d"
				, dat.DeviceID, dat.FBlockID, dat.InstID, dat.FktID, dat.OPType, dat.DataType, dat.Length
				, dat.Data[kPosSourceNr], dat.Data[kPosActivityType]);
	}
}

/*****************************************************************************
 処理概要：	応答コマンドあり/なし種別取得
 引数    ：	なし
 戻り値  ：	true：応答コマンドあり
*****************************************************************************/
bool CVhalMiconMostCmdItemSourceActivity::GetResponseType(void) const noexcept
{
	return true;
}

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	const uint32_t command_id						(i)コマンドID
         ：	const std::string& rear_path					(i)前席or後席映像パス
         ：	const std::string& rear_rse_disp_path			(i)後席専用映像パス（後席の場合のみ）
 戻り値  ：	なし
*****************************************************************************/
CVhalMiconMostCmdItemCurrentVideoSource::CVhalMiconMostCmdItemCurrentVideoSource(const uint32_t command_id, const std::string& path, const std::string& rear_rse_disp_path) noexcept
	:CVhalNetCommandItem(command_id)
	,video_path_(path)
	,video_rear_rse_disp_path_(rear_rse_disp_path)
{
	VHAL_LOGV("Item is created. this=%p, command_id=%d, path=%s, rear_rse_disp_path=%s", this, command_id, path.c_str(), rear_rse_disp_path.c_str());
}

/*****************************************************************************
 処理概要：	送信データ構築
 引数    ：	std::vector<uint8_t> &send_data		(o)送信データ
 戻り値  ：	なし
*****************************************************************************/
void CVhalMiconMostCmdItemCurrentVideoSource::Build(std::vector<uint8_t> &send_data) const noexcept
{
	/* 前席系映像パス情報変換用 */
	const most_video_path_map most_front_path_table_
	{
		/* 映像パス名,				{FblockID,									InstID,											SourceNr}  */
		{"",						{MostFBlockId::kSourceActivityVdsp,			MostInstID::kSourceActivityVdsp,			VHAL_VSRC_ID_OTHER	} },
		{"MEDIAPLAYER",				{MostFBlockId::kSourceActivityVdsp,			MostInstID::kSourceActivityVdsp,			VHAL_VSRC_ID_OTHER	} },
		{"CARPLAY",					{MostFBlockId::kSourceActivityApp,			MostInstID::kSourceActivityApp,				VHAL_VSRC_ID_OTHER	} },
		{"ANDROIDAUTO",				{MostFBlockId::kSourceActivityApp,			MostInstID::kSourceActivityApp,				VHAL_VSRC_ID_OTHER	} },
		{"DTV",						{MostFBlockId::kSourceActivityDtv,			MostInstID::kSourceActivityHdmi,			VHAL_VSRC_ID_DTV	} },
		{"HDMI",					{MostFBlockId::kSourceActivityHdmi,			MostInstID::kSourceActivityHdmi,			VHAL_VSRC_ID_HDMI	} },
		{"CAMERA-IMG-ADJ",			{MostFBlockId::kSourceActivityVdsp,			MostInstID::kSourceActivityVdsp,			VHAL_VSRC_ID_IMAGE_ADJUST_CAM	} },
		{"DRC",						{MostFBlockId::kSourceActivityHdmi,			MostInstID::kSourceActivityDrc,				VHAL_VSRC_ID_IMAGE_ADJUST_CAM	} },
		{"MULTISENSORY",			{MostFBlockId::kSourceActivityMultiSensory,	MostInstID::kSourceActivityMultiSensory,	VHAL_VSRC_ID_OTHER	} },
		{"FRAGRANCE",				{MostFBlockId::kSourceActivityVdsp,			MostInstID::kSourceActivityVdsp,			VHAL_VSRC_ID_OTHER	} },
	};

	const uint32_t command_id{GetOpc()};
	const auto iter_cmd = cmd_head_table_.find(command_id);
	if (iter_cmd != cmd_head_table_.end())
	{
		Ls3CtlApiDat dat{};
		constexpr std::size_t data_size{sizeof(dat)};
		(void)memset(&dat, 0, data_size);

		dat.DeviceID	= iter_cmd->second.deviceid_;
		dat.FBlockID	= static_cast<uint8_t>(iter_cmd->second.fblockid_);
		dat.InstID		= static_cast<uint8_t>(iter_cmd->second.instid_);
		dat.FktID		= static_cast<uint16_t>(iter_cmd->second.fktid_);
		dat.OPType		= static_cast<uint8_t>(iter_cmd->second.optype_);
		dat.DataType	= iter_cmd->second.datatype_;
		dat.Length		= static_cast<uint16_t>(iter_cmd->second.length_);

		if (MostCmdType::kFrontAvMasterCurrentVideoSource == command_id)
		{
			/* 前席 */
			if (0 != video_path_.compare(""))
			{
				const auto iter = most_front_path_table_.find(video_path_);
				if (iter != most_front_path_table_.end())
				{
					dat.Data[kPosFblock]	= static_cast<uint8_t>(iter->second.fblockid_);
					dat.Data[kPosInstID]	= static_cast<uint8_t>(iter->second.instid_);
					dat.Data[kPosSourceNr]	= I32ToUI8(UI32ToI32(iter->second.sourcenr_));
				}
			}
			else
			{
				/* 処理なし */
				/* VideoPowerOFF（映像パスクリア）の時は全て0x00をセット */
			}
		}
		else
		{
			/* 後席専用映像パス設定を優先 */
			std::string	rear_path{video_rear_rse_disp_path_};
			if (0 == rear_path.compare(""))
			{
				rear_path = video_path_;
			}

			if (0 != rear_path.compare(""))
			{
				const auto iter = most_rear_path_table_.find(rear_path);
				if (iter != most_rear_path_table_.end())
				{
					dat.Data[kPosFblock]	= static_cast<uint8_t>(iter->second.fblockid_);
					dat.Data[kPosInstID]	= static_cast<uint8_t>(iter->second.instid_);
					dat.Data[kPosSourceNr]	= I32ToUI8(UI32ToI32(iter->second.sourcenr_));
				}
			}
			else
			{
				/* 処理なし */
				/* VideoPowerOFF（映像パスクリア）の時は全て0x00をセット */
			}
		}

		/* 送信データ生成 */
		send_data.resize(data_size);
		(void)memcpy(static_cast<void*>(send_data.data()), static_cast<void*>(&dat), data_size);

		/* 送信データログ */
		VHAL_LOGI("most CurrentVideoSource dev=0x%04x fbl=0x%02x ins=0x%02x fkt=0x%04x opt=0x%02x dat=%d len=%d Data:fbl=0x%x ins=0x%x srcnr=0x%02x"
				, dat.DeviceID, dat.FBlockID, dat.InstID, dat.FktID, dat.OPType, dat.DataType, dat.Length
				, dat.Data[kPosFblock], dat.Data[kPosInstID], dat.Data[kPosSourceNr]);
	}
}

/*****************************************************************************
 処理概要：	応答コマンドあり/なし種別取得
 引数    ：	なし
 戻り値  ：	false：応答コマンドなし
*****************************************************************************/
bool CVhalMiconMostCmdItemCurrentVideoSource::GetResponseType(void) const noexcept
{
	return false;
}

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	const uint32_t command_id						(i)コマンドID
         ：	const std::string& rear_path					(i)後席映像パス
         ：	const std::string& rear_rse_disp_path			(i)後席専用映像パス
         ：	const int32_t w									(i)幅
         ：	const int32_t h									(i)高さ
 戻り値  ：	なし
*****************************************************************************/
CVhalMiconMostCmdItemVideoMode::CVhalMiconMostCmdItemVideoMode(const uint32_t command_id, const std::string& rear_path, const std::string& rear_rse_disp_path, const int32_t w, const int32_t h) noexcept
	:CVhalNetCommandItem(command_id)
	,video_rear_path_(rear_path)
	,video_rear_rse_disp_path_(rear_rse_disp_path)
	,rear_screen_w_(w)
	,rear_screen_h_(h)
{
	VHAL_LOGV("Item is created. this=%p, command_id=%d, rear_path=%s, rear_rse_disp_path=%s, rear_size=%dx%d", this, command_id, rear_path.c_str(), rear_rse_disp_path.c_str(), w, h);
}

/*****************************************************************************
 処理概要：	送信データ構築
 引数    ：	std::vector<uint8_t> &send_data		(o)送信データ
 戻り値  ：	なし
*****************************************************************************/
void CVhalMiconMostCmdItemVideoMode::Build(std::vector<uint8_t> &send_data) const noexcept
{
	const uint32_t command_id{GetOpc()};
	const auto iter_cmd = cmd_head_table_.find(command_id);
	if (iter_cmd != cmd_head_table_.end())
	{
		Ls3CtlApiDat dat{};
		constexpr std::size_t data_size{sizeof(dat)};
		(void)memset(&dat, 0, data_size);

		dat.DeviceID	= iter_cmd->second.deviceid_;
		dat.FBlockID	= static_cast<uint8_t>(iter_cmd->second.fblockid_);
		dat.InstID		= static_cast<uint8_t>(iter_cmd->second.instid_);
		dat.FktID		= static_cast<uint16_t>(iter_cmd->second.fktid_);
		dat.OPType		= static_cast<uint8_t>(iter_cmd->second.optype_);
		dat.DataType	= iter_cmd->second.datatype_;
		dat.Length		= static_cast<uint16_t>(iter_cmd->second.length_);

		/* 後席専用映像パス設定を優先 */
		std::string	rear_path{video_rear_rse_disp_path_};
		if (0 == rear_path.compare(""))
		{
			rear_path = video_rear_path_;
		}

		if (0 != rear_path.compare(""))
		{
			const auto iter = most_rear_path_table_.find(rear_path);
			if (iter != most_rear_path_table_.end())
			{
				dat.Data[kPosFblock] = static_cast<uint8_t>(iter->second.fblockid_);
				dat.Data[kPosInstID] = static_cast<uint8_t>(iter->second.instid_);
				dat.Data[kPosCompositeFblock] = 0xFFU;
				dat.Data[kPosCompositeInstID] = 0xFFU;
				/* [4 ～ 11]は0固定 */
				dat.Data[kPosScreenWHigh]	= static_cast<uint8_t>((I32ToUI32(rear_screen_w_) & 0xFF00U) >> static_cast<uint8_t>(UINT8_WIDTH));
				dat.Data[kPosScreenWLow]	= static_cast<uint8_t>(I32ToUI32(rear_screen_w_) & 0xFFU);
				dat.Data[kPosScreenHHigh]	= static_cast<uint8_t>((I32ToUI32(rear_screen_h_) & 0xFF00U) >> static_cast<uint8_t>(UINT8_WIDTH));
				dat.Data[kPosScreenHLow]	= static_cast<uint8_t>(I32ToUI32(rear_screen_h_) & 0xFFU);
			}
		}
		else
		{
			/* VideoPowerOFF（映像パスクリア）の時は固定値をセット */
			dat.Data[kPosFblock] = 0xFFU;
			dat.Data[kPosInstID] = 0xFFU;
			dat.Data[kPosCompositeFblock] = 0xFFU;
			dat.Data[kPosCompositeInstID] = 0xFFU;
			/* [4 ～ 11]は0固定 */
			/* [12 ～ 15]は0固定 */
		}

		/* 送信データ生成 */
		send_data.resize(data_size);
		(void)memcpy(static_cast<void*>(send_data.data()), static_cast<void*>(&dat), data_size);

		/* 送信データログ */
		VHAL_LOGI("most VideoMode dev=0x%04x fbl=0x%02x ins=0x%02x fkt=0x%04x opt=0x%02x dat=%d len=%d Data:fbl=0x%x ins=0x%x cfbl=0x%x cins=0x%x screen=0x%x%x*0x%x%x"
				, dat.DeviceID, dat.FBlockID, dat.InstID, dat.FktID, dat.OPType, dat.DataType, dat.Length
				, dat.Data[kPosFblock], dat.Data[kPosInstID], dat.Data[kPosCompositeFblock], dat.Data[kPosCompositeInstID]
				, dat.Data[kPosScreenWHigh], dat.Data[kPosScreenWLow], dat.Data[kPosScreenHHigh], dat.Data[kPosScreenHLow]);
	}
}

/*****************************************************************************
 処理概要：	応答コマンドあり/なし種別取得
 引数    ：	なし
 戻り値  ：	true：応答コマンドあり
*****************************************************************************/
bool CVhalMiconMostCmdItemVideoMode::GetResponseType(void) const noexcept
{
	return true;
}

/*****************************************************************************
 処理概要：	送信データ構築
 引数    ：	std::vector<uint8_t> &send_data		(o)送信データ
 戻り値  ：	なし
*****************************************************************************/
void CVhalMiconMostCmdItemStatusVpathRearNotification::Build(std::vector<uint8_t> &send_data) const noexcept
{
	/* 処理なし */
}

/*****************************************************************************
 処理概要：	応答コマンドあり/なし種別取得
 引数    ：	なし
 戻り値  ：	false：応答コマンドなし
*****************************************************************************/
bool CVhalMiconMostCmdItemStatusVpathRearNotification::GetResponseType(void) const noexcept
{
	return false;
}

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	const uint32_t command_id						(i)コマンドID
         ：	const bool mute									(i)Mute設定
 戻り値  ：	なし
*****************************************************************************/
CVhalMiconMostCmdItemVideoMute::CVhalMiconMostCmdItemVideoMute(const uint32_t command_id, const bool mute) noexcept
	:CVhalNetCommandItem(command_id)
	,mute_rear_disp_(mute)
{
	VHAL_LOGV("Item is created. this=%p, command_id=%d, mute=%d", this, command_id, mute);
}

/*****************************************************************************
 処理概要：	送信データ構築
 引数    ：	std::vector<uint8_t> &send_data		(o)送信データ
 戻り値  ：	なし
*****************************************************************************/
void CVhalMiconMostCmdItemVideoMute::Build(std::vector<uint8_t> &send_data) const noexcept
{
	const uint32_t command_id{GetOpc()};
	const auto iter_cmd = cmd_head_table_.find(command_id);
	if (iter_cmd != cmd_head_table_.end())
	{
		Ls3CtlApiDat dat{};
		constexpr std::size_t data_size{sizeof(dat)};
		(void)memset(&dat, 0, data_size);

		dat.DeviceID	= iter_cmd->second.deviceid_;
		dat.FBlockID	= static_cast<uint8_t>(iter_cmd->second.fblockid_);
		dat.InstID		= static_cast<uint8_t>(iter_cmd->second.instid_);
		dat.FktID		= static_cast<uint16_t>(iter_cmd->second.fktid_);
		dat.OPType		= static_cast<uint8_t>(iter_cmd->second.optype_);
		dat.DataType	= iter_cmd->second.datatype_;
		dat.Length		= static_cast<uint16_t>(iter_cmd->second.length_);

		/* Mute設定 */
		dat.Data[kPosMute] = BToUI8(mute_rear_disp_);

		/* 送信データ生成 */
		send_data.resize(data_size);
		(void)memcpy(static_cast<void*>(send_data.data()), static_cast<void*>(&dat), data_size);

		/* 送信データログ */
		VHAL_LOGI("most VideoMute dev=0x%04x fbl=0x%02x ins=0x%02x fkt=0x%04x opt=0x%02x dat=%d len=%d Data:mute=%d"
				, dat.DeviceID, dat.FBlockID, dat.InstID, dat.FktID, dat.OPType, dat.DataType, dat.Length
				, dat.Data[kPosMute]);
	}
}

/*****************************************************************************
 処理概要：	応答コマンドあり/なし種別取得
 引数    ：	なし
 戻り値  ：	false：応答コマンドなし
*****************************************************************************/
bool CVhalMiconMostCmdItemVideoMute::GetResponseType(void) const noexcept
{
	return false;
}

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	const uint32_t command_id						(i)コマンドID
         ：	const bool mute									(i)Mute設定
 戻り値  ：	なし
*****************************************************************************/
CVhalMiconMostCmdItemStatusMuteRearDisp::CVhalMiconMostCmdItemStatusMuteRearDisp(const uint32_t command_id, const bool mute) noexcept
	:CVhalNetCommandItem(command_id)
	,mute_rear_disp_(mute)
{
	VHAL_LOGV("Item is created. this=%p, command_id=%d, mute=%d", this, command_id, mute);
}

/*****************************************************************************
 処理概要：	送信データ構築
 引数    ：	std::vector<uint8_t> &send_data		(o)送信データ
 戻り値  ：	なし
*****************************************************************************/
void CVhalMiconMostCmdItemStatusMuteRearDisp::Build(std::vector<uint8_t> &send_data) const noexcept
{
	/* 処理なし */
}

/*****************************************************************************
 処理概要：	応答コマンドあり/なし種別取得
 引数    ：	なし
 戻り値  ：	false：応答コマンドなし
*****************************************************************************/
bool CVhalMiconMostCmdItemStatusMuteRearDisp::GetResponseType(void) const noexcept
{
	return false;
}

/*****************************************************************************
 処理概要：	Mute取得
 引数    ：	なし
 戻り値  ：	Mute状態
*****************************************************************************/
bool CVhalMiconMostCmdItemStatusMuteRearDisp::GetMute(void) const noexcept
{
	return mute_rear_disp_;
}

} /* namespace videohal */
