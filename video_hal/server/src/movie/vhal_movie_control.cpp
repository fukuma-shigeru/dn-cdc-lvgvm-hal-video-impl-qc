/*******************************************************************************
    機能名称    ：  動画制御モジュール
    ファイル名称：  vhal_movie_control.cpp
*******************************************************************************/
#include "vhal_movie_control.h"

#include "vhal_define.h"
#include "vhal_log.h"
#include "vhal_round_cast.h"
#include "vhal_main_control.h"
#include "vhal_property_control.h"
#include "vhal_layout_mng.h"
#include "vhal_event_item_movie_wait.h"

namespace videohal
{

namespace
{

CVhalMovieControl	*p_movie_ctrl_{nullptr};

/*****************************************************************************
 処理概要：	vhal_gst_playerのCallback関数(complete)
 引数    ：	void	*user_data			(i)User private data
 戻り値  ：	なし
*****************************************************************************/
extern "C" void gst_movie_completed_event(const void * const user_data)
{
	(void)user_data;

	if (nullptr != p_movie_ctrl_)
	{
		p_movie_ctrl_->HandleGstMovieEvent(VHAL_MOVIE_STS_FINISHED);
	}
}

/*****************************************************************************
 処理概要：	vhal_gst_playerのCallback関数(error)
 引数    ：	void	*user_data			(i)User private data
 戻り値  ：	なし
*****************************************************************************/
extern "C" void gst_movie_error_event(const void * const user_data)
{
	(void)user_data;

	if (nullptr != p_movie_ctrl_)
	{
		p_movie_ctrl_->HandleGstMovieEvent(VHAL_MOVIE_STS_FAILED);
	}
}

}

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalMovieControl::CVhalMovieControl(void)
	:p_main_(nullptr)
	,p_event_route_(nullptr)
	,p_layout_mng_(nullptr)
	,p_prop_(nullptr)
	,play_status_(VHAL_MOVIE_STS_NONE)
	,is_playing_(false)
	,gst_movie_ctx_(nullptr)
{
 	VHAL_LOGV("CVhalMovieControl is created. this=%p", this);
	p_movie_ctrl_ = this;
	gst_movie_callback_.completion = &gst_movie_completed_event;
	gst_movie_callback_.error = &gst_movie_error_event;
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalMovieControl::~CVhalMovieControl(void)
{
	VHAL_LOGV("CVhalMovieControl is deleted. this=%p", this);
	Finalize();
}

/*****************************************************************************
 処理概要：	初期化処理（内部リソースの確保）
 引数    ：	CVhalMainControl		*p_main			(i)メインコントロールインスタンスポインタ
 引数    ：	CVhalPropertyControl	*p_prop			(i)プロパティ制御インスタンスポインタ
 引数    ：	CVhalLayoutManager		*p_layout_mng	(i)レイアウト制御インスタンスポインタ
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalMovieControl::Initialize(CVhalMainControl * const p_main, CVhalPropertyControl * const p_prop, CVhalLayoutManager * const p_layout_mng)
{
	if ((nullptr == p_main) || (nullptr == p_prop) || (nullptr == p_layout_mng))
	{
		return VHAL_ERR_PARAM;
	}

	p_main_       = p_main;
	p_prop_       = p_prop;
	p_layout_mng_ = p_layout_mng;

	p_event_route_ = std::make_unique<CVhalEventRoute>();
	int32_t ret{p_event_route_->Initialize()};
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("EventRoute Initialize error. ret=%d", ret);
		return ret;
	}
	ret = p_main->RegisterEventSource(p_event_route_.get());
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("RegisterEventSource error. ret=%d", ret);
		return ret;
	}

	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	終了処理（内部リソースの解放）
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalMovieControl::Finalize(void)
{
	if (nullptr != p_event_route_)
	{
		p_main_->ClearEventSource(p_event_route_.get());
		p_event_route_ = nullptr;
	}
}

/*****************************************************************************
 処理概要：	汎用動画再生機能 動画再生パラメータ設定
 引数    ：	uint32_t screen_width				(i)スクリーン幅
         ：	uint32_t screen_height				(i)スクリーン高さ
         ：	const struct MovieParameter& param	(i)再生動画パラメータ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
				VHAL_ERR_PARAM			パラメータエラー
*****************************************************************************/
int32_t CVhalMovieControl::PrepareFrontMovie(const uint32_t screen_width, const uint32_t screen_height, const struct MovieParameter& param)
{
	int32_t ret{VHAL_SUCCESS};

	VHAL_LOGV_IN();

	if (false == IsValidPlayStatus(play_status_, MovieRequestType::kPrepare))
	{
		ret = VHAL_ERR;
	}
	else
	{
		int32_t filepath_ret{VHAL_SUCCESS};
		int32_t param_ret{VHAL_SUCCESS};

		if(param.GetFilePath().empty())
		{
			filepath_ret = VHAL_ERR_PARAM;
			VHAL_LOGE("movie filepath empty. ret=%d" , filepath_ret);
		}
		else
		{
			VHAL_LOGI("movie prepare x=%d y=%d w=%d h=%d file=%s", param.GetX(), param.GetY(), param.GetWidth(), param.GetHeight(), param.GetFilePath().c_str());
		}

		if ( ( (screen_width <= param.GetX()) )
		  || ( (screen_height <= param.GetY()) )
		  || ( (0U >= param.GetWidth()) || (screen_width < param.GetWidth()) )
		  || ( (0U >= param.GetHeight()) || (screen_height < param.GetHeight()) ) )
		{
			param_ret = VHAL_ERR_PARAM;
			VHAL_LOGE("movie parameter error. ret=%d screen_width=%d screen_height=%d param(%dx%dx%dx%d)", param_ret, screen_width, screen_height, param.GetX(), param.GetY(), param.GetWidth(), param.GetHeight());
		}
		
		if ( (VHAL_SUCCESS == filepath_ret) && (VHAL_SUCCESS == param_ret) )
		{
			movie_parameter_ = param;
			play_status_ = VHAL_MOVIE_STS_READY;
		}
		else
		{
			ret = VHAL_ERR_PARAM;
		}
	}

	VHAL_LOGV_OUT();

	return ret;
}

/*****************************************************************************
 処理概要：	汎用動画再生機能 動画再生パラメータクリア
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
				VHAL_ERR				異常終了
*****************************************************************************/
int32_t CVhalMovieControl::ClearFrontMovie(void)
{
	int32_t ret{VHAL_SUCCESS};

	VHAL_LOGV_IN();

	VHAL_LOGI("movie parameter clear");

	if (false == IsValidPlayStatus(play_status_, MovieRequestType::kClear))
	{
		ret = VHAL_ERR;
	}
	else
	{
		if (nullptr != gst_movie_ctx_)
		{
			vhal_player_stop(gst_movie_ctx_);
			gst_movie_ctx_ = nullptr;
		}
		movie_parameter_.SetFilePath("");
		movie_parameter_.SetX(0U);
		movie_parameter_.SetY(0U);
		movie_parameter_.SetWidth(0U);
		movie_parameter_.SetHeight(0U);
		
		play_status_ = VHAL_MOVIE_STS_NONE;
		is_playing_ = false;
	}

	VHAL_LOGV_OUT();

	return ret;
}

/*****************************************************************************
 処理概要：	汎用動画再生機能 動画再生開始
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
				VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
*****************************************************************************/
int32_t CVhalMovieControl::StartFrontMovie(void)
{
	int32_t ret{VHAL_SUCCESS};

	VHAL_LOGV_IN();
	VHAL_LOGI("start movie");
	if (false == IsValidPlayStatus(play_status_, MovieRequestType::kStart))
	{
		ret = VHAL_ERR;
	}
	else
	{
		ret = p_layout_mng_->SetMovieFrontDestRectangle( UI32ToI32(movie_parameter_.GetX()), UI32ToI32(movie_parameter_.GetY()), UI32ToI32(movie_parameter_.GetWidth()), UI32ToI32(movie_parameter_.GetHeight()));
		if (VHAL_SUCCESS == ret)
		{
			if (true == is_playing_)
			{
				VHAL_LOGE("movie playing is not finished.");
				ret = VHAL_ERR;
				play_status_ = VHAL_MOVIE_STS_FAILED;
			}
			else
			{
				is_playing_ = true;
				play_status_ = VHAL_MOVIE_STS_PLAYING;
				gst_movie_ctx_ = vhal_player_start(movie_parameter_.GetFilePath().c_str(), &gst_movie_callback_, nullptr);
				if (nullptr == gst_movie_ctx_)
				{
					VHAL_LOGE("failed to start movie %s",movie_parameter_.GetFilePath().c_str());
					ret = VHAL_ERR;
					play_status_ = VHAL_MOVIE_STS_FAILED;
					is_playing_ = false;
				}
				else
				{
					VHAL_LOGI("movie start file=%s",movie_parameter_.GetFilePath().c_str());
				}
			}
		}
		else
		{
			VHAL_LOGE("movie start error ret = %d", ret);
		}
	}

	VHAL_LOGV_OUT();
	
	return ret;
}

/*****************************************************************************
 処理概要：	汎用動画再生機能 動画再生中止
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
				VHAL_ERR				異常終了
*****************************************************************************/
int32_t CVhalMovieControl::CancelFrontMovie(void)
{
	int32_t ret{VHAL_SUCCESS};

	VHAL_LOGV_IN();

	if (false == IsValidPlayStatus(play_status_, MovieRequestType::kCancel))
	{
		ret = VHAL_ERR;
	}
	else
	{
		play_status_ = VHAL_MOVIE_STS_CANCELED;
		if (nullptr != gst_movie_ctx_)
		{
			vhal_player_stop(gst_movie_ctx_);
			gst_movie_ctx_ = nullptr;
		}
		HandleGstMovieEvent(VHAL_MOVIE_STS_FAILED);
	}

	VHAL_LOGV_OUT();
	
	return ret;
}

/*****************************************************************************
 処理概要：	汎用動画再生状態取得
 引数    ：	なし
 戻り値  ：	汎用動画再生状態
	          VHAL_MOVIE_STS_NONE     機能不使用
	          VHAL_MOVIE_STS_READY    動画再生待機
	          VHAL_MOVIE_STS_PLAYING  動画再生中
	          VHAL_MOVIE_STS_FINISHED 動画再生完了
	          VHAL_MOVIE_STS_CANCELED 動画再生中止
	          VHAL_MOVIE_STS_FAILED   動画再生異常
*****************************************************************************/
int32_t CVhalMovieControl::GetFrontMovieStatus(void) const noexcept
{
	return play_status_;
}

/*****************************************************************************
 処理概要：	汎用動画再生完了時状態設定
 引数    ：	int32_t result	汎用動画再生状態
 戻り値  ：	なし
*****************************************************************************/
void CVhalMovieControl::NotifyMovieFinished(const int32_t result)
{
	VHAL_LOGI("result=%d play_status_=%d", result, play_status_);

	if (VHAL_MOVIE_STS_PLAYING == play_status_)
	{
		play_status_ = result;
		p_prop_->UpdateMovieStartResult(play_status_);
	}

	is_playing_ = false;
}

/*****************************************************************************
 処理概要：	動画再生状態チェック
 引数    ：	int32_t             play_status         (i)動画再生状態
         ：	MovieRequestType    request             (i)動画再生設定種別
 戻り値  ：	処理結果
           		true	正常
           		false	異常
*****************************************************************************/
bool CVhalMovieControl::IsValidPlayStatus(const int32_t play_status, const MovieRequestType request) const
		{
	/* 動画再生状態と設定種別の組み合わせ確認 */
	struct ErrorCombination {
		MovieRequestType request;
		int32_t play_status;
	};

	const static std::vector<ErrorCombination> error_combination_list{
		{MovieRequestType::kPrepare, VHAL_MOVIE_STS_READY},
		{MovieRequestType::kPrepare, VHAL_MOVIE_STS_PLAYING},
		{MovieRequestType::kPrepare, VHAL_MOVIE_STS_FINISHED}, 
		{MovieRequestType::kPrepare, VHAL_MOVIE_STS_CANCELED}, 
		{MovieRequestType::kPrepare, VHAL_MOVIE_STS_FAILED},
		{MovieRequestType::kClear, VHAL_MOVIE_STS_NONE},
		{MovieRequestType::kClear, VHAL_MOVIE_STS_PLAYING},
		{MovieRequestType::kStart, VHAL_MOVIE_STS_NONE},
		{MovieRequestType::kStart, VHAL_MOVIE_STS_PLAYING},
		{MovieRequestType::kStart, VHAL_MOVIE_STS_FINISHED}, 
		{MovieRequestType::kStart, VHAL_MOVIE_STS_CANCELED}, 
		{MovieRequestType::kStart, VHAL_MOVIE_STS_FAILED},
		{MovieRequestType::kCancel, VHAL_MOVIE_STS_NONE},
		{MovieRequestType::kCancel, VHAL_MOVIE_STS_READY}, 
		{MovieRequestType::kCancel, VHAL_MOVIE_STS_FINISHED}, 
		{MovieRequestType::kCancel, VHAL_MOVIE_STS_CANCELED},
		{MovieRequestType::kCancel, VHAL_MOVIE_STS_FAILED}
	};

	bool ret{};
	const auto it = std::find_if(error_combination_list.begin(), error_combination_list.end(),
		[request, play_status] (const ErrorCombination e) noexcept {return ((e.request == request) && (e.play_status == play_status));});
	if (it != error_combination_list.end())
	{
		VHAL_LOGE("combination is invalid. play_status=%d request=%d", play_status, request);
		ret = false;
	}
	else
	{
		ret = true;
	}
	
	return ret;
}

/*****************************************************************************
 処理概要：	vhal_gst_playerのイベントハンドラー
 引数    ：	int32_t result	動画再生状態
 戻り値  ：	なし
*****************************************************************************/
void CVhalMovieControl::HandleGstMovieEvent(const int32_t result)
{
	VHAL_LOGI("result=%d", result);

	std::unique_ptr<CVhalEventItemMovieWait> p_movie_end_event{std::make_unique<CVhalEventItemMovieWait>()};
	p_movie_end_event->SetName(std::string("movie event"));
	p_movie_end_event->SetData(this, result);
	const int32_t ret{p_event_route_->WriteEvent(p_movie_end_event.get())};
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGEW("p_event_route_->WriteEvent ret=%d", ret);
	}
	else
	{
		(void)p_movie_end_event.release();
	}
}

} /* namespace videohal */

