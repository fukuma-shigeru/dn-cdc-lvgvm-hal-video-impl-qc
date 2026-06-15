/*******************************************************************************
    機能名称    ：  画面キャプチャ制御モジュール
    ファイル名称：  vhal_screen_shot.cpp
*******************************************************************************/
#include "vhal_screen_shot.h"

#include "vhal_define.h"
#include "vhal_log.h"
#include "vhal_layout_mng.h"
#include "vhal_main_control.h"
#include "vhal_event_item_screen_shot_event.h"

namespace videohal
{

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalScreenShot::CVhalScreenShot(void) noexcept
{
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalScreenShot::~CVhalScreenShot(void)
{
	Finalize();
}

/*****************************************************************************
 処理概要：	初期化処理（内部リソースの確保）
 引数    ：	CVhalMainControl		*p_main				(i)メインコントロールインスタンスポインタ
          	CVhalLayoutManager		*p_layout_mng		(i)レイアウト制御インスタンスポインタ
 戻り値  ：	処理結果
           		VHAL_ERR_****		エラー
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalScreenShot::Initialize(CVhalMainControl * const p_main, CVhalLayoutManager * const p_layout_mng)
{
	int32_t ret{VHAL_SUCCESS};

	p_main_ = p_main;
	p_layout_mng_ = p_layout_mng;

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
			ret = p_main->RegisterEventSource(p_route_.get());
			if (VHAL_SUCCESS != ret)
			{
				VHAL_LOGE("RegisterEventSource error. ret=%d", ret);
			}
			else
			{
				/* スレッド生成 */
				ret = CVhalWorkerThread::Start();
				if (VHAL_SUCCESS != ret)
				{
					VHAL_LOGE("CVhalWorkerThread Start error. ret=%d", ret);
				}
			}
		}
	}

	return ret;
}

/*****************************************************************************
 処理概要：	終了処理（内部リソースの解放）
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalScreenShot::Finalize(void)
{
	if (p_route_ != nullptr)
	{
		p_main_->ClearEventSource(p_route_.get());
		p_route_ = nullptr;
	}
}

/*****************************************************************************
 処理概要：	スレッド実行処理
 引数    ：	const VhalScreenShotData& data		(i)画面キャプチャデータ
	        const bool terminate				(i)処理強制終了(false:継続 true:終了)
 戻り値  ：	処理結果
           		VHAL_ERR_****		エラー
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalScreenShot::Execute(const VhalScreenShotData& data, const bool terminate) const noexcept
{
	int32_t result{VHAL_CAPTURE_STS_FAILED};
	int32_t ret{VHAL_SUCCESS};

	/* スクリーンショット取得 */
	if ( nullptr != p_layout_mng_ )
	{
		/* キャプチャタイプに応じて処理を分岐 */
		if (ScreenShotType::SCREEN == data.GetType())
		{
			ret = p_layout_mng_->GetIviScreenshot(data.GetIviId(), data.GetFilePath());
			if (VHAL_SUCCESS == ret)
			{
				result = VHAL_CAPTURE_STS_SUCCESS;
			}
			else
			{
				VHAL_LOGE("GetIviScreenshot error. ret=%d", ret);
				result = VHAL_CAPTURE_STS_FAILED;
			}
		}
		else if (ScreenShotType::SURFACE == data.GetType())
		{
			ret = p_layout_mng_->GetIviSurfaceScreenshot(data.GetIviId(), data.GetFilePath());
			if (VHAL_SUCCESS == ret)
			{
				result = VHAL_CAPTURE_STS_SUCCESS;
			}
			else
			{
				VHAL_LOGE("GetIviSurfaceScreenshot error. ret=%d", ret);
				result = VHAL_CAPTURE_STS_FAILED;
			}
		}
		else
		{
			VHAL_LOGE("Unknown capture type: %d", static_cast<int32_t>(data.GetType()));
			result = VHAL_CAPTURE_STS_FAILED;
		}
	}

	VHAL_LOGD("screenshot event. result=%d, terminate=%d", result, terminate);

	/* VideoHALイベントシステムにイベント(処理結果)を自投げ */
	std::unique_ptr<CVhalEventItemScreenShotEvent> p_screenshot_event{std::make_unique<videohal::CVhalEventItemScreenShotEvent>()};
	if (nullptr == p_screenshot_event)
	{
		ret = VHAL_ERR_MEMORY;
		VHAL_LOGE("new CVhalEventItemScreenShotEvent error. ret=%d", ret);
	}
	else
	{
		p_screenshot_event->SetName(std::string("screenshot event"));
		p_screenshot_event->SetData(p_layout_mng_, result, data.GetType());
		ret = p_route_->WriteEvent(p_screenshot_event.get());
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGEW("WriteEvent(p_screenshot_event) error. ret=%d", ret);
		}
		else
		{
			(void)p_screenshot_event.release();
		}
	}

	return ret;
}

} /* namespace videohal */

