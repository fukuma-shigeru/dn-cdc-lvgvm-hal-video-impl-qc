/*******************************************************************************
    機能名称    ：  iviコントローラーモジュール
    ファイル名称：  vhal_ivi_controller.cpp
*******************************************************************************/
#include "vhal_ivi_controller.h"

#include <iostream>
#include <ilm/ilm_control.h>
#include <json/json.h>
#include <cmath>

#include "vhal_define.h"
#include "vhal_log.h"
#include "vhal_round_cast.h"
#include "vhal_event_item_surface_configure.h"
#include "vhal_event_item_ivi_event.h"
#include "vhal_main_control.h"
#include "vhal_layout_mng.h"
#include "vhal_debug_system.h"

namespace videohal
{

namespace
{

CVhalIviController* pIviController{nullptr};		/* ListenerIlmEventSurface用 */

extern "C" void ListenerIlmEvent(ilmObjectType object, t_ilm_uint id, t_ilm_bool created, void* user_data)
{
	CVhalIviController* const p_ivi_controller{static_cast<CVhalIviController*>(user_data)};
	if( nullptr != p_ivi_controller )
	{
		bool nty_created{false};
		if (ILM_TRUE == created)
		{
			nty_created = true;
		}
		if (ILM_SURFACE == object)
		{
			p_ivi_controller->NotifyIviEvent(false, UI32ToI32(id), nty_created);
		}
		else if (ILM_LAYER == object)
		{
			p_ivi_controller->NotifyIviEvent(true, UI32ToI32(id), nty_created);
		}
		else
		{
			/* coverity対策 */
		}
	}
}

extern "C" void ListenerIlmEventSurface(t_ilm_uint id, struct ilmSurfaceProperties* sp, t_ilm_notification_mask m)
{
	uint32_t	event_mask{static_cast<uint32_t>(ILM_NOTIFICATION_VISIBILITY)};
	if (event_mask == (static_cast<uint32_t>(m) & event_mask))
	{
		VHAL_LOGI("ilm_notification_visibility surface_id=%d visibility=%d", id, sp->visibility);
	}
	event_mask = static_cast<uint32_t>(ILM_NOTIFICATION_DEST_RECT);
	if (event_mask == (static_cast<uint32_t>(m) & event_mask))
	{
		VHAL_LOGI("ilm_notification_dest_rect surface_id=%d dest_rect(%d,%d,%d,%d)", 
			id, sp->destX, sp->destY, sp->destWidth, sp->destHeight);
	}

	if (I32ToUI32(static_cast<int32_t>(ILM_NOTIFICATION_CONFIGURED)) == (static_cast<unsigned>(m) & static_cast<unsigned>(ILM_NOTIFICATION_CONFIGURED)))
	{
		if( nullptr != pIviController )
		{
			pIviController->NotifyIviConfigureSurface(UI32ToI32(id), UI32ToI32(sp->origSourceWidth), UI32ToI32(sp->origSourceHeight));
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//			int32_t fail_ret{0};
//			bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-032",fail_ret)};
//			if(true == fail)
//			{
//				sp->origSourceWidth = static_cast<uint32_t>(fail_ret);
//			}
//#endif
			if( (0U == sp->origSourceWidth) || (0U == sp->origSourceHeight) )
			{
				VHAL_LOGE("error NotifyIviConfigureSurface. Width=%d, Height=%d", sp->origSourceWidth, sp->origSourceHeight);
			}
		}
	}
}

}	// namespace




/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalIviController::CVhalIviController(void)
	:p_main_(nullptr)
	,p_route_(nullptr)
	,p_ivi_listener_(nullptr)
{
	VHAL_LOGV("CVhalIviController is created. this=%p", this);
	pIviController = this;
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalIviController::~CVhalIviController(void)
{
	VHAL_LOGV("CVhalIviController is deleted. this=%p", this);
	Finalize();
	pIviController = nullptr;
}

/*****************************************************************************
 処理概要：	初期化処理（内部リソースの確保）
 引数    ：	CVhalMainControl	*p_main		(i)メインコントロールインスタンスポインタ
 戻り値  ：	処理結果
           		VHAL_ERR_ILM_API	ilm APIエラー
           		VHAL_SUCCESS		正常終了
 フェールセーフNo  ：	F-VHAL-R-018
                   ：	F-VHAL-N-124
*****************************************************************************/
int32_t CVhalIviController::Initialize(CVhalMainControl * const p_main, CVhalIviEventListenerBase* const pIviEventListener)
{
	int32_t ret;
	ilmErrorTypes ilm_ret;

	p_main_ = p_main;
	p_ivi_listener_ = pIviEventListener;

	p_route_ = std::make_unique<CVhalEventRoute>();
	ret = p_route_->Initialize();
	if (0 > ret)
	{
		VHAL_LOGE("EventRoute Initialize error. ret=%d", ret);
		return ret;
	}

	ret = p_main->RegisterEventSource(p_route_.get());
	if (0 > ret)
	{
		VHAL_LOGE("RegisterEventSource error. ret=%d", ret);
		return ret;
	}

	uint32_t count{0U};
	while (true)
	{
		ilm_ret = ilm_init();
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//		int32_t fail_ret{0};
//		bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-R-018",fail_ret)};
//		if(true == fail)
//		{
//			ilm_ret = static_cast<ilmErrorTypes>(fail_ret);
//		}
//#endif
		if (ILM_SUCCESS == ilm_ret)
		{
			VHAL_LOGI("ilm_init() success. retry count(%d)", count);
			break;
		}

		count ++;
		constexpr uint32_t kVhalIlmInitRetryCount{120U};	/* ilm_init（weston接続）のリトライ回数 */
		if (count >= kVhalIlmInitRetryCount)
		{
			VHAL_LOGE("ilm_init() retry count(%d) over ilm_ret=%d.", count, ilm_ret);
			return VHAL_ERR_ILM_API;
		}

		constexpr uint32_t kVhalIlmInitRetryWait{500U};		/* ilm_init（weston接続）のリトライ待ち時間(ms) */
		ret = usleep(kVhalIlmInitRetryWait * 1000U);
		if (0 != ret)
		{
			VHAL_LOGE("usleep error. ret=%d", ret);
		}
	}

//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	int32_t fail_ret{0};
//	bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-124",fail_ret)};
//	if(true == fail)
//	{
//		ilm_ret = static_cast<ilmErrorTypes>(fail_ret);
//	}
//	else
//	{
//		ilm_ret = ilm_registerNotification(&ListenerIlmEvent, this);
//	}
//#else
	ilm_ret = ilm_registerNotification(&ListenerIlmEvent, this);
//#endif
	if (ILM_SUCCESS != ilm_ret)
	{
		VHAL_LOGE("ilm_registerNotification() error. ilm_ret=%d", ilm_ret);
		return VHAL_ERR_ILM_API;
	}

	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	終了処理（内部リソースの解放）
 引数    ：	なし
 戻り値  ：	なし
 フェールセーフNo  ：	F-VHAL-N-019
                   ：	F-VHAL-N-125
                   ：	F-VHAL-N-126
*****************************************************************************/
void CVhalIviController::Finalize(void)
{
	ilmErrorTypes ilm_ret{ilm_unregisterNotification()};
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	int32_t fail_ret{0};
//	bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-125",fail_ret)};
//	if(true == fail)
//	{
//		ilm_ret = static_cast<ilmErrorTypes>(fail_ret);
//	}
//#endif
	if (ILM_SUCCESS != ilm_ret)
	{
		VHAL_LOGE("ilm_unregisterNotification() error. ilm_ret=%d", ilm_ret);
	}

	/* ilmAPIライブラリの終了 */
	t_ilm_bool ret_ilm{ILM_FALSE};
	ret_ilm = ilm_isInitialized();
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	fail = CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-126",fail_ret);
//	if(true == fail)
//	{
//		ret_ilm = static_cast<ilmErrorTypes>(fail_ret);
//	}
//#endif
	if (ILM_TRUE == ret_ilm)
	{
		ilm_ret = ilm_destroy();
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//		fail = CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-019",fail_ret);
//		if(true == fail)
//		{
//			ilm_ret = static_cast<ilmErrorTypes>(fail_ret);
//		}
//#endif
		if (ILM_SUCCESS != ilm_ret)
		{
			VHAL_LOGE("ilm_destroy error. ilm_ret=%d", ilm_ret);
		}
	}
	else
	{
		VHAL_LOGE("ilm_isInitialized error. ret_ilm=%d", ret_ilm);
	}

	p_main_->ClearEventSource(p_route_.get());
	p_route_ = nullptr;

	p_ivi_listener_ = nullptr;
}

/*****************************************************************************
 処理概要：	スクリーンIDの取得
 引数    ：	なし
 戻り値  ：	スクリーンIDのvector
 フェールセーフNo  ：	F-VHAL-C-020
*****************************************************************************/
std::vector<uint32_t> CVhalIviController::GetScreenIDs(void) const
{
	ilmErrorTypes ret_ilm{ILM_SUCCESS};
	t_ilm_uint count{0U};
	t_ilm_uint *p_ids{nullptr};
	std::vector<uint32_t> ids{};

	ret_ilm = ilm_getScreenIDs(&count, &p_ids);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	int32_t fail_ret{0};
//	bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-C-020",fail_ret)};
//	if(true == fail)
//	{
//		ret_ilm = static_cast<ilmErrorTypes>(fail_ret);
//	}
//#endif
	if (ILM_SUCCESS == ret_ilm)
	{
		for (uint32_t i{0U}; i < count; i++)
		{
			ids.push_back(p_ids[i]);
		}

		if (p_ids != nullptr)
		{
			free(p_ids);
		}
	}
	else
	{
		VHAL_LOGE("error ilm_getScreenIDs ret=%d", ret_ilm);
	}

	return ids;
}

/*****************************************************************************
 処理概要：	スクリーン解像度の取得
 引数    ：	int32_t	screenId	(i)スクリーンID
         ：	int32_t	&width		(o)幅
         ：	int32_t	&height		(o)高さ
 戻り値  ：	処理結果
           		VHAL_ERR_ILM_API	ilm APIエラー
           		VHAL_SUCCESS		正常終了
 フェールセーフNo  ：	F-VHAL-R-021
*****************************************************************************/
int32_t CVhalIviController::GetScreenResolution(const int32_t screenId, int32_t& width, int32_t& height) const
{
	int32_t ret{VHAL_SUCCESS};
	ilmErrorTypes ret_ilm{ILM_SUCCESS};
	t_ilm_uint iviwidth{0U};
	t_ilm_uint iviheight{0U};

	// ilmスクリーン生成が遅いことがあるため、取得失敗時はリトライを行う
	constexpr uint32_t kVhalIlmGetScreenResolutionCount{25U};		/* ilm_getScreenResolutionのリトライ回数 */
	for ( uint32_t i{0U}; i<kVhalIlmGetScreenResolutionCount; ++i )
	{
		ret_ilm = ilm_getScreenResolution(I32ToUI32(screenId), &iviwidth, &iviheight);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//		int32_t fail_ret{0};
//		bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-R-021",fail_ret)};
//		if(true == fail)
//		{
//			ret_ilm = static_cast<ilmErrorTypes>(fail_ret);
//			width = 0;
//			height = 0;
//		}
//#endif
		if (ILM_SUCCESS == ret_ilm)
		{
			width  = UI32ToI32(iviwidth);
			height = UI32ToI32(iviheight);
			break;
		}
		else
		{
			VHAL_LOGI("error ilm_getScreenResolution id=%d ret_ilm=%d count=%d", screenId, ret_ilm, i);
			constexpr uint32_t kVhalIlmGetScreenResolutionRetryWait{200U};	/* ilm_getScreenResolutionのリトライ待ち時間(ms) */
			const int32_t ret_sleep{usleep(kVhalIlmGetScreenResolutionRetryWait * 1000U)};
			if (0 != ret_sleep)
			{
				VHAL_LOGE("usleep error. ret_sleep=%d", ret_sleep);
			}
		}
	}

	if ( ILM_SUCCESS != ret_ilm )
	{
		VHAL_LOGE("ilm_getScreenResolution retry over id=%d ret_ilm=%d ", screenId, ret_ilm);
		ret = VHAL_ERR_ILM_API;
	}

	return ret;
}

/*****************************************************************************
 処理概要：	ilmによる変更の反映
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_ERR_ILM_API	ilm APIエラー
           		VHAL_SUCCESS		正常終了
フェールセーフNo  ：	F-VHAL-N-022
*****************************************************************************/
int32_t CVhalIviController::CommitChanges(void)
{
	ilmErrorTypes ret_ilm{ILM_SUCCESS};

	ret_ilm = ilm_commitChanges();
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	int32_t fail_ret{0};
//	bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-022",fail_ret)};
//	if(true == fail)
//	{
//		ret_ilm = static_cast<ilmErrorTypes>(fail_ret);
//	}
//#endif
	if (ILM_SUCCESS != ret_ilm)
	{
		return VHAL_ERR_ILM_API;
	}
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	レイヤーの作成
 引数    ：	int32_t							screenId	(i)スクリーンID
         ：	const std::vector<LayerInfo>	&layerList	(i)レイヤーリスト
 戻り値  ：	処理結果
           		VHAL_ERR_ILM_API	ilm APIエラー
           		VHAL_SUCCESS		正常終了
 フェールセーフNo  ：	F-VHAL-N-023
                    	F-VHAL-N-024
*****************************************************************************/
int32_t CVhalIviController::CreateLayers(const int32_t screenId, const std::vector<LayerInfo>& layerList)
{
	std::vector<t_ilm_layer> ivilayer_ids{};

	std::vector<LayerInfo>::const_iterator itlayer{layerList.begin()};
	const std::vector<LayerInfo>::const_iterator itlayerend{layerList.end()};
	for( ; itlayer!=itlayerend; ++itlayer)
	{
		t_ilm_layer ivilayer_id{I32ToUI32(itlayer->layer_id_)};
		ilmErrorTypes ret_ilm{ilm_layerCreateWithDimension(&ivilayer_id, I32ToUI32(itlayer->width_), I32ToUI32(itlayer->height_))};
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//		int32_t fail_ret{0};
//		bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-023",fail_ret)};
//		if(true == fail)
//		{
//			ret_ilm = static_cast<ilmErrorTypes>(fail_ret);
//		}
//#endif
		if (ILM_SUCCESS == ret_ilm)
		{
			// 作成したらvisible設定も行う
			ret_ilm = ilm_layerSetVisibility(ivilayer_id, ILM_TRUE);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//			fail = CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-024",fail_ret);
//			if(true == fail)
//			{
//				ret_ilm = static_cast<ilmErrorTypes>(fail_ret);
//			}
//#endif
			if (ILM_SUCCESS != ret_ilm)
			{
				VHAL_LOGI("error ilm_layerSetVisibility id=%d", ivilayer_id);
				// ひとつ失敗しても処理は継続
			}
		} else {
			VHAL_LOGW("error ilm_layerCreateWithDimension id=%d", ivilayer_id);
		}

		// 既に存在するエラー時にも配置されるように、必ずvectorに追加する。
		ivilayer_ids.push_back(ivilayer_id);
	}

	// 作成したレイヤをまとめてスクリーンに登録
	return SetLayerOrder(screenId, ivilayer_ids);
}

/*****************************************************************************
 処理概要：	レイヤーの並べ替え
 引数    ：	int32_t							screenId		(i)スクリーンID
         ：	const std::vector<t_ilm_layer>	&ivilayer_ids	(i)レイヤーIDリスト
 戻り値  ：	処理結果
           		VHAL_ERR_ILM_API	ilm APIエラー
           		VHAL_SUCCESS		正常終了
 フェールセーフNo  ：	F-VHAL-N-025
                    	F-VHAL-N-026
*****************************************************************************/
int32_t CVhalIviController::SetLayerOrder(const int32_t screenId, std::vector<t_ilm_layer>& ivilayer_ids)
{
	ilmErrorTypes ret_ilm{ILM_SUCCESS};

	ret_ilm = ilm_displaySetRenderOrder(I32ToUI32(screenId), &ivilayer_ids[0], SizeToUI32(static_cast<std::size_t>(ivilayer_ids.size())));
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	int32_t fail_ret{0};
//	bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-025",fail_ret)};
//	if(true == fail)
//	{
//		ret_ilm = static_cast<ilmErrorTypes>(fail_ret);
//	}
//#endif
	if (ILM_SUCCESS != ret_ilm)
	{
		VHAL_LOGE("error ilm_displaySetRenderOrder id=%d", screenId);
		return VHAL_ERR_ILM_API;
	}

	ret_ilm = ilm_commitChanges();
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	fail = CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-026",fail_ret);
//	if(true == fail)
//	{
//		ret_ilm = static_cast<ilmErrorTypes>(fail_ret);
//	}
//#endif
	if (ILM_SUCCESS != ret_ilm)
	{
		VHAL_LOGE("error ilm_commitChanges");
		return VHAL_ERR_ILM_API;
	}

	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	サーフェスをレイヤーへ登録
 引数    ：	int32_t	surfaceId	(i)サーフェスID
         ：	int32_t	layerId		(i)レイヤーID
 戻り値  ：	処理結果
           		VHAL_ERR_ILM_API	ilm APIエラー
           		VHAL_SUCCESS		正常終了
 フェールセーフNo  ：	F-VHAL-N-027
                    	F-VHAL-N-028
                    	F-VHAL-N-029
                    	F-VHAL-N-030
                    	F-VHAL-N-031
*****************************************************************************/
int32_t CVhalIviController::AddSurfaceToLayer(const int32_t surfaceId, const int32_t layerId)
{
	ilmErrorTypes ret_ilm{ILM_SUCCESS};

	// 初期非表示
	ret_ilm = ilm_surfaceSetVisibility(I32ToUI32(surfaceId), ILM_FALSE);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	int32_t fail_ret{0};
//	bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-027",fail_ret)};
//	if(true == fail)
//	{
//		ret_ilm = static_cast<ilmErrorTypes>(fail_ret);
//	}
//#endif
	if (ILM_SUCCESS != ret_ilm) {
		VHAL_LOGE("error ilm_surfaceSetVisibility ret_ilm=%d", ret_ilm);
	}

	// レイヤにサーフェス登録
	ret_ilm = ilm_layerAddSurface(I32ToUI32(layerId), I32ToUI32(surfaceId));
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	fail = CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-028",fail_ret);
//	if(true == fail)
//	{
//		ret_ilm = static_cast<ilmErrorTypes>(fail_ret);
//	}
//#endif
	if (ILM_SUCCESS != ret_ilm) {
		VHAL_LOGE("error ilm_layerAddSurface ret_ilm=%d", ret_ilm);
	}

	// サーフェス状態変化通知登録
	ret_ilm = ilm_surfaceAddNotification(I32ToUI32(surfaceId), &ListenerIlmEventSurface);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	fail = CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-029",fail_ret);
//	if(true == fail)
//	{
//		ret_ilm = static_cast<ilmErrorTypes>(fail_ret);
//	}
//#endif
	if (ILM_SUCCESS != ret_ilm) {
		VHAL_LOGE("error ilm_surfaceAddNotification ret_ilm=%d", ret_ilm);
	}

	ret_ilm = ilm_commitChanges();
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	fail = CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-030",fail_ret);
//	if(true == fail)
//	{
//		ret_ilm = static_cast<ilmErrorTypes>(fail_ret);
//	}
//#endif
	if (ILM_SUCCESS != ret_ilm) {
		VHAL_LOGE("error ilm_commitChanges ret_ilm=%d", ret_ilm);
	}

	struct ilmSurfaceProperties sp;
	ret_ilm = ilm_getPropertiesOfSurface(I32ToUI32(surfaceId), &sp);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	fail = CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-031",fail_ret);
//	if(true == fail)
//	{
//		ret_ilm = static_cast<ilmErrorTypes>(fail_ret);
//	}
//#endif
	if (ILM_SUCCESS != ret_ilm) {
		VHAL_LOGE("error ilm_getPropertiesOfSurface ret_ilm=%d", ret_ilm);
	}

	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	サーフェスの可視状態取得
 引数    ：	int32_t	surfaceId	(i)サーフェスID
         ：	 bool& visibility	(o)可視状態
 戻り値  ：	処理結果
           		VHAL_ERR_ILM_API	ilm APIエラー
           		VHAL_SUCCESS		正常終了
 フェールセーフNo  ：	F-VHAL-N-033A
                   ：	F-VHAL-C-033B
*****************************************************************************/
int32_t CVhalIviController::GetSurfaceVisibility(const int32_t surfaceId, bool& visibility) const
{
	ilmErrorTypes ilm_ret{ILM_SUCCESS};
	t_ilm_bool ilm_visibility{ILM_FALSE};
	int32_t result{VHAL_SUCCESS};

	ilm_ret = ilm_surfaceGetVisibility(I32ToUI32(surfaceId), &ilm_visibility);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	int32_t fail_ret{0};
//	bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-033A",fail_ret)};
//	if(true == fail)
//	{
//		ilm_ret = static_cast<ilmErrorTypes>(fail_ret);
//	}
//#endif
	if (ILM_SUCCESS != ilm_ret)
	{
		VHAL_LOGE("ilm_surfaceGetVisibility() surface_id=%d, ilm_ret=%d", surfaceId, ilm_ret);
		result = VHAL_ERR_ILM_API;
	}
	else
	{
		if (ILM_TRUE == ilm_visibility)
		{
			visibility = true;
		}
	}

	return result;
}

/*****************************************************************************
 処理概要：	サーフェスの可視状態設定
 引数    ：	int32_t	surfaceId	(i)サーフェスID
         ：	bool	visible		(i)可視状態
 戻り値  ：	処理結果
           		VHAL_ERR_ILM_API	ilm APIエラー
           		VHAL_SUCCESS		正常終了
 フェールセーフNo  ：	F-VHAL-N-034
*****************************************************************************/
int32_t CVhalIviController::SetSurfaceVisibility(const int32_t surfaceId, const bool visible)
{
	int32_t ret{VHAL_SUCCESS};
	ilmErrorTypes ilm_ret;
	t_ilm_bool ilm_visibility{ILM_FALSE};

	if (true == visible)
	{
		ilm_visibility = ILM_TRUE;
	}
	VHAL_LOGD("surfaceId=%d, ilm_visibility=%d", surfaceId, ilm_visibility);
	ilm_ret = ilm_surfaceSetVisibility(I32ToUI32(surfaceId), ilm_visibility);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	int32_t fail_ret{0};
//	bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-034",fail_ret)};
//	if(true == fail)
//	{
//		ilm_ret = static_cast<ilmErrorTypes>(fail_ret);
//	}
//#endif
	if (ILM_SUCCESS != ilm_ret)
	{
		VHAL_LOGW("ilm_surfaceSetVisibility() surface_id=%d, ilm_ret=%d", surfaceId, ilm_ret);
		ret = VHAL_ERR_ILM_API;
	}
	
	return ret;
}

/*****************************************************************************
 処理概要：	サーフェスの不透明度設定
 引数    ：	int32_t		surfaceId	(i)サーフェスID
         ：	uint32_t	opacity		(i)不透明度(0-100)
 戻り値  ：	処理結果
           		VHAL_ERR_ILM_API	ilm APIエラー
           		VHAL_SUCCESS		正常終了
フェールセーフNo  ：	F-VHAL-N-036
*****************************************************************************/
int32_t CVhalIviController::SetSurfaceOpacity(const int32_t surfaceId, uint32_t opacity)
{
	ilmErrorTypes ilm_ret;
	t_ilm_float ilm_opacity{1.0F};
	int32_t result{VHAL_SUCCESS};

	if (100U < opacity)
	{
		opacity = 100U;
	}
	ilm_opacity = static_cast<t_ilm_float>(opacity) * 0.01F;
	ilm_ret = ilm_surfaceSetOpacity(I32ToUI32(surfaceId), ilm_opacity);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	int32_t fail_ret{0};
//	bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-036",fail_ret)};
//	if(true == fail)
//	{
//		ilm_ret = static_cast<ilmErrorTypes>(fail_ret);
//	}
//#endif
	if (ILM_SUCCESS != ilm_ret)
	{
		VHAL_LOGE("ilm_surfaceSetOpacity() surface_id=%d, ilm_ret=%d", surfaceId, ilm_ret);
		result = VHAL_ERR_ILM_API;
	}
	
	return result;
}

/*****************************************************************************
 処理概要：	サーフェスのソース矩形の設定
 引数    ：	int32_t	surfaceId	(i)サーフェスID
         ：	int32_t	x			(i)x座標
         ：	int32_t	y			(i)y座標
         ：	int32_t	w			(i)幅
         ：	int32_t	h			(i)高さ
 戻り値  ：	処理結果
           		VHAL_ERR_ILM_API	ilm APIエラー
           		VHAL_SUCCESS		正常終了
 フェールセーフNo  ：	F-VHAL-N-037
*****************************************************************************/
int32_t CVhalIviController::SetSurfaceSourceRectangle(const int32_t surfaceId, const int32_t x, const int32_t y, const int32_t w, const int32_t h) const
{
	ilmErrorTypes ilm_ret{ILM_SUCCESS};
	int32_t result{VHAL_SUCCESS};

	/* 入力矩形設定 */
	VHAL_LOGD("surfaceId=%d, x=%d, y=%d, w=%d, h=%d", surfaceId, x, y, w, h);
	ilm_ret = ilm_surfaceSetSourceRectangle(static_cast<t_ilm_surface>(surfaceId), x, y, w, h);
// MISRA C++-2008 Rule 16-2-1 #else
	// 入力矩形設定
	// VHAL_LOGD("surfaceId=%d, x=%d, y=%d, w=%d, h=%d", surfaceId, x, y, w, h);
	// ilm_ret = ilm_surfaceSetSourceRectangle(static_cast<t_ilm_surface>(surfaceId), x, y, w, h);
// MISRA C++-2008 Rule 16-2-1 #endif
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	int32_t fail_ret{0};
//	bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-037",fail_ret)};
//	if(true == fail)
//	{
//		ilm_ret = static_cast<ilmErrorTypes>(fail_ret);
//	}
//#endif
	if (ILM_SUCCESS != ilm_ret)
	{
		VHAL_LOGE("ilm_surfaceSetSourceRectangle() surface_id=%d, ilm_ret=%d", surfaceId, ilm_ret);
		result = VHAL_ERR_ILM_API;
	}
	
	return result;
}

/*****************************************************************************
 処理概要：	サーフェスのデスティネーション矩形の設定
 引数    ：	int32_t	x	(i)x座標
         ：	int32_t	y	(i)y座標
         ：	int32_t	w	(i)幅
         ：	int32_t	h	(i)高さ
 戻り値  ：	処理結果
           		VHAL_ERR_ILM_API	ilm APIエラー
           		VHAL_SUCCESS		正常終了
 フェールセーフNo  ：	F-VHAL-N-038
*****************************************************************************/
int32_t CVhalIviController::SetSurfaceDestinationRectangle(const int32_t surfaceId, const int32_t x, const int32_t y, const int32_t w, const int32_t h)
{
	ilmErrorTypes ilm_ret{ILM_SUCCESS};
	int32_t result{VHAL_SUCCESS};

	// 出力矩形設定
	VHAL_LOGD("surfaceId=%d, x=%d, y=%d, w=%d, h=%d", surfaceId, x, y, w, h);
	ilm_ret = ilm_surfaceSetDestinationRectangle(I32ToUI32(surfaceId), x, y, w, h);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	int32_t fail_ret{0};
//	bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-038",fail_ret)};
//	if(true == fail)
//	{
//		ilm_ret = static_cast<ilmErrorTypes>(fail_ret);
//	}
//#endif
	if (ILM_SUCCESS != ilm_ret)
	{
		VHAL_LOGE("ilm_surfaceSetDestinationRectangle() surface_id=%d, ilm_ret=%d", surfaceId, ilm_ret);
		result = VHAL_ERR_ILM_API;
	}
	
	return result;
}

/*****************************************************************************
 処理概要：	サーフェスのconfigureイベント通知
 引数    ：	int32_t	surfaceId	(i)サーフェスID
         ：	int32_t	width	(i)幅
         ：	int32_t	height	(i)高さ
 戻り値  ：	なし
*****************************************************************************/
void CVhalIviController::NotifyIviConfigureSurface(const int32_t surfaceId, const int32_t width, const int32_t height)
{
	std::unique_ptr<CVhalEventItemSurfaceConfigure> p_configure{std::make_unique<CVhalEventItemSurfaceConfigure>()};
	p_configure->SetName(std::string("ivi surface configure"));
	p_configure->SetData(p_ivi_listener_, I32ToUI32(surfaceId), I32ToUI32(width), I32ToUI32(height));
	const int32_t ret{p_route_->WriteEvent(p_configure.get())};
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGEW("p_route_->WriteEvent ret=%d", ret);
	}
	else
	{
		(void)p_configure.release();
	}
}

/*****************************************************************************
 処理概要：	IVIイベント通知
 引数    ：	bool	object			(i)true:layer false:surface
         ：	int32_t	id				(i)IVI-ID
         ：	bool	created			(i)true:create false:remove
 戻り値  ：	なし
*****************************************************************************/
void CVhalIviController::NotifyIviEvent(const bool object, const int32_t id, const bool created)
{
	std::unique_ptr <CVhalEventItemIviEvent> p_ivi_event{std::make_unique<CVhalEventItemIviEvent>()};
	p_ivi_event->SetName(std::string("ivi event"));
	p_ivi_event->SetData(p_ivi_listener_, object, id, created);
	const int32_t ret{p_route_->WriteEvent(p_ivi_event.get())};
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGEW("p_route_->WriteEvent ret=%d", ret);
	}
	else
	{
		(void)p_ivi_event.release();
	}
}

/*****************************************************************************
 処理概要：	生成されているサーフェスID一覧の取得
 引数    ：	std::vector<int32_t>& 	ivisurface_ids		(o)生成されているサーフェスID一覧
 戻り値  ：	処理結果
           		VHAL_ERR_ILM_API	ilm APIエラー
           		VHAL_SUCCESS		正常終了
 フェールセーフNo  ：	F-VHAL-C-039
*****************************************************************************/
int32_t CVhalIviController::GetSurfaceIds(std::vector<int32_t>& ivisurface_ids) const
{
	ilmErrorTypes ilm_ret{ILM_SUCCESS};
	t_ilm_int length{0};
	t_ilm_surface* pArray{nullptr};

	/* サーフェス一覧取得 */
	ilm_ret = ilm_getSurfaceIDs(&length, &pArray);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	int32_t fail_ret{0};
//	bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-C-039",fail_ret)};
//	if(true == fail)
//	{
//		ilm_ret = static_cast<ilmErrorTypes>(fail_ret);
//	}
//#endif
	if (ILM_SUCCESS != ilm_ret)
	{
		VHAL_LOGE("ilm_getSurfaceIDs() ilm_ret=%d", ilm_ret);
		return VHAL_ERR_ILM_API;
	}

	ivisurface_ids.clear();
	for (int32_t i{0}; i < length; ++i)
	{
		ivisurface_ids.push_back(UI32ToI32(pArray[i]));
	}
	if (nullptr != pArray)
	{
		free(pArray);
	}

	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	スクリーンのiviプロパティ情報取得
 引数    ：	const int32_t		ivi_id	(i)ivi id
         ：	const std::string	&name	(i)名前
 戻り値  ：	JSON文字列
 フェールセーフNo  ：	F-VHAL-C-040
****************************************************************************/
std::string CVhalIviController::GetPropertiesOfScreen(const int32_t ivi_id, const std::string &name)
{
	ilmErrorTypes ret_ilm{ILM_SUCCESS};
	struct ilmScreenProperties screenProp;
    Json::Value screenValue{};
    Json::Value layerArray{Json::Value(Json::arrayValue)};

	ret_ilm = ilm_getPropertiesOfScreen(I32ToUI32(ivi_id), &screenProp);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	int32_t fail_ret{0};
//	bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-C-040",fail_ret)};
//	if(true == fail)
//	{
//		ret_ilm = static_cast<ilmErrorTypes>(fail_ret);
//	}
//#endif
	if (ret_ilm == ILM_SUCCESS)
	{
		if (name != screenProp.connectorName)
		{
			/* スクリーンだけは名前をサーバー側が持っている */
			VHAL_LOGW("screen[%d] name is mismatched. received=[%s], expected=[%s]",
				ivi_id, screenProp.connectorName, name.c_str());
		}

		screenValue["id"] = ivi_id;
		screenValue["name"] = name;
		screenValue["width"] = screenProp.screenWidth;
		screenValue["height"] = screenProp.screenHeight;
		screenValue["layerCount"] = screenProp.layerCount;

		for (t_ilm_uint j{0U}; j < screenProp.layerCount; j++)
		{
			layerArray[j] = screenProp.layerIds[j];
		}

		free(screenProp.layerIds);
		screenValue["layerIds"] = layerArray;
	}
	else
	{
		VHAL_LOGE("ilm_getPropertiesOfScreen error. ret_ilm=%d", ret_ilm);
	}

	Json::StreamWriterBuilder builder{};
	builder["indentation"] = "";
	return Json::writeString(builder, screenValue);
}

/*****************************************************************************
 処理概要：	レイヤーのiviプロパティ情報取得
 引数    ：	const int32_t		ivi_id	(i)ivi id
         ：	const std::string	&name	(i)名前
 戻り値  ：	JSON文字列
 フェールセーフNo  ：	F-VHAL-C-040
                   ：	F-VHAL-N-123
****************************************************************************/
std::string CVhalIviController::GetPropertiesOfLayer(const int32_t ivi_id, const std::string &name)
{
	ilmErrorTypes ret_ilm{ILM_SUCCESS};
	struct ilmLayerProperties layer_prop;
	t_ilm_int surface_count{0};
	t_ilm_surface *p_surface_ids{nullptr};
    Json::Value layer_value{};
    Json::Value surface_ids{Json::Value(Json::arrayValue)};

	// レイヤー情報
	ret_ilm = ilm_getPropertiesOfLayer(I32ToUI32(ivi_id), &layer_prop);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	int32_t fail_ret{0};
//	bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-C-040",fail_ret)};
//	if(true == fail)
//	{
//		ret_ilm = static_cast<ilmErrorTypes>(fail_ret);
//	}
//#endif
	if (ret_ilm == ILM_SUCCESS)
	{
		layer_value["id"] = ivi_id;
		layer_value["name"] = name;
		layer_value["src_x"] = layer_prop.sourceX;
		layer_value["src_y"] = layer_prop.sourceY;
		layer_value["src_w"] = layer_prop.sourceWidth;
		layer_value["src_h"] = layer_prop.sourceHeight;
		layer_value["dest_x"] = layer_prop.destX;
		layer_value["dest_y"] = layer_prop.destY;
		layer_value["dest_w"] = layer_prop.destWidth;
		layer_value["dest_h"] = layer_prop.destHeight;
		layer_value["visibility"] = static_cast<bool>(layer_prop.visibility);
		layer_value["opacity"] = DToUI32(round(layer_prop.opacity * 100.0));
	}

	ret_ilm = ilm_getSurfaceIDsOnLayer(I32ToUI32(ivi_id), &surface_count, &p_surface_ids);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	fail_ret = 0;
//	fail = CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-123",fail_ret);
//	if(true == fail)
//	{
//		ret_ilm = static_cast<ilmErrorTypes>(fail_ret);
//	}
//#endif
	if (ret_ilm == ILM_SUCCESS)
	{
		layer_value["surfaceCount"] = surface_count;
		for (t_ilm_int j{0}; j < surface_count; j++)
		{
			surface_ids[j] = p_surface_ids[j];
		}

		free(p_surface_ids);
		layer_value["surfaceIds"] = surface_ids;
	}
	else
	{
		VHAL_LOGE("ilm_getPropertiesOfLayer error. ret_ilm=%d", ret_ilm);
	}

    Json::StreamWriterBuilder builder{};
    builder["indentation"] = "";
    return Json::writeString(builder, layer_value);
}

/*****************************************************************************
 処理概要：	サーフェスのiviプロパティ情報取得
 引数    ：	const int32_t		ivi_id	(i)ivi id
         ：	const std::string	&name	(i)名前
 戻り値  ：	JSON文字列
 フェールセーフNo  ：	F-VHAL-C-040
****************************************************************************/
std::string CVhalIviController::GetPropertiesOfSurface(const int32_t ivi_id, const std::string &name)
{
	ilmErrorTypes ret_ilm{ILM_SUCCESS};
	struct ilmSurfaceProperties surface_prop;
    Json::Value surface_value{};

	ret_ilm = ilm_getPropertiesOfSurface(I32ToUI32(ivi_id), &surface_prop);
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//	int32_t fail_ret{0};
//	bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-C-040",fail_ret)};
//	if(true == fail)
//	{
//		ret_ilm = static_cast<ilmErrorTypes>(fail_ret);
//	}
//#endif
	if (ret_ilm == ILM_SUCCESS)
	{
		surface_value["id"] = ivi_id;
		surface_value["name"] = name;
		surface_value["src_x"] = surface_prop.sourceX;
		surface_value["src_y"] = surface_prop.sourceY;
		surface_value["src_w"] = surface_prop.sourceWidth;
		surface_value["src_h"] = surface_prop.sourceHeight;
		surface_value["dest_x"] = surface_prop.destX;
		surface_value["dest_y"] = surface_prop.destY;
		surface_value["dest_w"] = surface_prop.destWidth;
		surface_value["dest_h"] = surface_prop.destHeight;
		surface_value["visibility"] = static_cast<bool>(surface_prop.visibility);
		surface_value["opacity"] = DToUI32(round(surface_prop.opacity * 100.0));
		surface_value["orig_w"] = surface_prop.origSourceWidth;
		surface_value["orig_h"] = surface_prop.origSourceHeight;
	}
	else
	{
		VHAL_LOGE("ilm_getPropertiesOfSurface error. ret_ilm=%d", ret_ilm);
	}

	Json::StreamWriterBuilder builder{};
	builder["indentation"] = "";
	return Json::writeString(builder, surface_value);
}

/*****************************************************************************
 処理概要：	サーフェスのiviプロパティ情報取得(FrameCounter)
 引数    ：	const int32_t		ivi_id	(i)ivi id
 戻り値  ：	FrameCounter値
****************************************************************************/
uint32_t CVhalIviController::GetPropertiesOfSurfaceFrameCounter(const int32_t ivi_id) const
{
	ilmErrorTypes				ret_ilm{ILM_SUCCESS};
	uint32_t					frame{0U};
	struct ilmSurfaceProperties	surface_prop{};

	ret_ilm = ilm_getPropertiesOfSurface(I32ToUI32(ivi_id), &surface_prop);
	if (ret_ilm == ILM_SUCCESS)
	{
		frame = surface_prop.frameCounter;
	}
	return frame;
}

/*****************************************************************************
 処理概要：	スクリーンショット取得(screen)
 引数    ：	const int32_t		ivi_id	(i)ivi id
         ：	const std::string	&path	(i)ファイルパス
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_ERR_ILM_API	ilm APIエラー
           		VHAL_SUCCESS		正常終了
 フェールセーフNo  ：	F-VHAL-C-041
****************************************************************************/
int32_t CVhalIviController::GetScreenshot(const int32_t ivi_id, const std::string &path) const
{
	const std::lock_guard<std::mutex> lock_step{mtx_screenshot_};
	int32_t ret{VHAL_SUCCESS};

	if (0 <= ivi_id)
	{
		ilmErrorTypes ret_ilm{ILM_SUCCESS};
		VHAL_LOGI("ilm_takeScreenshot ivi_id=%d, path=%s", ivi_id, path.c_str());
		ret_ilm = ilm_takeScreenshot(static_cast<t_ilm_uint>(ivi_id), static_cast<t_ilm_const_string>(path.c_str()));
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//		int32_t fail_ret{0};
//		bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-C-041",fail_ret)};
//		if(true == fail)
//		{
//			ret_ilm = static_cast<ilmErrorTypes>(fail_ret);
//		}
//#endif
		VHAL_LOGI("ilm_takeScreenshot ret=%d", ret_ilm);
		if (ret_ilm != ILM_SUCCESS)
		{
			VHAL_LOGE("ilm_takeScreenshot error. ret=%d", ret_ilm);
			ret = VHAL_ERR_ILM_API;
		}
	}
	else
	{
		VHAL_LOGE("ivi_id=%d", ivi_id);
		ret = VHAL_ERR_PARAM;
	}

	return ret;
}

/*****************************************************************************
 処理概要：	スクリーンショット取得(surface)
 引数    ：	const int32_t		ivi_id	(i)ivi id
         ：	const std::string	&path	(i)ファイルパス
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_ERR_ILM_API	ilm APIエラー
           		VHAL_SUCCESS		正常終了
 フェールセーフNo  ：	F-VHAL-C-195
****************************************************************************/
int32_t CVhalIviController::GetSurfaceScreenshot(const int32_t ivi_id, const std::string &path) const
{
	const std::lock_guard<std::mutex> lock_step{mtx_screenshot_};
	int32_t ret{VHAL_SUCCESS};

	if (0 <= ivi_id)
	{
		ilmErrorTypes ret_ilm{ILM_SUCCESS};
		VHAL_LOGI("ilm_takeSurfaceScreenshot ivi_id=%d, path=%s", ivi_id, path.c_str());
		ret_ilm = ilm_takeSurfaceScreenshot(static_cast<t_ilm_const_string>(path.c_str()), static_cast<t_ilm_uint>(ivi_id));
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//		int32_t fail_ret{0};
//		bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-C-195",fail_ret)};
//		if(true == fail)
//		{
//			ret_ilm = static_cast<ilmErrorTypes>(fail_ret);
//		}
//#endif
		VHAL_LOGI("ilm_takeSurfaceScreenshot ret=%d", ret_ilm);
		if (ret_ilm != ILM_SUCCESS)
		{
			VHAL_LOGE("ilm_takeSurfaceScreenshot error. ret=%d", ret_ilm);
			ret = VHAL_ERR_ILM_API;
		}
	}
	else
	{
		VHAL_LOGE("ivi_id=%d", ivi_id);
		ret = VHAL_ERR_PARAM;
	}

	return ret;
}

} /* namespace videohal */

