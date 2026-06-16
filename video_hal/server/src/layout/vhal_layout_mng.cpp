/*******************************************************************************
    機能名称    ：  レイアウト制御モジュール
    ファイル名称：  vhal_layout_mng.cpp
*******************************************************************************/
#include "vhal_layout_mng.h"

#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <algorithm>

#include "vhal_define.h"
#include "vhal_log.h"
#include "vhal_round_cast.h"
#include "vhal_ivi_controller.h"
#include "vhal_main_control.h"

namespace videohal
{

namespace
{
/* layer_config.jsonに記載されている情報 */
/* スクリーン名 */
const std::string kScreenNameFront{"front"};
const std::string kScreenNameRear{"rear"};
const std::string kScreenNameICluster{"instrumentcluster"};
const std::string kScreenNameHud{"hud"};

/* レイヤー名 */
const std::string kLayerNameVideoFront{"front_video"};
const std::string kLayerNameVideoRear{"rear_video"};
const std::string kLayerNameCamera{"layer_camera"};
const std::string kLayerNmaeVideoIClusterProjection{"ic_projection"};
const std::string kLayerNmaeVideoIClusterMap{"ic_map"};

/* サーフェス名 */
const std::string kSurfaceNameMuteFrontDisplay{"mute_front_display"};
const std::string kSurfaceNameMuteFrontVideo{"mute_front_video"};
const std::string kSurfaceNameMuteFrontVideoSync{"mute_front_video_sync"};
const std::string kSurfaceNameMuteRearDisplay{"mute_rear_display"};
const std::string kSurfaceNameMuteRearVideo{"mute_rear_video"};
const std::string kSurfaceNameMuteCamera{"mute_on_camera"};
const std::string kSurfaceNameMuteIcDisplay{"mute_ic_display"};
const std::string kSurfaceNameVideoFrontBg{"background_front_video"};
const std::string kSurfaceNameVideoRearBg{"background_rear_video"};
const std::string kSurfaceNameMovie{"general_movie"};
const std::string kSurfaceNameMuteHudDisplay{"mute_hud_display"};
}


/* スクリーン情報リスト */
CVhalLayoutManager::CScreenInfoList CVhalLayoutManager::screen_info_list_
{
#if 1	// for BEVstep3
	{ kScreenNameFront,    "screen0", ""                   },	/* 異常時の保険 */
	{ kScreenNameFront,    "screen0", "DisplayPortFront"   },
	{ kScreenNameICluster, "screen2", "GVIF-TX(Meter)"     },
	{ kScreenNameRear,     "screen1", "GVIF-TX(RSE)"       },
	{ kScreenNameHud,      "screen3", "HeadUpDisplay"     }
#else
	{ kScreenNameFront,    "DP-0", ""                 },	/* 異常時の保険 */
	{ kScreenNameFront,    "DP-1", "DisplayPortFront" },
	{ kScreenNameICluster, ""    , "GVIF-TX(Meter)"   },
	{ kScreenNameRear,     ""    , "GVIF-TX(RSE)"     }
#endif
};

/* 映像パス⇒各種情報リスト */
CVhalLayoutManager::CVideoPathInfoList CVhalLayoutManager::videopath_info_list_
{
	/* 映像パス名,      サーフェス名(front,rear,camera,ic)                                      映像出力先(VideoOutputTarget) */
	{ "",              { "",                  "",                 "",        "",                VIDEO_OUTPUT_TARGET_FRONT | VIDEO_OUTPUT_TARGET_REAR | VIDEO_OUTPUT_TARGET_CAMERA | VIDEO_OUTPUT_TARGET_IC } },
	{ "DTV",           { "front_dtv",         "rear_dtv",         "",        "",                VIDEO_OUTPUT_TARGET_FRONT | VIDEO_OUTPUT_TARGET_REAR } },
	{ "HDMI",          { "front_hdmi",        "rear_hdmi",        "",        "",                VIDEO_OUTPUT_TARGET_FRONT | VIDEO_OUTPUT_TARGET_REAR } },
	{ "DRC",           { "drive_recorder",    "",                 "",        "",                VIDEO_OUTPUT_TARGET_FRONT } },
	{ "CARPLAY",       { "carplay",           "",                 "",        "ic_carplay",      VIDEO_OUTPUT_TARGET_FRONT | VIDEO_OUTPUT_TARGET_IC } },
	{ "ANDROIDAUTO",   { "android_auto",      "",                 "",        "ic_android_auto", VIDEO_OUTPUT_TARGET_FRONT | VIDEO_OUTPUT_TARGET_IC } },
	{ "MEDIAPLAYER",   { "media_player",      "",                 "",        "",                VIDEO_OUTPUT_TARGET_FRONT } },
	{ "CAMERA-IMG-ADJ",{ "camera_img_adj",    "",                 "",        "",                VIDEO_OUTPUT_TARGET_FRONT } },
	{ "CAMERA",        { "",                  "",                 "camera",  "",                VIDEO_OUTPUT_TARGET_CAMERA } },
	{ "MAP",           { "",                  "",                 "",        "ic_map",          VIDEO_OUTPUT_TARGET_IC } },
	{ "MULTISENSORY",  { "front_multisensory","rear_multisensory","",        "",                VIDEO_OUTPUT_TARGET_FRONT | VIDEO_OUTPUT_TARGET_REAR } },
	{ "FRAGRANCE",     { "fragrance",         "",                 "",        "",                VIDEO_OUTPUT_TARGET_FRONT } }
};


/*****************************************************************************
 クラス名称：CVhalIviEventListener
 処理概要  ：IVIイベントリスナクラス
*****************************************************************************/
class CVhalIviEventListener : public CVhalIviEventListenerBase {
public:

	CVhalIviEventListener(CVhalLayoutManager* const pLayoutMng) noexcept
		:p_layout_mng_(pLayoutMng)
	{
	}
	~CVhalIviEventListener(void) override = default;
  	CVhalIviEventListener(const CVhalIviEventListener& src) = delete;
	CVhalIviEventListener& operator=(const CVhalIviEventListener& src) & = delete;
	CVhalIviEventListener(CVhalIviEventListener&& src) = delete;
	CVhalIviEventListener& operator=(CVhalIviEventListener&& src) & = delete;

	void NotifyEvent(const IviObjectType objType, const int32_t id, const bool created) const noexcept override
	{
		if (IviObjectType::kIviObjectSurface == objType)
		{
			if (true == created)
			{
				VHAL_LOGI("create surface id=%d", id);
				(void)p_layout_mng_->AddSurfaceToLayer(id);
			}
			else
			{
				VHAL_LOGI("remove surface id=%d", id);
				(void)p_layout_mng_->DeleteSurfaceToLayer(id);
			}
		}
	}
	void NotifySurfaceConfigure(const int32_t surfaceId, const int32_t width, const int32_t height) const noexcept override
	{
		const int32_t ret{p_layout_mng_->ConfigureSurface(surfaceId, width, height)};
		if( VHAL_SUCCESS != ret )
		{
			VHAL_LOGE("ConfigureSurface error id=%d, width=%d, height=%d", surfaceId, width, height);
		}
	}

private:
	CVhalLayoutManager* p_layout_mng_;
};



/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalLayoutManager::CVhalLayoutManager(void)
	:p_layout_file_(nullptr)
	,p_ivi_controller_(nullptr)
	,p_ivi_listener_(nullptr)
	,p_main_(nullptr)
	,p_layoutManager_listener_(nullptr)
	,screen_build_type_(ScreenBuildType::SCREEN_BUILD_TYPE_INVALID)
	,connected_rse_(VHAL_CONNECTED_RSE_INVALID)
	,movie_x_(0)
	,movie_y_(0)
	,movie_width_(0)
	,movie_height_(0)
	,front_disp_blinder_init_(true)
	,hud_disp_blinder_init_(true)	/* HUDディスプレイMUTE初期表示状態の初期値はtrue */
{
	VHAL_LOGV("CVhalLayoutManager is created. this=%p", this);
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalLayoutManager::~CVhalLayoutManager(void)
{
	VHAL_LOGV("CVhalLayoutManager is deleted. this=%p", this);
	Finalize();
}

/*****************************************************************************
 処理概要：	初期化処理（内部リソースの確保）
 引数    ：	CVhalMainControl	*p_main		(i)メインコントロールインスタンスポインタ
 戻り値  ：	処理結果
           		VHAL_ERR_ILM_API	ilm APIエラー
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::Initialize(CVhalMainControl * const p_main)
{
	int32_t ret;

	p_main_ = p_main;
	screen_build_type_ = ScreenBuildType::SCREEN_BUILD_TYPE_INVALID;

	/* 映像出力サイズ管理に使用する映像パス一覧を登録 */
	(void)video_setting_.Initialize();
	for ( auto it_info=videopath_info_list_.begin(); it_info!=videopath_info_list_.end(); ++it_info )
	{
		if ( false == it_info->first.empty() )
		{
			const int32_t ret_regist{video_setting_.RegisterVideoPath(it_info->first, it_info->second.GetOutputTarget())};
			if ( VHAL_SUCCESS != ret_regist )
			{
				VHAL_LOGE("RegisterVideoPath error ret=%d path=%s", ret_regist, it_info->first.c_str());
			}
		}
	}

	p_ivi_listener_ = std::make_unique<CVhalIviEventListener>(this);

	p_ivi_controller_ = std::make_unique<CVhalIviController>();
	ret = p_ivi_controller_->Initialize(p_main, p_ivi_listener_.get());
	if (0 > ret)
	{
		VHAL_LOGE("IviController Initialize error. ret=%d", ret);
		return ret;
	}

	p_layout_file_ = std::make_unique<CVhalLayoutConfigFile>();

	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	終了処理（内部リソースの解放）
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
void CVhalLayoutManager::Finalize(void) noexcept
{

	if( nullptr != p_layout_file_ ) {
		p_layout_file_ = nullptr;
	}

	if( nullptr != p_ivi_controller_ ) {
		p_ivi_controller_ = nullptr;
	}
	if( nullptr != p_ivi_listener_ ) {
		p_ivi_listener_ = nullptr;
	}

	video_setting_.Finalize();
}

/*****************************************************************************
 処理概要：	iviレイアウトの初期化
 引数    ：	const std::string&	filePath	(i)レイアウトファイルパス
 戻り値  ：	処理結果
           		VHAL_ERR_NOT_INITIALIZED	未初期化エラー
           		VHAL_SUCCESS				正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::InitLayout(const std::string& filePath)
{
	int32_t ret{VHAL_SUCCESS};

	ret = ReadLayoutFile(filePath);
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("ReadLayoutFile error. ret=%d", ret);
		return ret;
	}

	/* 現在存在するスクリーンに対してレイアウト設定を実施 */
	std::vector<uint32_t> screen_ids{};
	screen_ids = p_ivi_controller_->GetScreenIDs();
	for (auto itr_id = screen_ids.begin(); itr_id != screen_ids.end(); ++itr_id)
	{
		/* スクリーンサイズを取得 */
		int32_t width{0};
		int32_t height{0};
		ret = p_ivi_controller_->GetScreenResolution(UI32ToI32(*itr_id), width, height);
		if ( VHAL_SUCCESS == ret )
		{
			ret = SetLayoutScreen(UI32ToI32(*itr_id), width, height);
		}
		
		if (VHAL_SUCCESS != ret)
		{
			/* 初期化スキップするだけ。エラーとはしない。 */
			VHAL_LOGI("screen(%u) layout is not initialized.", *itr_id);
		}
	}

	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	レイアウトファイルの読み込み
 引数    ：	const std::string&	filePath	(i)レイアウトファイルパス
 戻り値  ：	処理結果
           		VHAL_ERR_NOT_INITIALIZED	未初期化エラー
           		VHAL_SUCCESS				正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::ReadLayoutFile(const std::string& filePath)
{
	if( nullptr == p_layout_file_ ) {
		return VHAL_ERR_NOT_INITIALIZED;
	}

	const int32_t ret{p_layout_file_->ReadFile(filePath)};

	return ret;
}

/*****************************************************************************
 処理概要：	スクリーンのレイアウト構築
 引数    ：	int32_t	screenId	(i)スクリーンID
 引数    ：	int32_t	width		(i)ディスプレイ解像度(幅)
 引数    ：	int32_t	height		(i)ディスプレイ解像度(高さ)
 戻り値  ：	処理結果
           		VHAL_ERR_NOT_INITIALIZED	未初期化エラー
           		VHAL_ERR_LAYOUT_INFO		レイアウト情報不正
           		VHAL_ERR_ILM_API			ilm APIエラー
           		VHAL_SUCCESS				正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::SetLayoutScreen(const int32_t screenId, const int32_t width, const int32_t height)
{
	int32_t ret{VHAL_SUCCESS};

	if( nullptr == p_layout_file_ )
	{
		ret = VHAL_ERR_NOT_INITIALIZED;
	}
	else
	{
		/* スクリーンのレイアウト構築済みかをチェック */
		const auto itr_screen_id = std::find(screen_id_list_.begin(), screen_id_list_.end(), screenId);
		if( itr_screen_id != screen_id_list_.end() )
		{
			/* 既に登録済なら処理スキップ */
			VHAL_LOGI("already set layout screen id=%d", screenId);
		}
		else
		{
			VHAL_LOGI("set layout screen id=%d", screenId);

			/* レイヤ構成ファイルのスクリーン情報取得 */
			CVhalLayoutScreenProperty screen_prop{};
			ret = p_layout_file_->GetScreenProperty(screenId, screen_prop);
			if ( VHAL_SUCCESS != ret )
			{
				VHAL_LOGE("error not found screen id=%d", screenId);
				ret = VHAL_ERR_LAYOUT_INFO;
			}
			else
			{
				/* 生成するレイヤをリスト化 */
				std::vector<CVhalIviController::LayerInfo> layer_list{};
				std::vector<int32_t> layer_ids{};
				(void)screen_prop.GetLayerIds(layer_ids);
				std::vector<int32_t>::const_iterator itlayer{layer_ids.cbegin()};
				const std::vector<int32_t>::const_iterator itlayerend{layer_ids.cend()};
				for( ; itlayer!=itlayerend; ++itlayer)
				{
					CVhalLayoutLayerProperty layer_prop{};
					(void)p_layout_file_->GetLayerProperty(*itlayer, layer_prop);
					const int32_t layer_id{layer_prop.GetIviId()};
					std::string layer_name{};
					(void)layer_prop.GetName(layer_name);
					VHAL_LOGD("create layer(%d)  name:%s", layer_id, layer_name.c_str());
	
					const CVhalIviController::LayerInfo layer_info{layer_id, width, height};
					layer_list.push_back(layer_info);
				}

				/* レイヤをまとめて生成 */
				(void)p_ivi_controller_->CreateLayers(screenId, layer_list);

				/* 内部スクリーンリストに登録 */
				screen_id_list_.push_back(screenId);

				/* スクリーン有効状態変化通知 */
				p_layoutManager_listener_->NotifyScreenAvailable(screenId, true, width, height);
			}
		}
	}

	return ret;
}

/*****************************************************************************
 処理概要：	レイヤ名から映像出力先を取得
 引数    ：	const std::string& layer_name		(i)レイヤ名
         ：	VideoOutputTarget& output_target	(o)映像出力先
 戻り値  ：	処理結果
           		VHAL_SUCCESS		正常終了
				VHAL_ERR_PARAM		パラメータ不正
*****************************************************************************/
int32_t CVhalLayoutManager::GetOutputTargetFromLayerName(const std::string& layer_name, VideoOutputTarget& output_target) const
{
	int32_t ret{VHAL_SUCCESS};
	
	if( 0 == layer_name.compare(kLayerNameVideoFront) )
	{
		output_target = VIDEO_OUTPUT_TARGET_FRONT;
	}
	else if( 0 == layer_name.compare(kLayerNameCamera) )
	{
		output_target = VIDEO_OUTPUT_TARGET_CAMERA;
	}
	else if( 0 == layer_name.compare(kLayerNameVideoRear) )
	{
		output_target = VIDEO_OUTPUT_TARGET_REAR;
	}
	else if( ( 0 == layer_name.compare(kLayerNmaeVideoIClusterProjection) ) ||
			 ( 0 == layer_name.compare(kLayerNmaeVideoIClusterMap) ) )
	{
		output_target = VIDEO_OUTPUT_TARGET_IC;
	}
	else
	{
		ret = VHAL_ERR_PARAM;
	}
	
	return ret;
}

/*****************************************************************************
 処理概要：	サーフェスをレイヤーへ登録
 引数    ：	int32_t	surfaceId	(i)サーフェスID
 戻り値  ：	処理結果
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::AddSurfaceToLayer(const int32_t surfaceId)
{
	VHAL_LOGI("surfaceID=%d", surfaceId);

	/* 登録すべきレイヤを取得 */
	CVhalLayoutSurfaceProperty surface_prop{};
	int32_t ret{p_layout_file_->GetSurfaceProperty(surfaceId, surface_prop)};
	if( VHAL_SUCCESS != ret ) {
		VHAL_LOGW("surfaceId(%d) is unknown.", surfaceId);
		return ret;
	}

	const int32_t layer_id{surface_prop.GetParentLayerIviId()};

	/* レイヤにサーフェス登録 */
	(void)p_ivi_controller_->AddSurfaceToLayer(surfaceId, layer_id);

	/* 映像パス設定可否判定状態通知（映像パスに紐づいたサーフェスの時のみ通知、それ以外のサーフェスは通知不要） */
	std::string surface_name{};
	std::string videopath_name{};
	(void)surface_prop.GetName(surface_name);
	VideoOutputTarget output_target{VIDEO_OUTPUT_TARGET_NONE};
	ret = GetVideoPathFromSurfaceName(surface_name, videopath_name, output_target);	/* サーフェス名を映像パス名に変換 */
	if( VHAL_SUCCESS == ret )
	{
		/* サーフェス名から映像パス名に変換出来たら通知を出す */
		std::vector<std::string> videopath_name_list{};
		videopath_name_list.push_back(videopath_name);
		p_layoutManager_listener_->NotifyVpathAvailable(videopath_name_list, output_target, true);
		
		/* サーフェスが再生成された場合、サーフェスのオリジナルサイズが変わっている可能性があるため、保持しているオリジナルサイズをクリアする */
		VideoOutputTarget output_target_from_layer_name{VIDEO_OUTPUT_TARGET_NONE};
		CVhalLayoutLayerProperty layer_prop{};
		std::string layer_name{};
		(void)p_layout_file_->GetLayerProperty(layer_id, layer_prop);
		(void)layer_prop.GetName(layer_name);

		ret = GetOutputTargetFromLayerName(layer_name, output_target_from_layer_name);
		if( VHAL_SUCCESS == ret )
		{
			ret = video_setting_.ClearVideoInputOriginalSize(videopath_name, output_target_from_layer_name);
			if( VHAL_SUCCESS != ret )
			{
				VHAL_LOGE("ClearVideoInputOriginalSize error ret=%d path=%s, output_target=0x%x", ret, videopath_name.c_str(), output_target_from_layer_name);
			}
		}
	}

	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	サーフェスをレイヤーから削除
 引数    ：	int32_t	surfaceId	(i)サーフェスID
 戻り値  ：	処理結果
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::DeleteSurfaceToLayer(const int32_t surfaceId)
{
	VHAL_LOGI("surfaceID=%d", surfaceId);

	/* サーフェスプロパティ取得 */
	CVhalLayoutSurfaceProperty surface_prop{};
	int32_t ret{p_layout_file_->GetSurfaceProperty(surfaceId, surface_prop)};
	if( VHAL_SUCCESS != ret ) {
		VHAL_LOGW("surfaceId(%d) is unknown.", surfaceId);
		return ret;
	}

	/* 映像パス設定可否判定状態通知（映像パスに紐づいたサーフェスの時のみ通知、それ以外のサーフェスは通知不要） */
	std::string surface_name{};
	std::string videopath_name{};
	(void)surface_prop.GetName(surface_name);
	VideoOutputTarget output_target{VIDEO_OUTPUT_TARGET_NONE};
	ret = GetVideoPathFromSurfaceName(surface_name, videopath_name, output_target);	/* サーフェス名を映像パス名に変換 */
	if( VHAL_SUCCESS == ret )
	{
		/* サーフェス名から映像パス名に変換出来たら通知を出す */
		std::vector<std::string> videopath_name_list{};
		videopath_name_list.push_back(videopath_name);
		p_layoutManager_listener_->NotifyVpathAvailable(videopath_name_list, output_target, false);
	}

	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	サーフェスのconfigure処理の実施
 引数    ：	int32_t	surfaceId	(i)サーフェスID
           	int32_t	width		(i)幅
           	int32_t	height		(i)高さ
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO		レイアウト情報不正
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::ConfigureSurface(const int32_t surfaceId, const int32_t width, const int32_t height)
{
	int32_t result{VHAL_SUCCESS};
	CVhalLayoutVideoSetting::VideoRectangleData out_src{0, 0, width, height, false};
	CVhalLayoutVideoSetting::VideoRectangleData out_dest{0, 0, width, height, false};

	VHAL_LOGI("surfaceid=%d, width=%d, height=%d", surfaceId, width, height);

	/* サイズゼロは処理をスキップ */
	if( (0 == width) || (0 == height) )
	{
		VHAL_LOGI("ignore configure size (%dx%d) id=%d", width, height, surfaceId);
		result = VHAL_SUCCESS;
	}
	else
	{
		/* サーフェスIDからサーフェス名を取得 */
		CVhalLayoutSurfaceProperty surface_prop{};
		int32_t ret{p_layout_file_->GetSurfaceProperty(surfaceId, surface_prop)};
		if( VHAL_SUCCESS != ret )
		{
			result = VHAL_ERR_LAYOUT_INFO;
		}
		else
		{
			std::string surface_name{};
			(void)surface_prop.GetName(surface_name);

			if( (0 == surface_name.compare(kSurfaceNameMuteFrontDisplay)) || (0 == surface_name.compare(kSurfaceNameMuteFrontVideo)) ||
				(0 == surface_name.compare(kSurfaceNameMuteRearDisplay))  || (0 == surface_name.compare(kSurfaceNameMuteRearVideo))  ||
				(0 == surface_name.compare(kSurfaceNameMuteCamera))       || (0 == surface_name.compare(kSurfaceNameMuteIcDisplay))  ||
				(0 == surface_name.compare(kSurfaceNameVideoFrontBg))     || (0 == surface_name.compare(kSurfaceNameVideoRearBg))    ||
				(0 == surface_name.compare(kSurfaceNameMuteFrontVideoSync)) || (0 == surface_name.compare(kSurfaceNameMuteHudDisplay)) )
	    	{
				result = ConfigureBlinder(surface_name, surfaceId, width, height);
			}
			else
			{
				if( 0 == surface_name.compare(kSurfaceNameMovie) )
				{
					result = ConfigureMovie(surface_name, surfaceId, width, height);
				}
				else
				{
					/* サーフェス初期表示設定を取得 */
					CVhalLayoutSurfaceProperty::InitialVisibility init_visibility{surface_prop.GetInitialVisibility()};

					/* サーフェス名から映像パスを取得 */
					VideoOutputTarget output_target_from_surface_name{VIDEO_OUTPUT_TARGET_NONE};
					std::string video_path{};
					ret = GetVideoPathFromSurfaceName(surface_name, video_path, output_target_from_surface_name);
					if( VHAL_SUCCESS == ret )
					{
						/* 以降は映像パスに紐づいているサーフェスに対する処理（表示判定、出力サイズ取得） */

						/* サーフェスの親レイヤ取得 */
						int32_t parent_layer{0};
						CVhalLayoutLayerProperty layer_prop{};
						std::string layer_name{};
						parent_layer = surface_prop.GetParentLayerIviId();
						(void)p_layout_file_->GetLayerProperty(parent_layer, layer_prop);
						(void)layer_prop.GetName(layer_name);

						/* どの映像レイヤに属しているサーフェスかを特定 */
						VideoOutputTarget output_target{VIDEO_OUTPUT_TARGET_NONE};
						ret = GetOutputTargetFromLayerName(layer_name, output_target);
						if( VHAL_SUCCESS == ret )
						{
							bool current_visibility{false};
							ret = video_setting_.GetVideoVisibility(video_path, output_target, current_visibility);
							if (VHAL_SUCCESS != ret)
							{
								VHAL_LOGE("GetVideoVisibility error ret=%d", ret);
							}
							VHAL_LOGD("video_path=%s, output_target=0x%x, current_visibility=%d", video_path.c_str(), output_target, current_visibility);

							if( 0 == layer_name.compare(kLayerNameVideoFront) )
							{
								/* 現在の前席映像パスと一致かつ、可視状態であれば表示。そうでないなら初期表示設定に依存 */
								if( (video_path == video_front_path_current_) && (true == current_visibility) )
								{
									/* 可視状態に更新する */
									init_visibility = CVhalLayoutSurfaceProperty::InitialVisibility::kVisible;
								}
							}
							else if( 0 == layer_name.compare(kLayerNameCamera) )
							{
								/* 現在のカメラパスと一致するなら表示（可視状態はVideoHAL内で制御）。そうでないなら初期表示設定に依存 */
								if( 0 == video_path.compare(camera_path_current_) )
								{
									init_visibility = CVhalLayoutSurfaceProperty::InitialVisibility::kVisible;
								}
							}
							else if( 0 == layer_name.compare(kLayerNameVideoRear) )
							{
								/* 現在の後席映像パスと一致かつ、可視状態であれば表示。そうでないなら初期表示設定に依存 */
								if( (video_path == video_rear_path_current_) && (true == current_visibility) )
								{
									/* 可視状態に更新する */
									init_visibility = CVhalLayoutSurfaceProperty::InitialVisibility::kVisible;
								}
							}
							else if( ( 0 == layer_name.compare(kLayerNmaeVideoIClusterProjection) ) ||
									 ( 0 == layer_name.compare(kLayerNmaeVideoIClusterMap) ) )
							{
								/* 現在のInstrument Cluster 映像パスと一致かつ、可視状態であれば表示。そうでないなら初期表示設定に依存 */
								if( (video_path == video_icluster_path_current_) && (true == current_visibility) )
								{
									init_visibility = CVhalLayoutSurfaceProperty::InitialVisibility::kVisible;
								}
							}
							else
							{
								/* 処理なし */
							}

							/* 映像パスの出力サイズ取得（その他は入力サイズをそのまま出力サイズとする） */

							if( VIDEO_OUTPUT_TARGET_NONE != output_target )
							{
								CVhalLayoutVideoSetting::VideoRectangleData in_src{0, 0, width, height, false};
								CVhalLayoutVideoSetting::VideoRectangleData in_dest{0, 0, width, height, false};

								/* 映像入力サイズ内部データ更新（オリジナルサイズ設定） */
								ret = video_setting_.SetVideoInputOriginalSize(video_path, output_target, in_src);
								if( VHAL_SUCCESS != ret )
								{
									VHAL_LOGE("SetVideoInputOriginalSize error ret=%d path=%s, output_target=0x%x", ret, video_path.c_str(), output_target);
								}

								CVhalLayoutVideoSetting::VideoRectangleData output_video{};
								ret = video_setting_.GetVideoOutputSize(video_path, output_target, output_video);
								if( VHAL_SUCCESS == ret )
								{
									/* 出力矩形が設定済みなので反映 */
									if( true == output_video.set_ )
									{
										in_dest = output_video;
										out_dest = output_video;
									}
									else
									{
										if( VIDEO_OUTPUT_TARGET_CAMERA == output_target )
										{
											/* カメラの場合 スクリーンサイズを反映 */
											constexpr int32_t screen_id{0};
											ret = GetScreenSize(screen_id, in_dest.width_, in_dest.height_);
											if( VHAL_SUCCESS != ret )
											{
												/* 入力短形を反映させる */
												VHAL_LOGE("GetScreenSize error. ret=%d", ret);
											}
										}

										/* 出力矩形が未設定なので、入力短形を反映 */
										ret = video_setting_.SetVideoOutputSize(video_path, output_target, in_dest);
										if( VHAL_SUCCESS != ret )
										{
											VHAL_LOGE("SetVideoOutputSize error ret=%d path=%s, output_target=0x%x", ret, video_path.c_str(), output_target);
										}

										/* カメラの場合 映像パス切替時に映像表示より先にMUTE表示するため先にサイズを合わせておく */
										if( VIDEO_OUTPUT_TARGET_CAMERA == output_target )
										{
											/* カメラMUTEサイズをカメラ映像サイズに追従させる */
											ret = SetVideoBlindSurfaceRectangle(kSurfaceNameMuteCamera, video_path, VIDEO_OUTPUT_TARGET_CAMERA);
											if( VHAL_SUCCESS != ret )
											{
												VHAL_LOGW("SetVideoBlindSurfaceRectangle error ret=%d, mute_surface_name=%s", ret, kSurfaceNameMuteCamera.c_str());
											}
										}

										if( VIDEO_OUTPUT_TARGET_IC == output_target )
										{
											ret = SetVideoBlindSurfaceRectangle(kSurfaceNameMuteIcDisplay, video_path, VIDEO_OUTPUT_TARGET_IC);
											if( VHAL_SUCCESS != ret )
											{
												VHAL_LOGW("SetVideoBlindSurfaceRectangle error ret=%d, mute_surface_name=%s", ret, kSurfaceNameMuteIcDisplay.c_str());
											}
										}
									}
								}

								/* クリッピング有効の場合、ソース短形にクリッピングサイズを使用 */
								bool clipping_enable{false};
								struct CVhalLayoutVideoSetting::VideoRectangleData input_clipping{};
								ret = GetSourceRectangleCommon(clipping_enable, input_clipping.x_, input_clipping.y_, input_clipping.width_, input_clipping.height_, video_path, output_target);
								if((VHAL_SUCCESS == ret) && clipping_enable)
								{
									in_src = input_clipping;
								}
								/* ワイド設定の短形取得 */
								ret = video_setting_.GetVideoWideRectangle(video_path, output_target, in_src, in_dest, out_src, out_dest);
								if( VHAL_SUCCESS != ret )
								{
									VHAL_LOGE("GetVideoWideRectangle error. ret=%d, path=%s, output_target=0x%x", ret, video_path.c_str(), output_target);
								}

								/* 映像可視状態の内部データ更新 */
								if( CVhalLayoutSurfaceProperty::InitialVisibility::kVisible == init_visibility )
								{
									ret = video_setting_.SetVideoVisibility(video_path, output_target, true);
								}
								else
								{
									ret = video_setting_.SetVideoVisibility(video_path, output_target, false);
								}
								if( VHAL_SUCCESS != ret )
								{
									VHAL_LOGE("SetVideoVisibility error ret=%d path=%s, output_target=0x%x", ret, video_path.c_str(), output_target);
								}
							}
						}
					}

					/* 入力矩形設定 */
					(void)p_ivi_controller_->SetSurfaceSourceRectangle(surfaceId, out_src.x_, out_src.y_, out_src.width_, out_src.height_);

					/* 出力矩形設定 */
					(void)p_ivi_controller_->SetSurfaceDestinationRectangle(surfaceId, out_dest.x_, out_dest.y_, out_dest.width_, out_dest.height_);

					/* 表示設定 */
					if ( CVhalLayoutSurfaceProperty::InitialVisibility::kVisible == init_visibility )
					{
						(void)p_ivi_controller_->SetSurfaceVisibility(surfaceId, true);
						VHAL_LOGI("id=%d name=%s visible=1 src(%d,%d,%d,%d) dest(%d,%d,%d,%d)", surfaceId, surface_name.c_str(), out_src.x_, out_src.y_, out_src.width_, out_src.height_, out_dest.x_, out_dest.y_, out_dest.width_, out_dest.height_);
					}
					else if ( CVhalLayoutSurfaceProperty::InitialVisibility::kInvisible == init_visibility )
					{
						(void)p_ivi_controller_->SetSurfaceVisibility(surfaceId, false);
						VHAL_LOGI("id=%d name=%s visible=0 src(%d,%d,%d,%d) dest(%d,%d,%d,%d)", surfaceId, surface_name.c_str(), out_src.x_, out_src.y_, out_src.width_, out_src.height_, out_dest.x_, out_dest.y_, out_dest.width_, out_dest.height_);
					}
					else
					{
						/* 表示設定しない */
						VHAL_LOGI("id=%d name=%s visible=none src(%d,%d,%d,%d) dest(%d,%d,%d,%d)", surfaceId, surface_name.c_str(), out_src.x_, out_src.y_, out_src.width_, out_src.height_, out_dest.x_, out_dest.y_, out_dest.width_, out_dest.height_);
					}

					(void)p_ivi_controller_->CommitChanges();
				}
			}
		}
	}

	return result;
}

/*****************************************************************************
 処理概要：	有効な映像path判定（共通）
 引数    ：	const std::string&		video_path_prev		(i)変更前vpath文字列
         ：	const std::string&		video_path_next		(i)変更後vpath文字列
         ：	const VideoOutputTarget output_target		(i)映像出力先
         ：	bool 					&already_path		(o)映像パス変更判定
 戻り値  ：	処理結果
           		false		無効な映像path
           		true		有効な映像path
*****************************************************************************/
bool CVhalLayoutManager::IsValidVideoPathCommon(const std::string& video_path_prev, const std::string& video_path_next, const VideoOutputTarget output_target, bool &already_path) const
{
	bool is_valid{true};
	already_path = false;

	/* 映像パス変更判定 */
	if (0 == video_path_prev.compare(video_path_next))
	{
		/* 映像パスに変化が無い場合はスキップ */
		already_path = true;
	}
	else
	{
		/* 映像パスのチェック（設定可能か） */
		const auto iter = videopath_info_list_.find(video_path_next);
		if( iter == videopath_info_list_.end() )
		{
			VHAL_LOGD("invalid video path=%s", video_path_next.c_str());
			is_valid = false;
		}

		/* 映像パスのチェック（出力先） */
		if( 0U == (iter->second.GetOutputTarget() & output_target) )
		{
			VHAL_LOGV("invalid output_target=0x%x, video path=%s", output_target, video_path_next.c_str());
			is_valid = false;
		}
	}

	return is_valid;
}

/*****************************************************************************
 処理概要：	前席 有効な映像path判定
 引数    ：	const std::string&	video_path		(i)映像vpath文字列
         ：	bool 				&already_path	(o)映像パス変更判定
 戻り値  ：	処理結果
           		false		無効な映像path
           		true		有効な映像path
*****************************************************************************/
bool CVhalLayoutManager::IsValidVideoFrontPath(const std::string& video_path, bool &already_path) const
{
	bool is_valid{false};
	is_valid = IsValidVideoPathCommon(video_front_path_current_, video_path, VIDEO_OUTPUT_TARGET_FRONT, already_path);
	return is_valid;
}

/*****************************************************************************
 処理概要：	現在の前席vpathの取得
 引数    ：	std::string&	path	(o)vpath文字列
 戻り値  ：	処理結果
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::GetFrontPath(std::string& path) const
{
	path = video_front_path_current_;
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	現在の前席vpathの設定
 引数    ：	const std::string&	path	(i)vpath文字列
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM			パラメータ不正
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::SetFrontPath(const std::string& path)
{
	int32_t ret{VHAL_SUCCESS};

	/* 前席映像pathの設定 */
	ret = SetVideoPathCommon(video_front_path_current_, path, kSurfaceNameMuteFrontVideo, kSurfaceNameVideoFrontBg, VIDEO_OUTPUT_TARGET_FRONT, true);
	if (VHAL_SUCCESS != ret)
	{
		/* 映像パス設定 失敗 */
		VHAL_LOGE("SetVideoPathCommon error. ret=%d vpath=%s", ret, path.c_str());
		return ret;
	}

	/* 映像背景サーフェス表示設定（映像表示/不透過状態により、映像背景の表示状態を連動） */
	ret = SetVideoBlindSurfaceVisibility(kSurfaceNameVideoFrontBg, video_front_path_current_, path, VIDEO_OUTPUT_TARGET_FRONT);
	if (VHAL_SUCCESS != ret)
	{
		/* 映像背景サーフェス表示設定 失敗 */
		VHAL_LOGE("SetVideoBlindSurfaceVisibility error. ret=%d vpath=%s", ret, path.c_str());
	}

	/* 映像MUTE解除 */
	ret = SetVideoMuteOffCommon(video_front_path_current_, path, kSurfaceNameMuteFrontVideo);
	if (VHAL_SUCCESS != ret)
	{
		/* 映像MUTE解除 失敗 */
		VHAL_LOGE("SetVideoMuteOffCommon error. ret=%d vpath=%s", ret, path.c_str());
	}

	/* 映像パス設定 成功 */
	video_front_path_current_ = path;
	return ret;
}

/*****************************************************************************
 処理概要：	前席映像のソース矩形の取得
 引数    ：	bool				&clipping_enable	(o)クリッピング有効フラグ
           	int32_t				&x					(o)x座標
           	int32_t				&y					(o)y座標
           	int32_t				&w					(o)幅
           	int32_t				&h					(o)高さ
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO		レイアウト情報不正
           		VHAL_SUCCESS				正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::GetFrontSourceRectangle(bool &clipping_enable, int32_t &x, int32_t &y, int32_t &w, int32_t &h) const
{
	/* 前席映像pathの入力矩形取得 */
	const int32_t ret{GetSourceRectangleCommon(clipping_enable, x, y, w, h, video_front_path_current_, VIDEO_OUTPUT_TARGET_FRONT)};
	return ret;
}

/*****************************************************************************
 処理概要：	前席映像のソース矩形の設定
 引数    ：	bool				clipping_enable		(i)クリッピング有効フラグ
           	int32_t				x					(i)x座標
           	int32_t				y					(i)y座標
           	int32_t				w					(i)幅
           	int32_t				h					(i)高さ
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO		レイアウト情報不正
           		VHAL_SUCCESS				正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::SetFrontSourceRectangle(const bool clipping_enable, const int32_t x, const int32_t y, const int32_t w, const int32_t h)
{
	/* 前席映像pathの入力矩形設定 */
	const int32_t ret{SetSourceRectangleCommon(clipping_enable, x, y, w, h, video_front_path_current_, VIDEO_OUTPUT_TARGET_FRONT)};
	return ret;
}

/*****************************************************************************
 処理概要：	前席映像のデスティネーション矩形の設定
 引数    ：	int32_t				x		(i)x座標
           	int32_t				y		(i)y座標
           	int32_t				w		(i)幅
           	int32_t				h		(i)高さ
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::SetFrontDestRectangle(const int32_t x, const int32_t y, const int32_t w, const int32_t h)
{
	int32_t ret{VHAL_SUCCESS};

	/* 前席映像pathの矩形設定 */
	ret = SetDestRectangleCommon(x, y, w, h, video_front_path_current_, kSurfaceNameMuteFrontVideo, kSurfaceNameVideoFrontBg, VIDEO_OUTPUT_TARGET_FRONT);
	return ret;
}

/*****************************************************************************
 処理概要：	前席映像の可視状態取得
 引数    ：	bool&		visibility		(o)可視状態
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::GetFrontVisibility(bool& visibility) const
{
	int32_t ret{VHAL_SUCCESS};

	/* 前席映像pathの可視状態取得 */
	ret = GetVisibilityCommon(visibility, video_front_path_current_, VIDEO_OUTPUT_TARGET_FRONT);

	return ret;
}

/*****************************************************************************
 処理概要：	前席映像の可視状態設定
 引数    ：	bool		visibility		(i)可視状態
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::SetFrontVisibility(const bool visibility)
{
	int32_t ret{VHAL_SUCCESS};

	/* 前席映像pathの可視状態設定 */
	ret = SetVisibilityCommon(visibility, video_front_path_current_, VIDEO_OUTPUT_TARGET_FRONT);
	if (VHAL_SUCCESS != ret)
	{
		/* 前席映像pathの可視状態設定 失敗 */
		VHAL_LOGE("SetVisibilityCommon error. ret=%d vpath=%s", ret, video_front_path_current_.c_str());
		return ret;
	}

	/* 映像背景サーフェス表示設定（映像表示/不透過状態により、映像背景の表示状態を連動） */
	ret = SetVideoBlindSurfaceVisibility(kSurfaceNameVideoFrontBg, "", video_front_path_current_, VIDEO_OUTPUT_TARGET_FRONT);
	if (VHAL_SUCCESS != ret)
	{
		/* 映像背景サーフェス表示設定 失敗 */
		VHAL_LOGE("SetVideoBlindSurfaceVisibility error. ret=%d vpath=%s", ret, video_front_path_current_.c_str());
	}

	return ret;
}

/*****************************************************************************
 処理概要：	前席映像の不透明度設定
 引数    ：	uint32_t	opacity		(i)不透明度(0-100)
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::SetFrontOpacity(const uint32_t opacity)
{
	int32_t ret{VHAL_SUCCESS};

	/* 前席映像の不透明度設定 */
	ret = SetOpacityCommon(opacity, video_front_path_current_, VIDEO_OUTPUT_TARGET_FRONT);
	if (VHAL_SUCCESS != ret)
	{
		/* 前席映像の不透明度設定 失敗 */
		VHAL_LOGE("SetOpacityCommon error. ret=%d vpath=%s", ret, video_front_path_current_.c_str());
		return ret;
	}

	/* 映像背景サーフェス表示設定（映像表示/不透過状態により、映像背景の表示状態を連動） */
	ret = SetVideoBlindSurfaceVisibility(kSurfaceNameVideoFrontBg, "", video_front_path_current_, VIDEO_OUTPUT_TARGET_FRONT);
	if (VHAL_SUCCESS != ret)
	{
		/* 映像背景サーフェス表示設定 失敗 */
		VHAL_LOGE("SetVideoBlindSurfaceVisibility error. ret=%d vpath=%s", ret, video_front_path_current_.c_str());
	}

	return ret;
}

/*****************************************************************************
 処理概要：	前席映像のワイド取得
 引数    ：	uint32_t	wide		(o)ワイド設定(0-2)
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::GetFrontWideMode(uint32_t &wide_mode) const
{
	int32_t ret{VHAL_SUCCESS};

	/* 映像のワイド取得 */
	ret = video_setting_.GetVideoWideMode(video_front_path_current_, VIDEO_OUTPUT_TARGET_FRONT, wide_mode);
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("GetVideoWideMode error. ret=%d", ret);
	}

	return ret;
}

/*****************************************************************************
 処理概要：	前席映像のワイド設定
 引数    ：	uint32_t	wide		(i)ワイド設定(0-2)
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::SetFrontWideMode(const uint32_t wide_mode)
{
	int32_t ret{VHAL_SUCCESS};

	/* 映像のワイド設定 */
	ret = SetWideModeCommon(wide_mode, video_front_path_current_, VIDEO_OUTPUT_TARGET_FRONT);
	return ret;
}

/*****************************************************************************
 処理概要：	後席 有効な映像path判定
 引数    ：	const std::string&	video_path		(i)映像vpath文字列
         ：	bool 				&already_path	(o)映像パス変更判定
 戻り値  ：	処理結果
           		false		無効な映像path
           		true		有効な映像path
*****************************************************************************/
bool CVhalLayoutManager::IsValidVideoRearPath(const std::string& video_path, bool &already_path) const
{
	bool is_valid{false};
	is_valid = IsValidVideoPathCommon(video_rear_path_current_, video_path, VIDEO_OUTPUT_TARGET_REAR, already_path);
	return is_valid;
}

/*****************************************************************************
 処理概要：	現在の後席vpathの取得
 引数    ：	std::string&	path	(o)vpath文字列
 戻り値  ：	処理結果
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::GetRearPath(std::string& path) const
{
	path = video_rear_path_current_;
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	現在の後席vpathの設定
 引数    ：	const std::string&	path	(i)vpath文字列
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM			パラメータ不正
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::SetRearPath(const std::string& path)
{
	int32_t ret{VHAL_SUCCESS};

	/* 後席映像pathの設定 */
	ret = SetVideoPathCommon(video_rear_path_current_, path, kSurfaceNameMuteRearVideo, kSurfaceNameVideoRearBg, VIDEO_OUTPUT_TARGET_REAR, true);
	if (VHAL_SUCCESS != ret)
	{
		/* 映像パス設定 失敗 */
		VHAL_LOGE("SetVideoPathCommon error. ret=%d vpath=%s", ret, path.c_str());
		return ret;
	}

	/* 映像背景サーフェス表示設定（映像表示/不透過状態により、映像背景の表示状態を連動） */
	ret = SetVideoBlindSurfaceVisibility(kSurfaceNameVideoRearBg, video_rear_path_current_, path, VIDEO_OUTPUT_TARGET_REAR);
	if (VHAL_SUCCESS != ret)
	{
		/* 映像背景サーフェス表示設定 失敗 */
		VHAL_LOGE("SetVideoBlindSurfaceVisibility error. ret=%d vpath=%s", ret, path.c_str());
	}

	/* 映像MUTE解除 */
	ret = SetVideoMuteOffCommon(video_rear_path_current_, path, kSurfaceNameMuteRearVideo);
	if (VHAL_SUCCESS != ret)
	{
		/* 映像MUTE解除 失敗 */
		VHAL_LOGE("SetVideoMuteOffCommon error. ret=%d vpath=%s", ret, path.c_str());
	}

	/* 映像パス設定 成功 */
	video_rear_path_current_ = path;
	return ret;
}

/*****************************************************************************
 処理概要：	後席映像のデスティネーション矩形の設定
 引数    ：	int32_t				x		(i)x座標
           	int32_t				y		(i)y座標
           	int32_t				w		(i)幅
           	int32_t				h		(i)高さ
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO		レイアウト情報不正
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::SetRearDestRectangle(const int32_t x, const int32_t y, const int32_t w, const int32_t h)
{
	int32_t ret{VHAL_SUCCESS};

	/* 後席映像pathの矩形設定 */
	ret = SetDestRectangleCommon(x, y, w, h, video_rear_path_current_, kSurfaceNameMuteRearVideo, kSurfaceNameVideoRearBg, VIDEO_OUTPUT_TARGET_REAR);
	return ret;
}

/*****************************************************************************
 処理概要：	後席映像の可視状態取得
 引数    ：	bool&		visibility		(o)可視状態
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::GetRearVisibility(bool& visibility) const
{
	int32_t ret{VHAL_SUCCESS};

	/* 後席映像pathの可視状態取得 */
	ret = GetVisibilityCommon(visibility, video_rear_path_current_, VIDEO_OUTPUT_TARGET_REAR);

	return ret;
}

/*****************************************************************************
 処理概要：	後席映像の可視状態設定
 引数    ：	bool		visibility		(i)可視状態
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::SetRearVisibility(const bool visibility)
{
	int32_t ret{VHAL_SUCCESS};

	/* 後席映像pathの可視状態設定 */
	ret = SetVisibilityCommon(visibility, video_rear_path_current_, VIDEO_OUTPUT_TARGET_REAR);
	if (VHAL_SUCCESS != ret)
	{
		/* 後席映像pathの可視状態設定 失敗 */
		VHAL_LOGE("SetVisibilityCommon error. ret=%d vpath=%s", ret, video_rear_path_current_.c_str());
		return ret;
	}

	/* 映像背景サーフェス表示設定（映像表示/不透過状態により、映像背景の表示状態を連動） */
	ret = SetVideoBlindSurfaceVisibility(kSurfaceNameVideoRearBg, "", video_rear_path_current_, VIDEO_OUTPUT_TARGET_REAR);
	if (VHAL_SUCCESS != ret)
	{
		/* 映像背景サーフェス表示設定 失敗 */
		VHAL_LOGE("SetVideoBlindSurfaceVisibility error. ret=%d vpath=%s", ret, video_rear_path_current_.c_str());
	}
	return ret;
}

/*****************************************************************************
 処理概要：	後席映像の不透明度設定
 引数    ：	uint32_t	opacity		(i)不透明度(0-100)
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::SetRearOpacity(const uint32_t opacity)
{
	int32_t ret{VHAL_SUCCESS};

	/* 後席映像pathの不透明度設定 */
	ret = SetOpacityCommon(opacity, video_rear_path_current_, VIDEO_OUTPUT_TARGET_REAR);
	if (VHAL_SUCCESS != ret)
	{
		/* 後席映像pathの不透明度設定 失敗 */
		VHAL_LOGE("SetOpacityCommon error. ret=%d vpath=%s", ret, video_rear_path_current_.c_str());
		return ret;
	}

	/* 映像背景サーフェス表示設定（映像表示/不透過状態により、映像背景の表示状態を連動） */
	ret = SetVideoBlindSurfaceVisibility(kSurfaceNameVideoRearBg, "", video_rear_path_current_, VIDEO_OUTPUT_TARGET_REAR);
	if (VHAL_SUCCESS != ret)
	{
		/* 映像背景サーフェス表示設定 失敗 */
		VHAL_LOGE("SetVideoBlindSurfaceVisibility error. ret=%d vpath=%s", ret, video_rear_path_current_.c_str());
	}
	return ret;
}

/*****************************************************************************
 処理概要：	後席映像のワイド取得
 引数    ：	uint32_t	wide		(o)ワイド設定(0-2)
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::GetRearWideMode(uint32_t &wide_mode) const
{
	int32_t ret{VHAL_SUCCESS};

	/* 映像のワイド取得 */
	ret = video_setting_.GetVideoWideMode(video_rear_path_current_, VIDEO_OUTPUT_TARGET_REAR, wide_mode);
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("GetVideoWideMode error. ret=%d", ret);
	}

	return ret;
}

/*****************************************************************************
 処理概要：	後席映像のワイド設定
 引数    ：	uint32_t	wide		(i)ワイド設定(0-2)
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::SetRearWideMode(const uint32_t wide_mode)
{
	int32_t ret{VHAL_SUCCESS};

	/* 映像のワイド設定 */
	ret = SetWideModeCommon(wide_mode, video_rear_path_current_, VIDEO_OUTPUT_TARGET_REAR);
	return ret;
}

/*****************************************************************************
 処理概要：	iviスクリーン情報の取得
 引数    ：	std::map<std::string, std::string>	&params	(i)パラメータmap
 戻り値  ：	JSON文字列
*****************************************************************************/
std::string CVhalLayoutManager::GetIviScreenInfo(std::map<std::string, std::string> &params) const
{
	int32_t ivi_id{0};
	std::string name{};
	CVhalLayoutScreenProperty screen_prop{};

	/* id優先で使う */
	const auto itr_id = params.find("ivi_id");
	if (itr_id != params.end())
	{
		ivi_id = std::stoi(itr_id->second);
		const int32_t ret{p_layout_file_->GetScreenProperty(ivi_id, screen_prop)};
		if (0 > ret)
		{
			VHAL_LOGE("GetScreenProperty(%d) error. ret=%d", ivi_id, ret);
			return std::string("");
		}
		(void)screen_prop.GetName(name);
	}
	else
	{
		const auto itr_name = params.find("name");
		if (itr_name == params.end())
		{
			VHAL_LOGE("Invalid params for screen");
			return std::string("");
		}
		const int32_t ret{p_layout_file_->GetScreenProperty(itr_name->second, screen_prop)};
		if (0 > ret)
		{
			VHAL_LOGE("GetScreenProperty(%s) error. ret=%d", itr_name->second.c_str(), ret);
			return std::string("");
		}
		ivi_id = screen_prop.GetIviId();
		name = itr_name->second;
	}

	return p_ivi_controller_->GetPropertiesOfScreen(ivi_id, name);
}

/*****************************************************************************
 処理概要：	iviレイヤー情報の取得
 引数    ：	std::map<std::string, std::string>	&params	(i)パラメータmap
 戻り値  ：	JSON文字列
*****************************************************************************/
std::string CVhalLayoutManager::GetIviLayerInfo(std::map<std::string, std::string> &params) const
{
	int32_t ret{VHAL_SUCCESS};
	CVhalLayoutLayerProperty layer_prop{};

	ret = GetLayerFromParams(params, layer_prop);
	if (0 > ret)
	{
		VHAL_LOGE("GetLayerFromParams() error. ret=%d", ret);
		return std::string("");
	}

	const int32_t ivi_id{layer_prop.GetIviId()};
	std::string name{};
	(void)layer_prop.GetName(name);

	return p_ivi_controller_->GetPropertiesOfLayer(ivi_id, name);
}

/*****************************************************************************
 処理概要：	iviサーフェス情報の取得
 引数    ：	std::map<std::string, std::string>	&params	(i)パラメータmap
 戻り値  ：	JSON文字列
*****************************************************************************/
std::string CVhalLayoutManager::GetIviSurfaceInfo(std::map<std::string, std::string> &params) const
{
	int32_t ivi_id{0};
	std::string name{};
	CVhalLayoutSurfaceProperty surface_prop{};

	/* id優先で使う */
	const auto itr_id = params.find("ivi_id");
	if (itr_id != params.end())
	{
		ivi_id = std::stoi(itr_id->second);
		const int32_t ret{p_layout_file_->GetSurfaceProperty(ivi_id, surface_prop)};
		if (0 > ret)
		{
			VHAL_LOGE("GetSurfaceProperty(%d) error. ret=%d", ivi_id, ret);
			return std::string("");
		}
		(void)surface_prop.GetName(name);
	}
	else
	{
		const auto itr_name = params.find("name");
		if (itr_name == params.end())
		{
			VHAL_LOGE("Invalid params for surface");
			return std::string("");
		}
		const int32_t ret{p_layout_file_->GetSurfaceProperty(itr_name->second, surface_prop)};
		if (0 > ret)
		{
			VHAL_LOGE("GetSurfaceProperty(%s) error. ret=%d", itr_name->second.c_str(), ret);
			return std::string("");
		}
		ivi_id = surface_prop.GetIviId();
		name = itr_name->second;
	}

	return p_ivi_controller_->GetPropertiesOfSurface(ivi_id, name);
}

/*****************************************************************************
 処理概要：	パラメータからレイヤープロパティ取得
 引数    ：	std::map<std::string, std::string>	&params		(i)パラメータmap
           	CVhalLayoutLayerProperty			&layer_prop	(o)レイヤープロパティ
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM			パラメータ不正
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::GetLayerFromParams(std::map<std::string, std::string> &params, CVhalLayoutLayerProperty &layer_prop) const
{
	/* id優先で使う */
	const auto itr_id = params.find("ivi_id");
	if (itr_id != params.end())
	{
		const int32_t ivi_id{std::stoi(itr_id->second)};
		const int32_t ret{p_layout_file_->GetLayerProperty(ivi_id, layer_prop)};
		if (0 > ret)
		{
			VHAL_LOGE("GetLayerProperty(%d) error. ret=%d", ivi_id, ret);
			return VHAL_ERR_LAYOUT_INFO;
		}
	}
	else
	{
		const auto itr_name = params.find("name");
		if (itr_name == params.end())
		{
			VHAL_LOGE("Invalid params for layer");
			return VHAL_ERR_PARAM;
		}
		const int32_t ret{p_layout_file_->GetLayerProperty(itr_name->second, layer_prop)};
		if (0 > ret)
		{
			VHAL_LOGE("GetLayerProperty(%s) error. ret=%d", itr_name->second.c_str(), ret);
			return VHAL_ERR_LAYOUT_INFO;
		}
	}

	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	レイヤーのorder値の取得
 引数    ：	std::map<std::string, std::string>	&params	(i)パラメータmap
           	int64_t								&value	(o) order値
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::GetIviLayerOrder(std::map<std::string, std::string> &params, int64_t &value) const
{
	int32_t ret{VHAL_SUCCESS};
	CVhalLayoutLayerProperty layer_prop{};

	VHAL_LOGV_IN();

	ret = GetLayerFromParams(params, layer_prop);
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("GetLayerFromParams() error. ret=%d", ret);
		return VHAL_ERR_LAYOUT_INFO;
	}

	const int32_t layer_id{layer_prop.GetIviId()};
	const int32_t order{layer_prop.GetOrder()};

	VHAL_LOGD("layer_id=%d, order=%d", layer_id, order);
	value = static_cast<int64_t>(order);

	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	レイヤーのorder値の設定
 引数    ：	std::map<std::string, std::string>	&params	(i)パラメータmap
           	int64_t								&value	(i) order値
           	bool								&changed(o) 設定値変化
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::SetIviLayerOrder(std::map<std::string, std::string> &params, const int64_t &value, bool &changed)
{
	int32_t ret{VHAL_SUCCESS};
	CVhalLayoutLayerProperty layer_prop{};
	changed = true;

	VHAL_LOGV_IN();

	ret = GetLayerFromParams(params, layer_prop);
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("GetLayerFromParams() error. ret=%d", ret);
		return VHAL_ERR_LAYOUT_INFO;
	}

	const int32_t layer_id{layer_prop.GetIviId()};
	const int32_t order{layer_prop.GetOrder()};

	const int32_t new_order{I64ToI32(value)};
	if (order == new_order)
	{
		VHAL_LOGI("layer[%d] order value is not changed. order=%d", layer_id, order);
		changed = false;
		return VHAL_SUCCESS;
	}

	/* レイヤーの並べ替え */
	VHAL_LOGI("layer[%d] order change %d -> %d", layer_id, order, new_order);
	ret = p_layout_file_->ReorderLayersOnScreen(layer_id, new_order);
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("ReorderLayersOnScreen() error. layer_id=%d, ret=%d", layer_id, ret);
		return VHAL_ERR_LAYOUT_INFO;
	}

	/* 親スクリーン上のレイヤiviidリストを取得 */
	const int32_t screen_id{layer_prop.GetParentScreenIviId()};
	VHAL_LOGD("layer[%d] is on screen[%d]", layer_id, screen_id);

	CVhalLayoutScreenProperty screen_prop{};
	ret = p_layout_file_->GetScreenProperty(screen_id, screen_prop);
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("error not found screen id=%d, ret=%d", screen_id, ret);
		return VHAL_ERR_LAYOUT_INFO;
	}

	std::vector<t_ilm_layer> ivilayer_ids{};
	std::vector<int32_t> layer_ids{};
	(void)screen_prop.GetLayerIds(layer_ids);

	std::vector<int32_t>::const_iterator itlayer{layer_ids.cbegin()};
	const std::vector<int32_t>::const_iterator itlayerend{layer_ids.cend()};
	for( ; itlayer!=itlayerend; ++itlayer)
	{
		const t_ilm_layer ivilayer_id{I32ToUI32(*itlayer)};
		ivilayer_ids.push_back(ivilayer_id);
	}

	/* レイヤーの並べ替え（ivi側に反映） */
	ret = p_ivi_controller_->SetLayerOrder(screen_id, ivilayer_ids);
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("SetLayerOrder() error. screen_id=%d, ret=%d", screen_id, ret);
		return VHAL_ERR_LAYOUT_INFO;
	}

	VHAL_LOGV_OUT();
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	iviスクリーンのスクリーンショット取得
 引数    ：	const int32_t ivi_id	(i)ivi id
            const std::string &path	(i)ファイルパス
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_ERR_ILM_API	ilm APIエラー
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::GetIviScreenshot(const int32_t ivi_id, const std::string &path) const
{
	int32_t ret{VHAL_ERR_PARAM};

	VHAL_LOGV_IN("ivi_id=%d, path=%s", ivi_id, path.c_str());

	CVhalLayoutScreenProperty screen_prop{};
	ret = p_layout_file_->GetScreenProperty(ivi_id, screen_prop);
	if (VHAL_SUCCESS == ret)
	{
		/* スクリーンショット取得 */
		ret = p_ivi_controller_->GetScreenshot(ivi_id, path);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("GetScreenshot(ivi_id=%d, path=%s) error. ret=%d", ivi_id, path.c_str(), ret);
		}
	}
	else
	{
		VHAL_LOGE("GetScreenProperty(ivi_id=%d) error. ret=%d", ivi_id, ret);
	}

	VHAL_LOGV_OUT("ret=%d", ret);
	return ret;
}

/*****************************************************************************
 処理概要：	iviサーフェスのスクリーンショット取得
 引数    ：	const int32_t ivi_id	(i)ivi id
            const std::string &path	(i)ファイルパス
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_ERR_ILM_API	ilm APIエラー
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::GetIviSurfaceScreenshot(const int32_t ivi_id, const std::string &path) const
{
	int32_t ret{VHAL_ERR_PARAM};

	VHAL_LOGV_IN("ivi_id=%d, path=%s", ivi_id, path.c_str());

	CVhalLayoutSurfaceProperty surface_prop{};
	ret = p_layout_file_->GetSurfaceProperty(ivi_id, surface_prop);
	if (VHAL_SUCCESS == ret)
	{
		/* スクリーンショット取得 */
		ret = p_ivi_controller_->GetSurfaceScreenshot(ivi_id, path);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("GetSurfaceScreenshot(ivi_id=%d, path=%s) error. ret=%d", ivi_id, path.c_str(), ret);
		}
	}
	else
	{
		VHAL_LOGE("GetSurfaceProperty(ivi_id=%d) error. ret=%d", ivi_id, ret);
	}

	VHAL_LOGV_OUT("ret=%d", ret);
	return ret;
}

/*****************************************************************************
 処理概要：	画面キャプチャ結果通知
 引数    ：	const int32_t result		(i)処理結果
           	const ScreenShotType type	(i)キャプチャタイプ
 戻り値  ：	なし
*****************************************************************************/
void CVhalLayoutManager::NotifyScreenShotResult(const int32_t result, const ScreenShotType type) const noexcept
{
	p_layoutManager_listener_->NotifyScreenShotResult(result, type);
}

/*****************************************************************************
 処理概要：	カメラ 有効な映像path判定
 引数    ：	const std::string&	video_path		(i)映像vpath文字列
         ：	bool 				&already_path	(o)映像パス変更判定
 戻り値  ：	処理結果
           		false		無効な映像path
           		true		有効な映像path
*****************************************************************************/
bool CVhalLayoutManager::IsValidVideoCameraPath(const std::string& video_path, bool &already_path) const
{
	bool is_valid{false};
	is_valid = IsValidVideoPathCommon(camera_path_current_, video_path, VIDEO_OUTPUT_TARGET_CAMERA, already_path);
	return is_valid;
}

/*****************************************************************************
 処理概要：	現在のカメラpathの取得
 引数    ：	std::string&	path	(o)カメラpath文字列
 戻り値  ：	処理結果
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::GetCameraPath(std::string& path) const
{
	path = camera_path_current_;
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	現在のカメラpathの設定
 引数    ：	const std::string&	path		(i)カメラpath文字列
         ：	bool				visibility	(i)可視状態
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM			パラメータ不正
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::SetCameraPath(const std::string& path, const bool visibility)
{
	int32_t ret{VHAL_SUCCESS};

	/* カメラ前席映像pathの設定 */
	ret = SetVideoPathCommon(camera_path_current_, path, kSurfaceNameMuteCamera, "", VIDEO_OUTPUT_TARGET_CAMERA, visibility);
	if (VHAL_SUCCESS != ret)
	{
		/* 映像パス設定 失敗 */
		VHAL_LOGE("SetVideoPathCommon error. ret=%d vpath=%s", ret, path.c_str());
	}
	else
	{
		/* 映像パス変更判定 */
		if( 0 != camera_path_current_.compare(path) )
		{
			/* カメラ映像表示→非表示の場合 */
			if( !camera_path_current_.empty() )
			{
				bool camera_visibility{false};
				ret = GetVisibilityCommon(camera_visibility, camera_path_current_, VIDEO_OUTPUT_TARGET_CAMERA);
				if( VHAL_SUCCESS != ret )
				{
					VHAL_LOGE("GetVisibilityCommon error. ret=%d path=%s, output_target=0x%x", ret, camera_path_current_.c_str(), VIDEO_OUTPUT_TARGET_CAMERA);
				}
				else
				{
					/* カメラ映像のサーフェスが表示状態のまま */
					if( true == camera_visibility )
					{
						VHAL_LOGE("No change to Visibility[%d]. path=%s, output_target=0x%x", camera_visibility, camera_path_current_.c_str(), VIDEO_OUTPUT_TARGET_CAMERA);
						ret = VHAL_ERR_LAYOUT_INFO;
					}
				}
			}
		}
	}

	if( VHAL_SUCCESS == ret )
	{
		/* 映像パス設定 成功 */
		camera_path_current_ = path;
	}
	return ret;
}

/*****************************************************************************
 処理概要：	カメラ映像のデスティネーション矩形の取得
 引数    ：	int32_t				&x		(i)x座標
           	int32_t				&y		(i)y座標
           	int32_t				&w		(i)幅
           	int32_t				&h		(i)高さ
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::GetCameraDestRectangle(int32_t &x, int32_t &y, int32_t &w, int32_t &h) const
{
	int32_t ret{VHAL_SUCCESS};

	/* カメラ映像pathの矩形取得 */
	ret = GetDestRectangleCommon(x, y, w, h, VHAL_PATH_CAMERA, VIDEO_OUTPUT_TARGET_CAMERA);
	return ret;
}

/*****************************************************************************
 処理概要：	カメラ映像のデスティネーション矩形の設定
 引数    ：	int32_t				x		(i)x座標
           	int32_t				y		(i)y座標
           	int32_t				w		(i)幅
           	int32_t				h		(i)高さ
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::SetCameraDestRectangle(const int32_t x, const int32_t y, const int32_t w, const int32_t h)
{
	int32_t ret{VHAL_SUCCESS};

	/* カメラ映像pathの矩形設定 */
	ret = SetDestRectangleCommon(x, y, w, h, VHAL_PATH_CAMERA, kSurfaceNameMuteCamera, "", VIDEO_OUTPUT_TARGET_CAMERA);
	return ret;

// MISRA C++-2008 Rule 16-2-1 #if 0
	// if (camera_path_current_.empty())
	// {
	// 	VHAL_LOGI("No camera path set.");
	// 	x = 0;
	// 	y = 0;
	// 	w = 0;
	// 	h = 0;
	// }
	// else
	// {
	// 	int32_t ret = VHAL_SUCCESS;
	// 	struct CVhalLayoutVideoSetting::VideoRectangleData output(x, y, w, h);
	// 	ret = video_setting_.SetVideoOutputSize(camera_path_current_, VIDEO_OUTPUT_TARGET_CAMERA, output);
	// 	if( VHAL_SUCCESS != ret )
	// 	{
	// 		VHAL_LOGE("SetVideoOutputSize error ret=%d path=%s", ret, camera_path_current_.c_str());
	// 	}

	// 	CVhalLayoutSurfaceProperty surface_prop_cur;
	// 	ret = p_layout_file_->GetSurfaceProperty(camera_path_current_, surface_prop_cur);
	// 	if( VHAL_SUCCESS == ret )
	// 	{
	// 		int32_t surface_id = surface_prop_cur.GetIviId();
	// 		p_ivi_controller_->SetSurfaceDestinationRectangle(surface_id, x, y, w, h);

// MISRA C++-2008 Rule 16-2-1 #if 0
			/* ブラインダーのサイズを追従させる */
			// CVhalLayoutSurfaceProperty blinder_prop;
			// ret = p_layout_file_->GetSurfaceProperty(kSurfaceNameMuteFrontVideo, blinder_prop);
			// if( VHAL_SUCCESS == ret )
			// {
			// 	surface_id = blinder_prop.GetIviId();
			// 	p_ivi_controller_->SetSurfaceDestinationRectangle(surface_id, x, y, w, h);
			// }
// MISRA C++-2008 Rule 16-2-1 #endif
	// 		p_ivi_controller_->CommitChanges();
	// 	}
	// }

	// return VHAL_SUCCESS;
// MISRA C++-2008 Rule 16-2-1 #endif
}

/*****************************************************************************
 処理概要：	カメラ映像の入力サイズ強制設定
 引数    ：	int32_t	w	(i)幅
           	int32_t	h	(i)高さ
 戻り値  ：	処理結果
            	VHAL_ERR_****		エラー
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::SetCameraForceInputSize(const int32_t w, const int32_t h)
{
	int32_t	ret{VHAL_SUCCESS};

	/* 映像入力パラメータ設定 */
	const struct CVhalLayoutVideoSetting::VideoRectangleData input{0, 0, w, h, false};
	ret = video_setting_.SetVideoInputParam(VHAL_PATH_CAMERA, VIDEO_OUTPUT_TARGET_CAMERA, true, input);
	if( VHAL_SUCCESS != ret )
	{
		VHAL_LOGE("SetVideoInputParam error ret=%d path=%s", ret, VHAL_PATH_CAMERA.c_str());
	}

	return ret;
}

/*****************************************************************************
 処理概要：	カメラ映像のソース矩形の強制設定
 引数    ：	int32_t				x					(i)x座標
           	int32_t				y					(i)y座標
           	int32_t				w					(i)幅
           	int32_t				h					(i)高さ
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO		レイアウト情報不正
           		VHAL_SUCCESS				正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::SetCameraForceSourceRectangle(const int32_t x, const int32_t y, const int32_t w, const int32_t h)
{
	int32_t result{VHAL_SUCCESS};

	/* カメラ映像pathの入力矩形設定 */
	int32_t ret{SetSourceRectangleCommon(true, x, y, w, h, VHAL_PATH_CAMERA, VIDEO_OUTPUT_TARGET_CAMERA)};
	if( VHAL_SUCCESS != ret )
	{
		VHAL_LOGE("SetSourceRectangleCommon error ret=%d path=%s", ret, VHAL_PATH_CAMERA.c_str());
		result = ret;
	}
	else
	{
		/* [POST21MM-53784]ソース矩形変更後のWIDE設定により、カメラ映像の出力サイズが変更されるため、カメラMUTEサイズを追随する(WIDE設定は初回出力サイズが設定されるまで有効) */
		ret = SetVideoBlindSurfaceRectangle(kSurfaceNameMuteCamera, VHAL_PATH_CAMERA, VIDEO_OUTPUT_TARGET_CAMERA);
		if( VHAL_SUCCESS != ret )
		{
			VHAL_LOGW("SetVideoBlindSurfaceRectangle error ret=%d, mute_surface_name=%s", ret, kSurfaceNameMuteCamera.c_str());
		}
	}

	return result;
}

/*****************************************************************************
 処理概要：	カメラ映像のワイド取得
 引数    ：	uint32_t	wide		(o)ワイド設定(0-3)
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::GetCameraWideMode(uint32_t &wide_mode) const
{
	int32_t ret{VHAL_SUCCESS};

	/* 映像のワイド取得 */
	ret = video_setting_.GetVideoWideMode(VHAL_PATH_CAMERA, VIDEO_OUTPUT_TARGET_CAMERA, wide_mode);
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("GetVideoWideMode error. ret=%d", ret);
	}

	return ret;
}

/*****************************************************************************
 処理概要：	カメラ映像のワイド設定(値更新のみ)
 引数    ：	uint32_t	wide		(i)ワイド設定(0-3)
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::SetCameraWideMode(const uint32_t wide_mode)
{
	int32_t ret{VHAL_SUCCESS};

	/* 映像のワイド設定 */
	ret = video_setting_.SetVideoWideMode(VHAL_PATH_CAMERA, VIDEO_OUTPUT_TARGET_CAMERA, wide_mode);
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("SetVideoWideMode error. ret=%d", ret);
	}

	return ret;
}

/*****************************************************************************
 処理概要：	Instrument Cluster 有効な映像path判定
 引数    ：	const std::string&	video_path			(i)映像vpath文字列
         ：	bool 				&already_path		(o)映像パス変更判定
 戻り値  ：	処理結果
           		false		無効な映像path
           		true		有効な映像path
*****************************************************************************/
bool CVhalLayoutManager::IsValidVideoIClusterPath(const std::string& video_path, bool &already_path) const
{
	bool is_valid{false};
	is_valid = IsValidVideoPathCommon(video_icluster_path_current_, video_path, VIDEO_OUTPUT_TARGET_IC, already_path);
	return is_valid;
}

/*****************************************************************************
 処理概要：	現在のInstrument Cluster pathの取得
 引数    ：	std::string&	path	(o)Instrument Cluster path文字列
 戻り値  ：	処理結果
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::GetIClusterPath(std::string& path) const
{
	path = video_icluster_path_current_;
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	現在のInstrument Cluster vpathの設定
 引数    ：	const std::string&	path	(i)vpath文字列
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM			パラメータ不正
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::SetIClusterPath(const std::string& path)
{
	int32_t ret{VHAL_SUCCESS};

	/* Instrument Cluster 映像pathの設定 (ブラインダなし) */
	ret = SetVideoPathCommon(video_icluster_path_current_, path, "", "", VIDEO_OUTPUT_TARGET_IC, true);
	if (VHAL_SUCCESS != ret)
	{
		/* 映像パス設定 失敗 */
		VHAL_LOGE("SetVideoPathCommon error. ret=%d vpath=%s", ret, path.c_str());
		return ret;
	}

	/* 映像パス設定 成功 */
	video_icluster_path_current_ = path;
	return ret;
}

/*****************************************************************************
 処理概要：	Instrument Cluster 映像のソース矩形の取得
 引数    ：	bool				&clipping_enable	(o)クリッピング有効フラグ
           	int32_t				&x					(o)x座標
           	int32_t				&y					(o)y座標
           	int32_t				&w					(o)幅
           	int32_t				&h					(o)高さ
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO		レイアウト情報不正
           		VHAL_SUCCESS				正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::GetIClusterSourceRectangle(bool &clipping_enable, int32_t &x, int32_t &y, int32_t &w, int32_t &h) const
{
	/* Instrument Cluster 映像pathの入力矩形取得 */
	const int32_t ret{GetSourceRectangleCommon(clipping_enable, x, y, w, h, video_icluster_path_current_, VIDEO_OUTPUT_TARGET_IC)};
	return ret;
}

/*****************************************************************************
 処理概要：	Instrument Cluster 映像のソース矩形の設定
 引数    ：	bool				clipping_enable		(i)クリッピング有効フラグ
           	int32_t				x					(i)x座標
           	int32_t				y					(i)y座標
           	int32_t				w					(i)幅
           	int32_t				h					(i)高さ
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO		レイアウト情報不正
           		VHAL_SUCCESS				正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::SetIClusterSourceRectangle(const bool clipping_enable, const int32_t x, const int32_t y, const int32_t w, const int32_t h)
{
	/* Instrument Cluster 映像pathの入力矩形設定 */
	const int32_t ret{SetSourceRectangleCommon(clipping_enable, x, y, w, h, video_icluster_path_current_, VIDEO_OUTPUT_TARGET_IC)};
	return ret;
}

/*****************************************************************************
 処理概要：	Instrument Cluster 映像のデスティネーション矩形の設定
 引数    ：	int32_t				x		(i)x座標
           	int32_t				y		(i)y座標
           	int32_t				w		(i)幅
           	int32_t				h		(i)高さ
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO		レイアウト情報不正
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::SetIClusterDestRectangle(const int32_t x, const int32_t y, const int32_t w, const int32_t h)
{
	int32_t ret{VHAL_SUCCESS};

	/* Instrument Cluster 映像pathの矩形設定 */
	ret = SetDestRectangleCommon(x, y, w, h, video_icluster_path_current_, kSurfaceNameMuteIcDisplay, "", VIDEO_OUTPUT_TARGET_IC);
	return ret;
}

/*****************************************************************************
 処理概要：	Instrument Cluster 映像の可視状態取得
 引数    ：	bool&		visibility		(o)可視状態
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::GetIClusterVisibility(bool& visibility) const
{
	int32_t ret{VHAL_SUCCESS};

	/* Instrument Cluster 映像pathの可視状態取得 */
	ret = GetVisibilityCommon(visibility, video_icluster_path_current_, VIDEO_OUTPUT_TARGET_IC);

	return ret;
}

/*****************************************************************************
 処理概要：	Instrument Cluster 映像の可視状態設定
 引数    ：	bool		visibility		(i)可視状態
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::SetIClusterVisibility(const bool visibility)
{
	int32_t ret{VHAL_SUCCESS};

	/* Instrument Cluster 映像pathの可視状態設定 */
	ret = SetVisibilityCommon(visibility, video_icluster_path_current_, VIDEO_OUTPUT_TARGET_IC);
	return ret;
}

/*****************************************************************************
 処理概要：	Instrument Cluster 映像の不透明度設定
 引数    ：	uint32_t	opacity		(i)不透明度(0-100)
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::SetIClusterOpacity(const uint32_t opacity)
{
	int32_t ret{VHAL_SUCCESS};

	/* Instrument Cluster 映像の不透明度設定 */
	ret = SetOpacityCommon(opacity, video_icluster_path_current_, VIDEO_OUTPUT_TARGET_IC);
	return ret;
}

/*****************************************************************************
 処理概要：	ブラインダーの有効無効を切り替える
 引数    ：	VhalBlinderType	blinder_type	(i)ブラインダータイプ
           	bool enable						(i)true有効／false無効
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::SetBlinderEnable(const VhalBlinderType blinder_type, const bool enable)
{
	int32_t result{VHAL_SUCCESS};
	std::string name{};

	switch(blinder_type)
	{
		case VhalBlinderType::VHAL_BLINDER_TYPE_FRONT_DISPLAY:
			name = kSurfaceNameMuteFrontDisplay;
			break;
		case VhalBlinderType::VHAL_BLINDER_TYPE_FRONT_VIDEO:
			if ((0U == video_front_path_current_.length()) && (true == enable))
			{
				result = VHAL_ERR_PARAM;
				break;
			}
			name = kSurfaceNameMuteFrontVideo;
			break;
		case VhalBlinderType::VHAL_BLINDER_TYPE_REAR_DISPLAY:
			name = kSurfaceNameMuteRearDisplay;
			break;
		case VhalBlinderType::VHAL_BLINDER_TYPE_REAR_VIDEO:
			/* 映像パスが設定されていない場合はMUTEを有効にしない */
			if ((0U == video_rear_path_current_.length()) && (true == enable))
			{
				result = VHAL_ERR_PARAM;
				break;
			}
			name = kSurfaceNameMuteRearVideo;
			break;
		case VhalBlinderType::VHAL_BLINDER_TYPE_CAMERA:
			name = kSurfaceNameMuteCamera;
			break;
		case VhalBlinderType::VHAL_BLINDER_TYPE_IC_DISPLAY:
			name = kSurfaceNameMuteIcDisplay;
			break;
		case VhalBlinderType::VHAL_BLINDER_TYPE_FRONT_VIDEO_SYNC:
			name = kSurfaceNameMuteFrontVideoSync;
			break;
		case VhalBlinderType::VHAL_BLINDER_TYPE_HUD_DISPLAY:
			name = kSurfaceNameMuteHudDisplay;
			break;
		default:
			result = VHAL_ERR_PARAM;
			break;
	}

	if (VHAL_SUCCESS == result)
	{
		CVhalLayoutSurfaceProperty surface_prop{};
		const int32_t ret{p_layout_file_->GetSurfaceProperty(name, surface_prop)};
		if (VHAL_SUCCESS == ret)
		{
			const int32_t ivi_id{surface_prop.GetIviId()};
			(void)p_ivi_controller_->SetSurfaceVisibility(ivi_id, enable);
			(void)p_ivi_controller_->CommitChanges();
		}
		else
		{
			VHAL_LOGI("not found blinder [%s]", name.c_str());
			result = ret;
		}
	}

	return result;
}

/*****************************************************************************
 処理概要：	前席ディスプレイのMUTE初期設定を実施する
 引数    ：	bool enable						(i)true有効／false無効
 戻り値  ：	なし
*****************************************************************************/
void CVhalLayoutManager::SetBlinderFrontDispMuteInit(const bool enable) noexcept
{
	front_disp_blinder_init_ = enable;
}

/*****************************************************************************
 処理概要：	HUD MUTEディスプレイサーフェス(mute_hud_display)の初期化設定
 引数    ：	const bool enable	(i) visible
 戻り値  ：	なし
*****************************************************************************/
void CVhalLayoutManager::SetBlinderHudDispMuteInit(const bool enable) noexcept
{
	hud_disp_blinder_init_ = enable;
}

/*****************************************************************************
 処理概要：	ブラインダーのivi_idを取得する
 引数    ：	VhalBlinderType	blinder_type	(i)ブラインダータイプ
         ：	int32_t			&ivi_id			(o)ブラインダーのivi_id
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM		パラメータ不正
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::GetBlinderID(const VhalBlinderType blinder_type, int32_t &ivi_id) const
{
	int32_t result{VHAL_SUCCESS};
	std::string name{};

	switch(blinder_type)
	{
		case VhalBlinderType::VHAL_BLINDER_TYPE_FRONT_DISPLAY:
			name = kSurfaceNameMuteFrontDisplay;
			break;
		case VhalBlinderType::VHAL_BLINDER_TYPE_FRONT_VIDEO:
			name = kSurfaceNameMuteFrontVideo;
			break;
		case VhalBlinderType::VHAL_BLINDER_TYPE_REAR_DISPLAY:
			name = kSurfaceNameMuteRearDisplay;
			break;
		case VhalBlinderType::VHAL_BLINDER_TYPE_REAR_VIDEO:
			name = kSurfaceNameMuteRearVideo;
			break;
		case VhalBlinderType::VHAL_BLINDER_TYPE_CAMERA:
			name = kSurfaceNameMuteCamera;
			break;
		case VhalBlinderType::VHAL_BLINDER_TYPE_IC_DISPLAY:
			name = kSurfaceNameMuteIcDisplay;
			break;
		case VhalBlinderType::VHAL_BLINDER_TYPE_FRONT_VIDEO_BG:
			name = kSurfaceNameVideoFrontBg;
			break;
		case VhalBlinderType::VHAL_BLINDER_TYPE_REAR_VIDEO_BG:
			name = kSurfaceNameVideoRearBg;
			break;
		case VhalBlinderType::VHAL_BLINDER_TYPE_FRONT_VIDEO_SYNC:
			name = kSurfaceNameMuteFrontVideoSync;
			break;
		case VhalBlinderType::VHAL_BLINDER_TYPE_HUD_DISPLAY:
			name = kSurfaceNameMuteHudDisplay;
			break;
		default:
			result = VHAL_ERR_PARAM;
			break;
	}

	if (VHAL_SUCCESS == result)
	{
		CVhalLayoutSurfaceProperty surface_prop{};
		const int32_t ret{p_layout_file_->GetSurfaceProperty(name, surface_prop)};
		if (VHAL_SUCCESS == ret)
		{
			ivi_id = surface_prop.GetIviId();
		}
		else
		{
			VHAL_LOGI("not found blinder [%s]", name.c_str());
			result = ret;
		}
	}

	return result;
}

/*****************************************************************************
 処理概要：	ブラインダーのレイアウト初期設定
 引数    ：	const std::string	&name	(i)サーフェス名
           	int32_t	surfaceId			(i)サーフェスID
           	int32_t	width				(i)幅
           	int32_t	height				(i)高さ
 戻り値  ：	処理結果
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::ConfigureBlinder(const std::string &name, const int32_t surfaceId, const int32_t width, const int32_t height)
{
	int32_t ret{VHAL_SUCCESS};
	int32_t dst_w{width};
	int32_t dst_h{height};
	bool visibility{false};
	std::string screen_name{};

	if (0 == name.compare(kSurfaceNameMuteFrontDisplay))
	{
		screen_name = kScreenNameFront;
		visibility = front_disp_blinder_init_;
	}
	else if (0 == name.compare(kSurfaceNameMuteRearDisplay))
	{
		screen_name = kScreenNameRear;
	}
	else if (0 == name.compare(kSurfaceNameMuteIcDisplay))
	{
		screen_name = kScreenNameICluster;
	}
	else if (0 == name.compare(kSurfaceNameMuteCamera))
	{
		screen_name = kScreenNameFront;
	}
	else if (0 == name.compare(kSurfaceNameMuteHudDisplay))
	{
		screen_name = kScreenNameHud;
		visibility = hud_disp_blinder_init_;
	}
	else
	{
		/* スクリーンサイズ対象以外の場合はscreen_name設定不要 */
	}

	/* スクリーンサイズ対象の場合、スクリーンサイズを取得 */
	if (!screen_name.empty())
	{
		CVhalLayoutScreenProperty screen_prop{};
		ret = p_layout_file_->GetScreenProperty(screen_name, screen_prop);	
		if( VHAL_SUCCESS == ret )
		{
			const int32_t ivi_id{screen_prop.GetIviId()};
			const bool screen_chk{IsScreenAvailable(ivi_id)};
			if( true == screen_chk )
			{
				ret = p_ivi_controller_->GetScreenResolution(ivi_id, dst_w, dst_h);
				if( VHAL_SUCCESS != ret )
				{
					VHAL_LOGW("GetScreenResolution error. ret=%d", ret);
				}
			}
		}
		else
		{
			VHAL_LOGW("GetScreenProperty error. ret=%d", ret);
		}
	}

	(void)p_ivi_controller_->SetSurfaceSourceRectangle(surfaceId, 0, 0, width, height);
	(void)p_ivi_controller_->SetSurfaceDestinationRectangle(surfaceId, 0, 0, dst_w, dst_h);
	(void)p_ivi_controller_->SetSurfaceVisibility(surfaceId, visibility);/* サーフェス初期表示に依存せず、内部設定で更新する */
	ret = p_ivi_controller_->CommitChanges();
	if( VHAL_SUCCESS != ret )
	{
		VHAL_LOGE("CommitChanges error. ret=%d", ret);
	}
	else
	{
		VHAL_LOGI("id=%d name=%s visible=%d src(0,0,%d,%d) dest(0,0,%d,%d)", surfaceId, name.c_str(), visibility, width, height, dst_w, dst_h);
	}

	return ret;
}

/*****************************************************************************
 処理概要：	動画再生のデスティネーション矩形の設定
 引数    ：	int32_t				x		(i)x座標
           	int32_t				y		(i)y座標
           	int32_t				w		(i)幅
           	int32_t				h		(i)高さ
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::SetMovieFrontDestRectangle(const int32_t x, const int32_t y, const int32_t w, const int32_t h) noexcept
{
	constexpr int32_t ret{VHAL_SUCCESS};

	movie_x_ = x;
	movie_y_ = y;
	movie_width_ = w;
	movie_height_ = h;
	
	return ret;
}
/*****************************************************************************
 処理概要：	動画再生のレイアウト初期設定
 引数    ：	int32_t	surfaceId			(i)サーフェスID
           	int32_t	x					(i)X座標
           	int32_t	y					(i)Y座標
           	int32_t	width				(i)幅
           	int32_t	height				(i)高さ
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
*****************************************************************************/
int32_t CVhalLayoutManager::ConfigureMovie(const std::string &name, const int32_t surfaceId, const int32_t width, const int32_t height) const
{
	const int32_t dst_x{movie_x_};
	const int32_t dst_y{movie_y_};
	const int32_t dst_w{movie_width_};
	const int32_t dst_h{movie_height_};

	(void)p_ivi_controller_->SetSurfaceSourceRectangle(surfaceId, 0, 0, width, height);
	(void)p_ivi_controller_->SetSurfaceDestinationRectangle(surfaceId, dst_x, dst_y, dst_w, dst_h);
	(void)p_ivi_controller_->SetSurfaceVisibility(surfaceId, true);
	
	const int32_t ret{p_ivi_controller_->CommitChanges()};
	if( VHAL_SUCCESS != ret )
	{
		VHAL_LOGE("CommitChanges error. ret=%d", ret);
		p_layoutManager_listener_->NotifyMovieResult(VHAL_MOVIE_STS_FAILED);
	}
	else
	{
		VHAL_LOGI("Movie [%s] src(%dx%d) dst(%d,%d,%d,%d)", name.c_str(), width, height,dst_x, dst_y, dst_w, dst_h);
		p_layoutManager_listener_->NotifyMovieResult(VHAL_MOVIE_STS_PLAYING);
	}

	return ret;
}

/*****************************************************************************
 処理概要：	映像path設定（共通）
 引数    ：	const std::string&	video_path_prev		(i)変更前vpath文字列
         ：	const std::string&	video_path_next		(i)変更後vpath文字列
         ：	const std::string&	mute_surface_name	(i)MUTEサーフェス名
         ：	const std::string&	bg_surface_name		(i)背景サーフェス名
         ：	VideoOutputTarget 	output_target		(i)映像出力先
         ：	bool				visibility			(i)可視状態
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM			パラメータ不正
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::SetVideoPathCommon(const std::string& video_path_prev, const std::string& video_path_next, const std::string& mute_surface_name, const std::string& bg_surface_name, const VideoOutputTarget output_target, const bool visibility)
{
	/* 映像パス変更判定 */
	if (0 == video_path_prev.compare(video_path_next))
	{
		/* 映像パスに変化が無い場合はスキップ */
		VHAL_LOGI("already set video path=%s", video_path_next.c_str());
		return VHAL_SUCCESS;
	}

	/* 新しい映像パスのサーフェスを表示 */
	if( !video_path_next.empty() )
	{
		int32_t surface_id_next{0};
		int32_t ret{GetSurfaceIdFromVideoPath(video_path_next, output_target, surface_id_next)};
		if( VHAL_SUCCESS == ret )
		{
			/* カメラ映像パスの場合のみ映像可視状態を更新（その他映像パスの場合は映像可視状態設定時に行う） */
			if (0U < (VIDEO_OUTPUT_TARGET_CAMERA & output_target))
			{
				/* 映像可視状態の内部データ更新 */
				ret = video_setting_.SetVideoVisibility(video_path_next, output_target, visibility);
				if( VHAL_SUCCESS != ret )
				{
					VHAL_LOGE("SetVideoVisibility error ret=%d path=%s, output_target=0x%x", ret, video_path_next.c_str(), output_target);
				}

				/* 切り替え後のサーフェスを表示 */
				if (true == visibility)
				{
					(void)p_ivi_controller_->SetSurfaceVisibility(surface_id_next, true);
				}
			}

			/* 映像MUTEサーフェスのサイズを追従させる */
			if( !mute_surface_name.empty() )
			{
				ret = SetVideoBlindSurfaceRectangle(mute_surface_name, video_path_next, output_target);
				if( VHAL_SUCCESS != ret )
				{
					VHAL_LOGW("SetVideoBlindSurfaceRectangle error ret=%d, surface_name=%s", ret, mute_surface_name.c_str());
				}
			}
			/* 映像背景サーフェスのサイズを追従させる */
			if( !bg_surface_name.empty() )
			{
				ret = SetVideoBlindSurfaceRectangle(bg_surface_name, video_path_next, output_target);
				if( VHAL_SUCCESS != ret )
				{
					VHAL_LOGW("SetVideoBlindSurfaceRectangle error ret=%d, surface_name=%s", ret, bg_surface_name.c_str());
				}
			}
		}
		else
		{
			VHAL_LOGE("not found next path=%s", video_path_next.c_str());
			return VHAL_ERR_LAYOUT_INFO;
		}
	}

	/* 古い映像パスのサーフェスを非表示 */
	if( !video_path_prev.empty() )
	{
		int32_t surface_id_prev{0};
		int32_t ret{GetSurfaceIdFromVideoPath(video_path_prev, output_target, surface_id_prev)};
		if( VHAL_SUCCESS == ret )
		{
			/* 映像可視状態の内部データ更新 */
			ret = video_setting_.SetVideoVisibility(video_path_prev, output_target, false);
			if( VHAL_SUCCESS != ret )
			{
				VHAL_LOGE("SetVideoVisibility error ret=%d path=%s, output_target=0x%x", ret, video_path_prev.c_str(), output_target);
			}

			(void)p_ivi_controller_->SetSurfaceVisibility(surface_id_prev, false);
		}
		else
		{
			VHAL_LOGI("not found prev path=%s", video_path_prev.c_str());
		}
	}

	(void)p_ivi_controller_->CommitChanges();

	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	映像MUTE解除設定（共通）
 引数    ：	const std::string&	video_path_prev		(i)変更前vpath文字列
         ：	const std::string&	video_path_next		(i)変更後vpath文字列
         ：	const std::string&	mute_surface_name	(i)MUTEサーフェス名
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM			パラメータ不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::SetVideoMuteOffCommon(const std::string& video_path_prev, const std::string& video_path_next, const std::string& mute_surface_name)
{
	/* 映像パス変更判定 */
	if (0 == video_path_prev.compare(video_path_next))
	{
		/* 映像パスに変化が無い場合はスキップ */
		VHAL_LOGI("already set video path=%s", video_path_next.c_str());
		return VHAL_SUCCESS;
	}

	/* 映像パスがクリアされた場合、ブラインダーサーフェスも非表示 */
	if (video_path_next.empty())
	{
		if (!mute_surface_name.empty())
		{
			CVhalLayoutSurfaceProperty blinder_prop{};
			const int32_t ret{p_layout_file_->GetSurfaceProperty(mute_surface_name, blinder_prop)};
			if (VHAL_SUCCESS == ret)
			{
				const int32_t blind_surface{blinder_prop.GetIviId()};
				(void)p_ivi_controller_->SetSurfaceVisibility(blind_surface, false);
				(void)p_ivi_controller_->CommitChanges();
			}
			else
			{
				VHAL_LOGI("not found blinder [%s]", mute_surface_name.c_str());
			}
		}
	}


	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	映像pathの入力矩形取得（共通）
 引数    ：	bool				&clipping_enable	(o)クリッピング有効フラグ
           	int32_t				&x					(o)x座標
           	int32_t				&y					(o)y座標
           	int32_t				&w					(o)幅
           	int32_t				&h					(o)高さ
           	const std::string&	video_path_current	(i)対象映像vpath文字列
            VideoOutputTarget	output_target		(i)映像出力先
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO		レイアウト情報不正
           		VHAL_SUCCESS				正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::GetSourceRectangleCommon(bool &clipping_enable, int32_t &x, int32_t &y, int32_t &w, int32_t &h, const std::string& video_path_current, const VideoOutputTarget output_target) const
{
	int32_t ret{VHAL_SUCCESS};

	if (video_path_current.empty())
	{
		VHAL_LOGI("no vpath set");
		clipping_enable = false;
		x = 0;
		y = 0;
		w = 0;
		h = 0;
	}
	else
	{
		/* 映像入力パラメータ取得 */
		struct CVhalLayoutVideoSetting::VideoRectangleData input{};
		ret = video_setting_.GetVideoInputParam(video_path_current, output_target, clipping_enable, input);
		if( VHAL_SUCCESS == ret )
		{
			x = input.x_;
			y = input.y_;
			w = input.width_;
			h = input.height_;
		}
	}

	return ret;
}

/*****************************************************************************
 処理概要：	映像pathの入力矩形設定（共通）
 引数    ：	bool				clipping_enable		(i)クリッピング有効フラグ
           	int32_t				x					(i)x座標
           	int32_t				y					(i)y座標
           	int32_t				w					(i)幅
           	int32_t				h					(i)高さ
           	const std::string&	video_path_current	(i)対象映像vpath文字列
            VideoOutputTarget 	output_target		(i)映像出力先
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM				パラメータ不正
           		VHAL_ERR_LAYOUT_INFO		レイアウト情報不正
           		VHAL_SUCCESS				正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::SetSourceRectangleCommon(const bool clipping_enable, const int32_t x, const int32_t y, const int32_t w, const int32_t h, const std::string& video_path_current, const VideoOutputTarget output_target)
{
	int32_t ret{VHAL_SUCCESS};

	if (video_path_current.empty())
	{
		VHAL_LOGI("no vpath set");
	}
	else
	{
		const struct CVhalLayoutVideoSetting::VideoRectangleData input{x, y, w, h, false};
		/* 映像入力パラメータ設定 */
		ret = video_setting_.SetVideoInputParam(video_path_current, output_target, clipping_enable, input);
		if( VHAL_SUCCESS != ret )
		{
			VHAL_LOGE("SetVideoInputParam error ret=%d path=%s", ret, video_path_current.c_str());
		}
		else
		{
			/* 映像スケーリングの表示矩形取得 */
			CVhalLayoutVideoSetting::VideoRectangleData out_src{};
			CVhalLayoutVideoSetting::VideoRectangleData out_dest{};
			ret = GetVideoScalingRectangleCommon(video_path_current, output_target, out_src, out_dest);
			if( VHAL_SUCCESS != ret )
			{
				VHAL_LOGE("GetVideoScalingRectangleCommon error. ret=%d, video_path=%s, output_target=0x%x", ret, video_path_current.c_str(), output_target);
			}
			else
			{
				/* 映像パス名と映像出力先からサーフェスIDを取得 */
				int32_t surface_id{0};
				ret = GetSurfaceIdFromVideoPath(video_path_current, output_target, surface_id);
				if( VHAL_SUCCESS == ret )
				{
					/* 入力矩形設定 */
					ret = p_ivi_controller_->SetSurfaceSourceRectangle(surface_id, out_src.x_, out_src.y_, out_src.width_, out_src.height_);
					if( VHAL_SUCCESS != ret )
					{
						VHAL_LOGE("SetSurfaceSourceRectangle error. ret=%d", ret);
					}
					else
					{
						/* 出力矩形設定 */
						ret = p_ivi_controller_->SetSurfaceDestinationRectangle(surface_id, out_dest.x_, out_dest.y_, out_dest.width_, out_dest.height_);
						if( VHAL_SUCCESS != ret )
						{
							VHAL_LOGE("SetSurfaceDestinationRectangle error. ret=%d", ret);
						}
						else
						{
							/* 表示設定 */
							ret = p_ivi_controller_->CommitChanges();
							if( VHAL_SUCCESS != ret )
							{
								VHAL_LOGE("CommitChanges error. ret=%d", ret);
							}
						}
					}
				}
				else
				{
					ret = VHAL_ERR_LAYOUT_INFO;
				}
			}
		}
	}

	return ret;
}

/*****************************************************************************
 処理概要：	映像pathの出力矩形取得（共通）
 引数    ：	int32_t				&x		(o)x座標
           	int32_t				&y		(o)y座標
           	int32_t				&w		(o)幅
           	int32_t				&h		(o)高さ
           	const std::string&	video_path_current	(i)対象映像vpath文字列
            VideoOutputTarget output_target			(i)映像出力先
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO		レイアウト情報不正
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::GetDestRectangleCommon(int32_t &x, int32_t &y, int32_t &w, int32_t &h, const std::string& video_path_current, const VideoOutputTarget output_target) const
{
	int32_t ret{VHAL_SUCCESS};

	if (video_path_current.empty())
	{
		VHAL_LOGI("no vpath set");
		x = 0;
		y = 0;
		w = 0;
		h = 0;
	}
	else
	{
		struct CVhalLayoutVideoSetting::VideoRectangleData output{};
		ret = video_setting_.GetVideoOutputSize(video_path_current, output_target, output);
		if( VHAL_SUCCESS == ret )
		{
			/* 出力サイズが設定されていれば、その値を使用 */
			if ( true == output.set_ )
			{
				x = output.x_;
				y = output.y_;
				w = output.width_;
				h = output.height_;
			}
			else
			{
				struct CVhalLayoutVideoSetting::VideoRectangleData input{};
				bool clipping_enable{false};
				ret = video_setting_.GetVideoInputParam(video_path_current, output_target, clipping_enable, input);
				if( VHAL_SUCCESS == ret )
				{
					/* 出力サイズが設定されていなければ、入力サイズをそのまま使用 */
					if ( true == input.set_ )
					{
						x = input.x_;
						y = input.y_;
						w = input.width_;
						h = input.height_;
					}
					/* 入力サイズも設定されていなければ、出力サイズの初期値を使用 */
					else
					{
						x = output.x_;
						y = output.y_;
						w = output.width_;
						h = output.height_;
					}
				}
			}
		}
	}

	return ret;
}

/*****************************************************************************
 処理概要：	映像pathの矩形設定（共通）
 引数    ：	int32_t				x		(i)x座標
           	int32_t				y		(i)y座標
           	int32_t				w		(i)幅
           	int32_t				h		(i)高さ
           	const std::string&	video_path_current	(i)対象映像vpath文字列
            const std::string&	mute_surface_name	(i)MUTEサーフェス名
            const std::string&	bg_surface_name		(i)背景サーフェス名
            VideoOutputTarget output_target			(i)映像出力先
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO		レイアウト情報不正
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::SetDestRectangleCommon(const int32_t x, const int32_t y, const int32_t w, const int32_t h, const std::string& video_path_current, const std::string& mute_surface_name, const std::string& bg_surface_name, const VideoOutputTarget output_target)
{
	int32_t ret{VHAL_SUCCESS};

	if (video_path_current.empty())
	{
		VHAL_LOGI("no vpath set");
	}
	else
	{
		struct CVhalLayoutVideoSetting::VideoRectangleData output{};
		output.x_      = x;
		output.y_      = y;
		output.width_  = w;
		output.height_ = h;
		ret = video_setting_.SetVideoOutputSize(video_path_current, output_target, output);
		if( VHAL_SUCCESS != ret )
		{
			VHAL_LOGE("SetVideoOutputSize error ret=%d path=%s", ret, video_path_current.c_str());
		}

		int32_t surface_id{0};
		ret = GetSurfaceIdFromVideoPath(video_path_current, output_target, surface_id);
		if( VHAL_SUCCESS == ret )
		{
			bool	setting_enable{true};
			/* カメラのみ描画されていない状態であればサイズ更新を行わない */
			if( VIDEO_OUTPUT_TARGET_CAMERA == output_target)
			{
				uint32_t	frame{0U};
				frame = p_ivi_controller_->GetPropertiesOfSurfaceFrameCounter(surface_id);
				if (0U == frame)
				{
					setting_enable = false;
				}
			}

			if (true == setting_enable)
			{
				/* 映像スケーリングの表示矩形取得 */
				CVhalLayoutVideoSetting::VideoRectangleData out_src{output};
				CVhalLayoutVideoSetting::VideoRectangleData out_dest{output};
				ret = GetVideoScalingRectangleCommon(video_path_current, output_target, out_src, out_dest);
				if( VHAL_SUCCESS != ret )
				{
					VHAL_LOGE("GetVideoScalingRectangleCommon error. ret=%d, video_path=%s, output_target=0x%x", ret, video_path_current.c_str(), output_target);
				}

				/* 入力矩形設定 */
				ret = p_ivi_controller_->SetSurfaceSourceRectangle(surface_id, out_src.x_, out_src.y_, out_src.width_, out_src.height_);
				if( VHAL_SUCCESS != ret )
				{
					VHAL_LOGE("SetSurfaceSourceRectangle error. ret=%d", ret);
				}
				else
				{
					/* 出力矩形設定 */
					ret = p_ivi_controller_->SetSurfaceDestinationRectangle(surface_id, out_dest.x_, out_dest.y_, out_dest.width_, out_dest.height_);
					if( VHAL_SUCCESS != ret )
					{
						VHAL_LOGE("SetSurfaceDestinationRectangle error. ret=%d", ret);
					}
				}

				if(VHAL_SUCCESS == ret)
				{
					/* 映像MUTEサーフェスのサイズを追従させる */
					if( !mute_surface_name.empty() ) {
						ret = SetVideoBlindSurfaceRectangle(mute_surface_name, video_path_current, output_target);
						if( VHAL_SUCCESS != ret )
						{
							VHAL_LOGW("SetVideoBlindSurfaceRectangle error ret=%d, surface_name=%s", ret, mute_surface_name.c_str());
						}
						/* 前席映像パスの場合は前席映像同期面も追従させる */
						if (VIDEO_OUTPUT_TARGET_FRONT == output_target)
						{
							ret = SetVideoBlindSurfaceRectangle(kSurfaceNameMuteFrontVideoSync, video_path_current, output_target);
							if( VHAL_SUCCESS != ret )
							{
								VHAL_LOGW("SetVideoBlindSurfaceRectangle error ret=%d, surface_name=%s", ret, kSurfaceNameMuteFrontVideoSync.c_str());
							}
						}
					}
					/* 映像背景サーフェスのサイズを追従させる */
					if( !bg_surface_name.empty() )
					{
						ret = SetVideoBlindSurfaceRectangle(bg_surface_name, video_path_current, output_target);
						if( VHAL_SUCCESS != ret )
						{
							VHAL_LOGW("SetVideoBlindSurfaceRectangle error ret=%d, surface_name=%s", ret, bg_surface_name.c_str());
						}
					}

					(void)p_ivi_controller_->CommitChanges();
				}
			}
		}
		else
		{
			ret = VHAL_ERR_LAYOUT_INFO;
		}
	}

	return ret;
}

/*****************************************************************************
 処理概要：	映像pathの可視状態取得（共通）
 引数    ：	bool				&visibility			(o)可視状態
           	const std::string&	video_path_current	(i)対象映像vpath文字列
           	VideoOutputTarget	output_target		(i)映像出力先
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::GetVisibilityCommon(bool &visibility, const std::string& video_path_current, const VideoOutputTarget output_target) const
{
	int32_t ret{VHAL_SUCCESS};

	if(video_path_current.empty())
	{
		VHAL_LOGW("video_path is empty");
		visibility = false;
	}
	else 
	{
		int32_t surface_id{0};
		ret = GetSurfaceIdFromVideoPath(video_path_current, output_target, surface_id);
		if( VHAL_SUCCESS == ret )
		{
			ret = p_ivi_controller_->GetSurfaceVisibility(surface_id, visibility);
		}
		else
		{
			ret = VHAL_ERR_LAYOUT_INFO;
		}
	}

	return ret;
}

/*****************************************************************************
 処理概要：	映像pathの可視状態設定（共通）
 引数    ：	bool		visibility		(i)可視状態
           	const std::string&	video_path_current	(i)対象映像vpath文字列
            VideoOutputTarget output_target			(i)映像出力先
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::SetVisibilityCommon(const bool visibility, const std::string& video_path_current, const VideoOutputTarget output_target)
{
	int32_t ret{VHAL_SUCCESS};
	int32_t surface_id{0};

	ret = GetSurfaceIdFromVideoPath(video_path_current, output_target, surface_id);
	if( VHAL_SUCCESS == ret )
	{
		/* 映像可視状態の内部データ更新 */
		ret = video_setting_.SetVideoVisibility(video_path_current, output_target, visibility);
		if( VHAL_SUCCESS != ret )
		{
			VHAL_LOGE("SetVideoVisibility error ret=%d path=%s, output_target=0x%x", ret, video_path_current.c_str(), output_target);
		}

		ret = p_ivi_controller_->SetSurfaceVisibility(surface_id, visibility);
		if (VHAL_SUCCESS == ret)
		{
			(void)p_ivi_controller_->CommitChanges();
		}
	}
	else
	{
		ret = VHAL_ERR_LAYOUT_INFO;
	}

	return ret;
}

/*****************************************************************************
 処理概要：	映像pathの不透明度設定（共通）
 引数    ：	uint32_t	opacity		(i)不透明度(0-100)
           	const std::string&	video_path_current	(i)対象映像vpath文字列
            VideoOutputTarget output_target			(i)映像出力先
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::SetOpacityCommon(const uint32_t opacity, const std::string& video_path_current, const VideoOutputTarget output_target)
{
	int32_t ret{VHAL_SUCCESS};
	int32_t surface_id{0};

	ret = GetSurfaceIdFromVideoPath(video_path_current, output_target, surface_id);
	if( VHAL_SUCCESS == ret )
	{
		/* 映像不透明度の設定 */
		ret = video_setting_.SetVideoOpacity(video_path_current, output_target, opacity);
		if( VHAL_SUCCESS != ret )
		{
			VHAL_LOGE("SetVideoOpacity error ret=%d path=%s, output_target=0x%x", ret, video_path_current.c_str(), output_target);
		}
		else
		{
			ret = p_ivi_controller_->SetSurfaceOpacity(surface_id, opacity);
			if (VHAL_SUCCESS == ret)
			{
				(void)p_ivi_controller_->CommitChanges();
			}
		}
	}
	else
	{
		ret = VHAL_ERR_LAYOUT_INFO;
	}

	return ret;
}

/*****************************************************************************
 処理概要：	映像pathのワイド設定（共通）
 引数    ：	uint32_t				wide_mode			(i)ワイド設定(0-2)
           	const std::string&		video_path_current	(i)対象映像vpath文字列
            VideoOutputTarget		output_target		(i)映像出力先
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::SetWideModeCommon(const uint32_t wide_mode, const std::string& video_path_current, const VideoOutputTarget output_target)
{
	int32_t ret{VHAL_SUCCESS};

	/* 映像ワイドモードの設定 */
	ret = video_setting_.SetVideoWideMode(video_path_current, output_target, wide_mode);
	if( VHAL_SUCCESS != ret )
	{
		VHAL_LOGE("SetVideoWideMode error. ret=%d, video_path=%s, output_target=0x%x, wide_mode=%d", ret, video_path_current.c_str(), output_target, wide_mode);
	}
	else
	{
		/* 映像スケーリングの表示矩形取得 */
		CVhalLayoutVideoSetting::VideoRectangleData out_src{};
		CVhalLayoutVideoSetting::VideoRectangleData out_dest{};
		ret = GetVideoScalingRectangleCommon(video_path_current, output_target, out_src, out_dest);
		if( VHAL_SUCCESS != ret )
		{
			VHAL_LOGE("GetVideoScalingRectangleCommon error. ret=%d, video_path=%s, output_target=0x%x", ret, video_path_current.c_str(), output_target);
		}

		/* 映像パス名と映像出力先からサーフェスIDを取得 */
		int32_t surface_id{0};
		ret = GetSurfaceIdFromVideoPath(video_path_current, output_target, surface_id);
		if( VHAL_SUCCESS == ret )
		{
			/* 入力矩形設定 */
			(void)p_ivi_controller_->SetSurfaceSourceRectangle(surface_id, out_src.x_, out_src.y_, out_src.width_, out_src.height_);
			/* 出力矩形設定 */
			(void)p_ivi_controller_->SetSurfaceDestinationRectangle(surface_id, out_dest.x_, out_dest.y_, out_dest.width_, out_dest.height_);
			/* 表示設定 */
			(void)p_ivi_controller_->CommitChanges();
		}
		else
		{
			ret = VHAL_ERR_LAYOUT_INFO;
		}
	}

	return ret;
}

/*****************************************************************************
 処理概要：	映像スケーリングの表示矩形取得（共通）
 引数    ：	const std::string&									video_path_current	(i)対象映像vpath文字列
            VideoOutputTarget									output_target		(i)映像出力先
           	struct CVhalLayoutVideoSetting::VideoRectangleData 	&out_src			(o)ソース短形
           	struct CVhalLayoutVideoSetting::VideoRectangleData 	&out_dest			(o)デスティネーション短形
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::GetVideoScalingRectangleCommon(const std::string& video_path_current, const VideoOutputTarget output_target, struct CVhalLayoutVideoSetting::VideoRectangleData &out_src, struct CVhalLayoutVideoSetting::VideoRectangleData &out_dest) const
{
	int32_t ret{VHAL_SUCCESS};

	/* 映像入力ソース矩形取得 */
	bool clipping_enable{false};
	CVhalLayoutVideoSetting::VideoRectangleData in_src{};
	ret = GetSourceRectangleCommon(clipping_enable, in_src.x_, in_src.y_, in_src.width_, in_src.height_, video_path_current, output_target);
	if( VHAL_SUCCESS != ret )
	{
		VHAL_LOGE("GetSourceRectangleCommon error. ret=%d, video_path=%s, output_target=0x%x", ret, video_path_current.c_str(), output_target);
		return ret;
	}

	/* 映像入力デスティネーション矩形取得 */
		CVhalLayoutVideoSetting::VideoRectangleData in_dest{};
	ret = GetDestRectangleCommon(in_dest.x_, in_dest.y_, in_dest.width_, in_dest.height_, video_path_current, output_target);
	if( VHAL_SUCCESS != ret )
	{
		VHAL_LOGE("GetDestRectangleCommon error. ret=%d, output_target=0x%x", ret, output_target);
		return ret;
	}

	/* ワイド設定の短形取得 */
	ret = video_setting_.GetVideoWideRectangle(video_path_current, output_target, in_src, in_dest, out_src, out_dest);
	if( VHAL_SUCCESS != ret )
	{
		VHAL_LOGE("GetVideoWideRectangle error. ret=%d, video_path=%s, output_target=0x%x", ret, video_path_current.c_str(), output_target);
		return ret;
	}

	return ret;
}

/*****************************************************************************
 処理概要：	映像ブラインドサーフェス矩形設定
           	[POST21MM-53784]カメラMUTEのみWIDE設定を考慮する(WIDE設定は初回出力サイズが設定されるまで有効)
 引数    ：	const std::string&	surface_name		(i)ブラインドサーフェス名
         ：	const std::string&	video_path_next		(i)変更後vpath文字列
         ：	VideoOutputTarget 	output_target		(i)映像出力先
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM			パラメータ不正
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::SetVideoBlindSurfaceRectangle(const std::string& surface_name, const std::string& video_path_next, const VideoOutputTarget output_target)
{
	int32_t result{VHAL_SUCCESS};

	VHAL_LOGD("surface_name=%s, video_path=%s, output_target=0x%x", surface_name.c_str(), video_path_next.c_str(), output_target);

	if (surface_name.empty())
	{
		VHAL_LOGE("parameter error. surface_name is empty");
		result = VHAL_ERR_PARAM;
	}
	else if (video_path_next.empty())
	{
		VHAL_LOGE("parameter error. video_path_next is empty");
		result = VHAL_ERR_PARAM;
	}
	else if (VIDEO_OUTPUT_TARGET_NONE == output_target)
	{
		VHAL_LOGE("parameter error. output_target is none");
		result = VHAL_ERR_PARAM;
	}
	else
	{
		/* ブラインダーのサーフェスサイズを、新しい映像パスサーフェスサイズに追従させる */
		int32_t blind_x{0};
		int32_t blind_y{0};
		int32_t blind_w{0};
		int32_t blind_h{0};
		int32_t ret{GetDestRectangleCommon(blind_x, blind_y, blind_w, blind_h, video_path_next, output_target)};
		if (VHAL_SUCCESS == ret)
		{
			/* [POST21MM-53784]カメラMUTEのみWIDE設定を考慮する(WIDE設定は初回出力サイズが設定されるまで有効) */
			if (VIDEO_OUTPUT_TARGET_CAMERA == output_target)
			{
				/* ワイド設定がNORMALならINVALIDとする */
				uint32_t wide_mode{VHAL_WIDE_MODE_NORMAL};
				ret = GetCameraWideMode(wide_mode);
				if (VHAL_SUCCESS == ret)
				{
					if (VHAL_WIDE_MODE_NORMAL == wide_mode)
					{
						CVhalLayoutVideoSetting::VideoRectangleData out_src{};
						CVhalLayoutVideoSetting::VideoRectangleData out_dest{};
						ret = GetVideoScalingRectangleCommon(video_path_next, output_target, out_src, out_dest);
						if (VHAL_SUCCESS == ret)
						{
							blind_x = out_dest.x_;
							blind_y = out_dest.y_;
							blind_w = out_dest.width_;
							blind_h = out_dest.height_;
						}
						else
						{
							/* MUTEのワイド設定エラーのため処理は継続させる */
							VHAL_LOGW("GetVideoScalingRectangleCommon error. ret=%d, video_path=%s, output_target=0x%x", ret, video_path_next.c_str(), output_target);
						}
					}
				}
				else
				{
					/* MUTEのワイド設定エラーのため処理は継続させる */
					VHAL_LOGW("GetCameraWideMode error. ret=%d", ret);
				}
			}
			
			CVhalLayoutSurfaceProperty blinder_prop{};
			ret = p_layout_file_->GetSurfaceProperty(surface_name, blinder_prop);
			if (VHAL_SUCCESS == ret)
			{
				const int32_t surface_id{blinder_prop.GetIviId()};
				(void)p_ivi_controller_->SetSurfaceDestinationRectangle(surface_id, blind_x, blind_y, blind_w, blind_h);
			}
			else
			{
				VHAL_LOGE("surface_name=%s, ret=%d", surface_name.c_str(), ret);
				result = ret;
			}
		}
		else
		{
			VHAL_LOGE("GetDestRectangleCommon error. ret=%d, output_target=0x%x", ret, output_target);
			result = ret;
		}
		VHAL_LOGD("DestRectangle Setting Result. result=%d, output_target=0x%x, rect[%d,%d,%d,%d]", result, output_target, blind_x, blind_y, blind_w, blind_h);
	}

	return result;
}

/*****************************************************************************
 処理概要：	映像ブラインドサーフェス表示設定
 引数    ：	const std::string&	surface_name		(i)ブラインドサーフェス名
         ：	const std::string&	video_path_current	(i)変更前vpath文字列
         ：	const std::string&	video_path_next		(i)変更後vpath文字列
         ：	VideoOutputTarget 	output_target		(i)映像出力先
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM			パラメータ不正
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::SetVideoBlindSurfaceVisibility(const std::string& surface_name, const std::string& video_path_current, const std::string& video_path_next, const VideoOutputTarget output_target)
{
	VHAL_LOGD("surface_name=%s, video_path_current=%s, video_path_next=%s, output_target=0x%x", surface_name.c_str(), video_path_current.c_str(), video_path_next.c_str(), output_target);

	/* 映像パス変更判定 */
	if (0 == video_path_current.compare(video_path_next))
	{
		/* 映像パスに変化が無い場合はスキップ */
		VHAL_LOGD("already set video_path=%s", video_path_next.c_str());
		return VHAL_SUCCESS;
	}

	if (surface_name.empty())
	{
		VHAL_LOGE("parameter error. surface_name is empty");
		return VHAL_ERR_PARAM;
	}
	CVhalLayoutSurfaceProperty blinder_prop{};
	int32_t ret{p_layout_file_->GetSurfaceProperty(surface_name, blinder_prop)};
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("surface_name=%s, ret=%d", surface_name.c_str(), ret);
		return VHAL_ERR_PARAM;
	}

	/* 表示設定 */
	const int32_t surface_id{blinder_prop.GetIviId()};
	bool visible{false};
	if (!video_path_next.empty())
	{
		/* 映像可視状態の取得 */
		bool visibility{false};
		ret = video_setting_.GetVideoVisibility(video_path_next, output_target, visibility);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("GetVideoVisibility error ret=%d path=%s, output_target=0x%x", ret, video_path_next.c_str(), output_target);
		}

		/* 映像不透明度の取得 */
		uint32_t opacity{0U};
		ret = video_setting_.GetVideoOpacity(video_path_next, output_target, opacity);
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("GetVideoOpacity error ret=%d path=%s, output_target=0x%x", ret, video_path_next.c_str(), output_target);
		}

		/* 映像表示の場合は映像ブラインドサーフェスを表示 */
		VHAL_LOGD("video_path=%s, visibility=%d, opacity=%d", video_path_next.c_str(), visibility, opacity);
		if ((true == visibility) && (VHAL_OPACITY_VALUE_MAX == opacity))
		{
			visible = true;
		}
	}
	(void)p_ivi_controller_->SetSurfaceVisibility(surface_id, visible);
	(void)p_ivi_controller_->CommitChanges();

	return ret;
}

/*****************************************************************************
 処理概要：	出力デバイス追加通知
 引数    ：	int32_t width				(i) 出力デバイス先の画面幅
          	int32_t height				(i) 出力デバイス先の画面高さ
          	const std::string& model	(i) 出力デバイス先のモデル名
          	const std::string& make		(i) 出力デバイス先の製造名
 戻り値  ：	なし
*****************************************************************************/
void CVhalLayoutManager::NotifyOutputAdded(const int32_t width, const int32_t height, const std::string& model, const std::string& make)
{
	VHAL_LOGI("add output width=%d height=%d model=%s make=%s", width, height, model.c_str(), make.c_str());

	/* 初回の通知内容で、make名、model名を参照するかを決める */
	if( ScreenBuildType::SCREEN_BUILD_TYPE_INVALID == screen_build_type_ )
	{
		if( (model.empty()) && (make.empty()) )
		{
			screen_build_type_ = ScreenBuildType::SCREEN_BUILD_TYPE_NOTIFY;
		}
		else
		{
			screen_build_type_ = ScreenBuildType::SCREEN_BUILD_TYPE_MODEL_MAKE;
		}
		VHAL_LOGI("screen build type=%d", screen_build_type_);
	}

	int32_t screen_id{-1};
	if( ScreenBuildType::SCREEN_BUILD_TYPE_MODEL_MAKE == screen_build_type_ )
	{
		std::string screen_name{""};					/* スクリーン名 */
		/* make名からスクリーン名を取得する */
		const auto it_find_make = std::find_if(screen_info_list_.begin(), screen_info_list_.end(),
			[make](const struct ScrInfo make_screen_data) noexcept -> bool { return 0 == make_screen_data.GetMakeName().compare(make); });
		if( (it_find_make != screen_info_list_.end()) && (!make.empty()) )
		{
			screen_name = it_find_make->GetScreenName();
		}
		else
		{
			/* make名でスクリーン名が取得できなかった場合はmodel名で再度スクリーン名を取得する */
			const auto it_find_model = std::find_if(screen_info_list_.begin(), screen_info_list_.end(),
				[model](const struct ScrInfo model_screen_data) noexcept -> bool { return 0 == model_screen_data.GetModelName().compare(model); });
			if( (it_find_model != screen_info_list_.end()) && (!model.empty()) )
			{
				screen_name = it_find_model->GetScreenName();
			}
			else
			{
				VHAL_LOGE("error not found make name=%s, model name=%s", make.c_str(), model.c_str());
			}
		}
		
		if(!screen_name.empty())
		{
			/* レイヤ構成ファイルのスクリーン情報取得 */
			CVhalLayoutScreenProperty screen_prop{};
			const int32_t ret{p_layout_file_->GetScreenProperty(screen_name, screen_prop)};
			if( VHAL_SUCCESS != ret )
			{
				VHAL_LOGE("error not found screen ret=%d name=%s", ret, screen_name.c_str());
			}
			else
			{
				screen_id = screen_prop.GetIviId();
			}
		}
	}
	else if( ScreenBuildType::SCREEN_BUILD_TYPE_NOTIFY == screen_build_type_ )
	{
		std::vector<int32_t> screen_ids{};
		(void)p_layout_file_->GetScreenIviidList(screen_ids);
		if( screen_id_list_.size() < screen_ids.size() )
		{
			screen_id = static_cast<int32_t>(screen_ids.at(screen_id_list_.size()));
		}
		else
		{
			VHAL_LOGE("error screen layout count=%ld", screen_id_list_.size());
		}
	}
	else
	{
		/* 処理なし */
	}

	if( 0 <= screen_id )
	{
		/* スクリーンのレイアウト構築 */
		int32_t ret{SetLayoutScreen(screen_id, width, height)};
		if( VHAL_SUCCESS != ret )
		{
			VHAL_LOGE("SetLayoutScreen error. ret=%d", ret);
		}
		else
		{
			/* サーフェス一覧取得 */
			std::vector<int32_t> surface_ids{};
			ret = p_ivi_controller_->GetSurfaceIds(surface_ids);
			if( VHAL_SUCCESS != ret )
			{
				VHAL_LOGE("GetSurfaceIds error. ret=%d", ret);
			}
			else
			{
				/* 新しいスクリーンに登録すべきサーフェスのみを登録 */
				for( auto itr_id = surface_ids.begin(); itr_id != surface_ids.end(); ++itr_id )
				{
					CVhalLayoutSurfaceProperty surface_prop{};
					ret = p_layout_file_->GetSurfaceProperty(*itr_id, surface_prop);
					if( VHAL_SUCCESS == ret )
					{
						const int32_t layer_id{surface_prop.GetParentLayerIviId()};
						CVhalLayoutLayerProperty layer_prop{};
						(void)p_layout_file_->GetLayerProperty(layer_id, layer_prop);
						if( screen_id == layer_prop.GetParentScreenIviId() )
						{
							/* スクリーンに登録対象のサーフェス */
							(void)AddSurfaceToLayer(*itr_id);
						}
					}
				}
			}
		}
	}

}

/*****************************************************************************
 処理概要：	スクリーン有効判定
 引数    ：	int32_t screen_id			(i) スクリーンID
 戻り値  ：	true:有効 false:無効
*****************************************************************************/
bool CVhalLayoutManager::IsScreenAvailable(const int32_t screen_id) const
{
	bool available{false};

	// スクリーンのレイアウト構築済みかをチェック
	const auto itr_screen_id = std::find(screen_id_list_.begin(), screen_id_list_.end(), screen_id);
	if( itr_screen_id != screen_id_list_.end() )
	{
		available = true;
	}
	return available;
}

/*****************************************************************************
 処理概要：	スクリーンサイズ取得
 引数    ：	int32_t screen_id			(i) スクリーンID
          	int32_t &width				(o) スクリーンの幅
          	int32_t &height				(o) スクリーンの高さ
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::GetScreenSize(const int32_t screen_id, int32_t &width, int32_t &height) const
{
	// スクリーンサイズを取得
	int32_t w{0};
	int32_t h{0};
	int32_t ret{VHAL_SUCCESS};

	ret = p_ivi_controller_->GetScreenResolution(screen_id, w, h);
	if ( VHAL_SUCCESS != ret )
	{
		VHAL_LOGE("error get screen resolution id=%d", screen_id);
		return VHAL_ERR_LAYOUT_INFO;
	}
	width  = w;
	height = h;

	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	映像パス名と映像出力先からサーフェスIDを取得
 引数    ：	const std::string& video_path		(i) 映像パス名
          	VideoOutputTarget output_target		(i) 映像出力先
          	int32_t& surface_id					(o) サーフェスID
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM			パラメータ不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::GetSurfaceIdFromVideoPath(const std::string& video_path, const VideoOutputTarget output_target, int32_t& surface_id) const
{
	if( video_path.empty() )
	{
		VHAL_LOGW("video_path is empty");
		return VHAL_ERR_PARAM;
	}

	/* 映像パス情報から映像パスに紐づいたサーフェス名を取得 */
	const auto it_vpath_info = videopath_info_list_.find(video_path);
	if( it_vpath_info == videopath_info_list_.end() )
	{
		VHAL_LOGE("not found videopath info video_path=%s output_target=%d", video_path.c_str(), output_target);
		return VHAL_ERR_PARAM;
	}
	std::string surface_name{};
	bool is_supported{true};
	switch( output_target )
	{
	case VIDEO_OUTPUT_TARGET_FRONT:
		surface_name = it_vpath_info->second.GetSurfaceFront();
		break;
	case VIDEO_OUTPUT_TARGET_REAR:
		surface_name = it_vpath_info->second.GetSurfaceRear();
		break;
	case VIDEO_OUTPUT_TARGET_CAMERA:
		surface_name = it_vpath_info->second.GetSurfaceCamera();
		break;
	case VIDEO_OUTPUT_TARGET_IC:
		surface_name = it_vpath_info->second.GetSurfaceIc();
		break;
	case VIDEO_OUTPUT_TARGET_NONE:
	default:
		is_supported = false;
		VHAL_LOGE("not support output target video_path=%s output_target=%d", video_path.c_str(), output_target);
		break;
	}

	if (false == is_supported)
	{
		return VHAL_ERR_PARAM;
	}

	/* サーフェスのプロパティ取得（サーフェス名は固有のため、特定可能） */
	CVhalLayoutSurfaceProperty surface_prop{};
	const int32_t ret{p_layout_file_->GetSurfaceProperty(surface_name, surface_prop)};
	if( VHAL_SUCCESS != ret )
	{
		VHAL_LOGE("GetSurfaceProperty error ret=%d video_path=%s output_target=%d surface_name=%s", ret, video_path.c_str(), output_target, surface_name.c_str());
		return ret;
	}

	surface_id = surface_prop.GetIviId();

	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	映像サーフェス名から映像パス名/映像出力先を取得
 引数    ：	const std::string& surface_name		(i) サーフェス名
          	std::string& video_path				(o) 映像パス名
          	VideoOutputTarget& output_target	(o) 映像出力先
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM			パラメータ不正（映像パスに紐づいたサーフェスではない）
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::GetVideoPathFromSurfaceName(const std::string& surface_name, std::string& video_path, VideoOutputTarget& output_target) const
{
	int32_t ret{VHAL_ERR_PARAM};

	if( surface_name.empty() )
	{
		VHAL_LOGW("surface_name is empty");
	}
	else
	{
		bool done{false};
		for( auto it_info=videopath_info_list_.begin(); (!done) && (it_info!=videopath_info_list_.end()); ++it_info )
		{
			if( false == it_info->first.empty() )
			{
				if(0 == (surface_name.compare(it_info->second.GetSurfaceFront())))
				{
					video_path = it_info->first;
					output_target = VIDEO_OUTPUT_TARGET_FRONT;
					ret = VHAL_SUCCESS;
					done = true;
				}
				else if(0 == (surface_name.compare(it_info->second.GetSurfaceRear())))
				{
					video_path = it_info->first;
					output_target = VIDEO_OUTPUT_TARGET_REAR;
					ret = VHAL_SUCCESS;
					done = true;
				}
				else if(0 == (surface_name.compare(it_info->second.GetSurfaceCamera())))
				{
					video_path = it_info->first;
					output_target = VIDEO_OUTPUT_TARGET_CAMERA;
					ret = VHAL_SUCCESS;
					done = true;
				}
				else if(0 == (surface_name.compare(it_info->second.GetSurfaceIc())))
				{
					video_path = it_info->first;
					output_target = VIDEO_OUTPUT_TARGET_IC;
					ret = VHAL_SUCCESS;
					done = true;
				}
				else
				{
					output_target = VIDEO_OUTPUT_TARGET_NONE;
				}
			}
		}
	}

	return ret;
}

/*****************************************************************************
 処理概要：	サーフェスID取得（共通）
 引数    ：	const std::string&	surface_name	(i)サーフェス名
           	int32_t&			surface_id		(o)サーフェスID
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::GetSurfaceIdCommon(const std::string& surface_name, int32_t& surface_id) const
{
	int32_t result{VHAL_SUCCESS};
	CVhalLayoutSurfaceProperty surface_prop{};

	const int32_t ret{p_layout_file_->GetSurfaceProperty(surface_name, surface_prop)};
	if (VHAL_SUCCESS == ret)
	{
		surface_id = surface_prop.GetIviId();
	}
	else
	{
		VHAL_LOGE("not found surface=%s, ret=%d", surface_name.c_str(), ret);
		result = ret;
	}

	return result;
}

/*****************************************************************************
 処理概要：	サーフェス設定可否チェック
 引数    ：	const std::string	&video_path		(i)映像vpath文字列
         ：	VideoOutputTarget 	output_target	(i)映像出力先
 戻り値  ：	処理結果
           		false		設定不可
           		true		設定可
****************************************************************************/
bool CVhalLayoutManager::IsValidSurfaceAvailable(const std::string &video_path, const VideoOutputTarget output_target) const
{
	bool available{false};
	int32_t ret{VHAL_SUCCESS};
	int32_t surface_id{0};

	/* 映像パス名と映像出力先からサーフェスIDを取得 */
	ret = GetSurfaceIdFromVideoPath(video_path, output_target, surface_id);
	if( VHAL_SUCCESS != ret )
	{
		VHAL_LOGE("not found surface. ret=%d, video_path=%s, output_target=0x%x", ret, video_path.c_str(), output_target);
		return available;
	}

	/* サーフェス一覧取得 */
	std::vector<int32_t> surface_ids{};
	ret = p_ivi_controller_->GetSurfaceIds(surface_ids);
	if( VHAL_SUCCESS != ret )
	{
		VHAL_LOGE("GetSurfaceIds. ret=%d", ret);
		return available;
	}

	/* サーフェスが生成されているかチェック */
	const auto itr_surface_id = std::find(surface_ids.begin(), surface_ids.end(), surface_id);
	if( itr_surface_id != surface_ids.end() )
	{
		available = true;
	}

	VHAL_LOGD("available=%d", available);
	return available;
}

/*****************************************************************************
 処理概要：	サーフェス設定可否チェック
 引数    ：	const int32_t	surface_id	(i)サーフェスID
 戻り値  ：	処理結果
           		false		設定不可
           		true		設定可
****************************************************************************/
bool CVhalLayoutManager::IsValidSurfaceIdAvailable(const int32_t surface_id) const
{
	bool available{false};
	int32_t ret{VHAL_SUCCESS};
	
	/* サーフェス一覧取得 */
	std::vector<int32_t> surface_ids{};
	ret = p_ivi_controller_->GetSurfaceIds(surface_ids);
	if( VHAL_SUCCESS == ret )
	{
		/* サーフェスが生成されているかチェック */
		const auto itr_surface_id{std::find(surface_ids.begin(), surface_ids.end(), surface_id)};
		if( itr_surface_id != surface_ids.end() )
		{
			available = true;
		}
	}
	else
	{
		VHAL_LOGE("GetSurfaceIds. ret=%d", ret);
	}

	VHAL_LOGD("available=%d", available);
	return available;
}

/*****************************************************************************
 処理概要：	イベントリスナー登録
 引数    ：	CVhalLayoutManagerEventListenerBase* p_listener		(i)リスナーポインタ
 戻り値  ：	処理結果
           		VHAL_ERR_PARAM			パラメータ不正
           		VHAL_SUCCESS			正常終了
****************************************************************************/
int32_t CVhalLayoutManager::RegisterEventListener(CVhalLayoutManagerEventListenerBase* const p_listener)
{
	if (nullptr == p_listener)
	{
		VHAL_LOGE("p_listener is null");
		return VHAL_ERR_PARAM;
	}

	p_layoutManager_listener_ = p_listener;

	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	イベントリスナー解除
 引数    ：	なし
 戻り値  ：	なし
****************************************************************************/
void CVhalLayoutManager::ClearEventListener(void) noexcept
{
	p_layoutManager_listener_ = nullptr;
}

/*****************************************************************************
 処理概要：	現在のRSE種別の取得
 引数    ：	int32_t& rse		(o) RSE種別
*****************************************************************************/
void CVhalLayoutManager::GetConnectedRse(int32_t& rse) const noexcept
{
	rse = connected_rse_;
}

/*****************************************************************************
 処理概要：	現在のRSE種別の設定
 引数    ：	int32_t rse			(i) RSE種別
*****************************************************************************/
void CVhalLayoutManager::SetConnectedRse(const int32_t rse) noexcept
{
	connected_rse_ = rse;
}

/*****************************************************************************
 処理概要：	前席スクリーンID取得
 引数    ：	int32_t&	screen_id	(o)スクリーンID
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::GetScreenIdFront(int32_t &screen_id) const
{
	int32_t result{VHAL_SUCCESS};

	/* 前席スクリーンID */
	result = GetScreenIdCommon(kScreenNameFront, screen_id);
	return result;
}

/*****************************************************************************
 処理概要：	後席スクリーンID取得
 引数    ：	int32_t&	screen_id	(o)スクリーンID
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::GetScreenIdRear(int32_t &screen_id) const
{
	int32_t result{VHAL_SUCCESS};

	/* 後席スクリーンID */
	result = GetScreenIdCommon(kScreenNameRear, screen_id);
	return result;
}

/*****************************************************************************
 処理概要：	Instrument Cluster スクリーンID取得
 引数    ：	int32_t&	screen_id	(o)スクリーンID
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::GetScreenIdICluster(int32_t &screen_id) const
{
	int32_t result{VHAL_SUCCESS};

	/* Instrument Cluster スクリーンID */
	result = GetScreenIdCommon(kScreenNameICluster, screen_id);
	return result;
}

/*****************************************************************************
 処理概要：	HUD スクリーンID取得
 引数    ：	int32_t&	screen_id	(o)スクリーンID
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::GetScreenIdHud(int32_t &screen_id) const
{
	int32_t result{VHAL_SUCCESS};

	/* HUD スクリーンID */
	result = GetScreenIdCommon(kScreenNameHud, screen_id);
	return result;
}

/*****************************************************************************
 処理概要：	スクリーンID取得（共通）
 引数    ：	const std::string&	screen_name	(i)スクリーン名
           	int32_t&			screen_id	(o)スクリーンID
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutManager::GetScreenIdCommon(const std::string& screen_name, int32_t &screen_id) const
{
	int32_t result{VHAL_SUCCESS};
	CVhalLayoutScreenProperty screen_prop{};

	const int32_t ret{p_layout_file_->GetScreenProperty(screen_name, screen_prop)};
	if (VHAL_SUCCESS == ret)
	{
		screen_id = screen_prop.GetIviId();
	}
	else
	{
		VHAL_LOGE("not found screen=%s, ret=%d", screen_name.c_str(), ret);
		result = ret;
	}

	return result;
}

/*****************************************************************************
 処理概要：	前席映像クリッピングサイズクリア
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
           		VHAL_SUCEESS以外		異常終了
*****************************************************************************/
int32_t CVhalLayoutManager::ClearFrontClippingSize(void)
{
	const int32_t ret{video_setting_.ClearVideoInputClippingSize(video_front_path_current_, VIDEO_OUTPUT_TARGET_FRONT)};
	return ret;
}

/*****************************************************************************
 処理概要：	Instrument Cluster 映像クリッピングサイズクリア
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
           		VHAL_SUCEESS以外		異常終了
*****************************************************************************/
int32_t CVhalLayoutManager::ClearIClusterClippingSize(void)
{
	const int32_t ret{video_setting_.ClearVideoInputClippingSize(video_icluster_path_current_, VIDEO_OUTPUT_TARGET_IC)};
	return ret;
}

/*****************************************************************************
 処理概要：	前席映像同期面MUTE設定
 引数    ：	const VhalMuteFrontReason	reason		(i)前席映像同期面MUTE理由
         ：	const bool					mute		(i)MUTE状態(true有効／false無効)
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
           		VHAL_SUCEESS以外		異常終了
*****************************************************************************/
int32_t CVhalLayoutManager::SetBrinderSync(const VhalMuteFrontReason reason, const bool mute)
{
	int32_t			result{VHAL_SUCCESS};
	VHAL_LOGV_IN();

	std::size_t		pos{static_cast<std::size_t>(reason)};

	if (pos < mute_front_reason_.size())
	{
		if (true == mute)
		{
			/* MUTE-ON実施 */
			mute_front_reason_.set(pos);
			result = SetBlinderEnable(VhalBlinderType::VHAL_BLINDER_TYPE_FRONT_VIDEO_SYNC, mute);
		}
		else
		{
			/* 全ビットがOFFになるならばMUTE-OFF実施 */
			mute_front_reason_.reset(pos);
			if (mute_front_reason_.none())
			{
				result = SetBlinderEnable(VhalBlinderType::VHAL_BLINDER_TYPE_FRONT_VIDEO_SYNC, mute);
			}
		}
	}
	else
	{
		result = VHAL_ERR_PARAM;
	}

	if (VHAL_SUCCESS != result)
	{
		VHAL_LOGE("mute failed. result=%d mute=%s reason=%llu size=%llu", result, BToCh(mute), pos, mute_front_reason_.size());
	}

	VHAL_LOGV_OUT();
	return result;
}

} /* namespace videohal */

