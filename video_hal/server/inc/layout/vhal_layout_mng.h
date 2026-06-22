/*******************************************************************************
    機能名称    ：  レイアウト制御モジュール
    ファイル名称：  vhal_layout_mng.h
*******************************************************************************/
#ifndef	VHAL_LAYOUT_MNG_H
#define	VHAL_LAYOUT_MNG_H

#include <string>
#include <vector>
#include <map>
#include <bitset>
#include "vhal_layout_file.h"
#include "vhal_layout_video_setting.h"

namespace videohal
{
/* 前方宣言 */
enum class ScreenShotType : int32_t;

enum class VhalBlinderType : int32_t {
	VHAL_BLINDER_TYPE_NONE,
	VHAL_BLINDER_TYPE_FRONT_DISPLAY,
	VHAL_BLINDER_TYPE_FRONT_VIDEO,
	VHAL_BLINDER_TYPE_REAR_DISPLAY,
	VHAL_BLINDER_TYPE_REAR_VIDEO,
	VHAL_BLINDER_TYPE_CAMERA,
	VHAL_BLINDER_TYPE_FRONT_VIDEO_BG,
	VHAL_BLINDER_TYPE_REAR_VIDEO_BG,
	VHAL_BLINDER_TYPE_IC_DISPLAY,
	VHAL_BLINDER_TYPE_FRONT_VIDEO_SYNC,
	VHAL_BLINDER_TYPE_HUD_DISPLAY,
};

/* 前席映像同期面MUTE理由 */
enum class VhalMuteFrontReason : uint32_t {
	VHAL_MUTE_FRONT_REASON_HDMI = 0U,
	VHAL_MUTE_FRONT_REASON_MAX
};

class CVhalLayoutConfigFile;
class CVhalMainControl;
class CVhalIviController;
class CVhalIviEventListener;


/*****************************************************************************
 クラス名称：CVhalLayoutManagerEventListenerBase
 処理概要  ：VhalLayoutManagerイベントリスナベース。
*****************************************************************************/
class CVhalLayoutManagerEventListenerBase {
public:
	CVhalLayoutManagerEventListenerBase(void) noexcept = default;
	virtual ~CVhalLayoutManagerEventListenerBase(void) = default;
  	CVhalLayoutManagerEventListenerBase(const CVhalLayoutManagerEventListenerBase& src) = delete;
	CVhalLayoutManagerEventListenerBase(CVhalLayoutManagerEventListenerBase&& src) = delete;

	virtual void NotifyVpathAvailable(const std::vector<std::string>& videopath_names, const VideoOutputTarget output_target, const bool created) const noexcept = 0;
	virtual void NotifyScreenAvailable(const int32_t screen_id, const bool display, const int32_t width, const int32_t height) const noexcept = 0;
	virtual void NotifyScreenShotResult(const int32_t result, const ScreenShotType type) const noexcept = 0;
	virtual void NotifyMovieResult(const int32_t result) const noexcept = 0;
private:
	CVhalLayoutManagerEventListenerBase& operator=(const CVhalLayoutManagerEventListenerBase& src) & = delete;
	CVhalLayoutManagerEventListenerBase& operator=(CVhalLayoutManagerEventListenerBase&& src) & = delete;
};

/*****************************************************************************
 クラス名称：CVhalLayoutManager
 処理概要  ：画面レイアウトの管理を行う。
*****************************************************************************/
class CVhalLayoutManager {
public:
	CVhalLayoutManager(void);
	virtual ~CVhalLayoutManager(void);
  	CVhalLayoutManager(const CVhalLayoutManager& src) = delete;
	CVhalLayoutManager& operator=(const CVhalLayoutManager& src) & = delete;
	CVhalLayoutManager(CVhalLayoutManager&& src) = delete;
	CVhalLayoutManager& operator=(CVhalLayoutManager&& src) & = delete;

	int32_t Initialize(CVhalMainControl * const p_main);
	void Finalize(void) noexcept;

	/* iviレイアウトの初期化 */
	int32_t InitLayout(const std::string& filePath);

	/* サーフェスをレイヤーへ登録 */
	int32_t AddSurfaceToLayer(const int32_t surfaceId);
	/* サーフェスをレイヤーから削除 */
	int32_t DeleteSurfaceToLayer(const int32_t surfaceId);
	/* サーフェスのconfigure処理の実施 */
	int32_t ConfigureSurface(const int32_t surfaceId, const int32_t width, const int32_t height);

	/* 有効な映像path判定（共通） */
	bool IsValidVideoPathCommon(const std::string& video_path_prev, const std::string& video_path_next, const VideoOutputTarget output_target, bool &already_path) const;

	/* 前席 有効な映像path判定 */
	bool IsValidVideoFrontPath(const std::string& video_path, bool &already_path) const;
	/* 現在の前席vpathの取得 */
	int32_t GetFrontPath(std::string& path) const;
	/* 現在の前席vpathの設定 */
	int32_t SetFrontPath(const std::string& path);
	/* 前席映像のソース矩形の取得 */
	int32_t GetFrontSourceRectangle(bool &clipping_enable, int32_t &x, int32_t &y, int32_t &w, int32_t &h) const;
	/* 前席映像のソース矩形の設定 */
	int32_t SetFrontSourceRectangle(const bool clipping_enable, const int32_t x, const int32_t y, const int32_t w, const int32_t h);
	/* 前席映像のデスティネーション矩形の設定 */
	int32_t SetFrontDestRectangle(const int32_t x, const int32_t y, const int32_t w, const int32_t h);
	/* 前席映像の可視状態の取得 */
	int32_t GetFrontVisibility(bool& visibility) const;
	/* 前席映像の可視状態の設定 */
	int32_t SetFrontVisibility(const bool visibility);
	/* 前席映像の不透明度設定 */
	int32_t SetFrontOpacity(const uint32_t opacity);
	/* 前席映像のワイド取得 */
	int32_t GetFrontWideMode(uint32_t &wide_mode) const;
	/* 前席映像のワイド設定 */
	int32_t SetFrontWideMode(const uint32_t wide_mode);

	/* 後席 有効な映像path判定 */
	bool IsValidVideoRearPath(const std::string& video_path, bool &already_path) const;
	/* 現在の後席vpathの取得 */
	int32_t GetRearPath(std::string& path) const;
	/* 現在の後席vpathの設定 */
	int32_t SetRearPath(const std::string& path);
	/* 後席映像のデスティネーション矩形の設定 */
	int32_t SetRearDestRectangle(const int32_t x, const int32_t y, const int32_t w, const int32_t h);
	/* 後席映像の可視状態の取得 */
	int32_t GetRearVisibility(bool& visibility) const;
	/* 後席映像の可視状態の設定 */
	int32_t SetRearVisibility(const bool visibility);
	/* 後席映像の不透明度設定 */
	int32_t SetRearOpacity(const uint32_t opacity);
	/* 後席映像のワイド取得 */
	int32_t GetRearWideMode(uint32_t &wide_mode) const;
	/* 後席映像のワイド設定 */
	int32_t SetRearWideMode(const uint32_t wide_mode);

	/* カメラ 有効な映像path判定 */
	bool IsValidVideoCameraPath(const std::string& video_path, bool &already_path) const;
	/* 現在のカメラpathの取得 */
	int32_t GetCameraPath(std::string& path) const;
	/* 現在のカメラpathの設定 */
	int32_t SetCameraPath(const std::string& path, const bool visibility);
	/* カメラ映像のデスティネーション矩形の取得 */
	int32_t GetCameraDestRectangle(int32_t &x, int32_t &y, int32_t &w, int32_t &h) const;
	/* カメラ映像のデスティネーション矩形の設定 */
	int32_t SetCameraDestRectangle(const int32_t x, const int32_t y, const int32_t w, const int32_t h);
	/* カメラ映像の入力サイズ強制設定 */
	int32_t SetCameraForceInputSize(const int32_t w, const int32_t h);
	/* カメラ映像のソース矩形の強制設定 */
	int32_t SetCameraForceSourceRectangle(const int32_t x, const int32_t y, const int32_t w, const int32_t h);
	/* カメラ映像のワイド取得 */
	int32_t GetCameraWideMode(uint32_t &wide_mode) const;
	/* カメラ映像のワイド設定 */
	int32_t SetCameraWideMode(const uint32_t wide_mode);

	/* Instrument Cluster 有効な映像path判定 */
	bool IsValidVideoIClusterPath(const std::string& video_path, bool &already_path) const;
	/* 現在のInstrument Cluster vpathの取得 */
	int32_t GetIClusterPath(std::string& path) const;
	/* 現在のInstrument Cluster vpathの設定 */
	int32_t SetIClusterPath(const std::string& path);
	/* Instrument Cluster 映像のソース矩形の取得 */
	int32_t GetIClusterSourceRectangle(bool &clipping_enable, int32_t &x, int32_t &y, int32_t &w, int32_t &h) const;
	/* Instrument Cluster 映像のソース矩形の設定 */
	int32_t SetIClusterSourceRectangle(const bool clipping_enable, const int32_t x, const int32_t y, const int32_t w, const int32_t h);
	/* Instrument Cluster 映像のデスティネーション矩形の設定 */
	int32_t SetIClusterDestRectangle(const int32_t x, const int32_t y, const int32_t w, const int32_t h);
	/* Instrument Cluster 映像の可視状態の取得 */
	int32_t GetIClusterVisibility(bool& visibility) const;
	/* Instrument Cluster 映像の可視状態の設定 */
	int32_t SetIClusterVisibility(const bool visibility);
	/* Instrument Cluster 映像の不透明度設定 */
	int32_t SetIClusterOpacity(const uint32_t opacity);

	/* 動画再生のデスティネーション矩形の設定 */
	int32_t SetMovieFrontDestRectangle(const int32_t x, const int32_t y, const int32_t w, const int32_t h) noexcept;

	/* iviスクリーン情報の取得 */
	std::string GetIviScreenInfo(std::map<std::string, std::string> &params) const;
	/* iviレイヤー情報の取得 */
	std::string GetIviLayerInfo(std::map<std::string, std::string> &params) const;
	/* iviサーフェス情報の取得 */
	std::string GetIviSurfaceInfo(std::map<std::string, std::string> &params) const;
	/* レイヤーのorder値の取得 */
	int32_t GetIviLayerOrder(std::map<std::string, std::string> &params, int64_t &value) const;
	/* レイヤーのorder値の設定 */
	int32_t SetIviLayerOrder(std::map<std::string, std::string> &params, const int64_t &value, bool &changed);
	/* iviスクリーンのスクリーンショット取得 */
	int32_t GetIviScreenshot(const int32_t ivi_id, const std::string &path) const;
	/* iviサーフェスのスクリーンショット取得 */
	int32_t GetIviSurfaceScreenshot(const int32_t ivi_id, const std::string &path) const;
	/* 画面キャプチャ結果通知 */
	void NotifyScreenShotResult(const int32_t result, const ScreenShotType type) const noexcept;

	/* ブラインダーの有効無効を切り替える */
	int32_t SetBlinderEnable(const VhalBlinderType blinder_type, const bool enable);
	/* ブラインダーのivi_idを取得する */
	int32_t GetBlinderID(const VhalBlinderType blinder_type, int32_t &ivi_id) const;

	/* 出力デバイス追加通知 */
	void NotifyOutputAdded(const int32_t width, const int32_t height, const std::string& model, const std::string& make);
	/* スクリーン有効判定 */
	bool IsScreenAvailable(const int32_t screen_id) const;
	/* スクリーンサイズ取得 */
	int32_t GetScreenSize(const int32_t screen_id, int32_t &width, int32_t &height) const;

	/* 映像パス名と映像出力先からサーフェスIDを取得 */
	int32_t GetSurfaceIdFromVideoPath(const std::string& video_path, const VideoOutputTarget output_target, int32_t& surface_id) const;
	/* 映像サーフェス名から映像パス名/映像出力先を取得 */
	int32_t GetVideoPathFromSurfaceName(const std::string& surface_name, std::string& video_path, VideoOutputTarget& output_target) const;
	/* サーフェス名からサーフェスIDを取得 */
	int32_t GetSurfaceIdCommon(const std::string& surface_name, int32_t& surface_id) const;

	/* サーフェス設定可否チェック */
	bool IsValidSurfaceAvailable(const std::string &video_path, const VideoOutputTarget output_target) const;
	/* サーフェス設定可否チェック(サーフェスIDで検索) */
	bool IsValidSurfaceIdAvailable(const int32_t surface_id) const;

	/* イベントリスナー登録 */
	int32_t RegisterEventListener(CVhalLayoutManagerEventListenerBase* const p_listener);
	/* イベントリスナー解除 */
	void ClearEventListener(void) noexcept;

	/* 前席スクリーンID取得 */
	int32_t GetScreenIdFront(int32_t &screen_id) const;
	/* 後席スクリーンID取得 */
	int32_t GetScreenIdRear(int32_t &screen_id) const;
	/* Instrument Cluster スクリーンID取得 */
	int32_t GetScreenIdICluster(int32_t &screen_id) const;
	/* HUD スクリーンID取得 */
	int32_t GetScreenIdHud(int32_t &screen_id) const;
	/* スクリーンID取得（共通） */
	int32_t GetScreenIdCommon(const std::string& screen_name, int32_t &screen_id) const;

	/* 現在のRSE種別の取得 */
	void GetConnectedRse(int32_t& rse) const noexcept;
	/* 現在のRSE種別の設定 */
	void SetConnectedRse(const int32_t rse) noexcept;
	/* 前席ディスプレイのMUTE初期設定 */
	void SetBlinderFrontDispMuteInit(const bool enable) noexcept;
	/* HUDディスプレイのMUTE初期設定 */
	void SetBlinderHudDispMuteInit(const bool enable) noexcept;

	/* 前席映像クリッピングサイズクリア */
	int32_t ClearFrontClippingSize(void);
	/*  Instrument Cluster 映像クリッピングサイズクリア */
	int32_t ClearIClusterClippingSize(void);
	/* 前席映像同期面MUTE設定 */
	int32_t SetBrinderSync(const VhalMuteFrontReason reason, const bool mute);

	/*HUD スクリーン有効判定 */
	bool IsHudScreenAvailable(void) const;

private:
	/* レイアウトファイルの読み込み */
	int32_t ReadLayoutFile(const std::string& filePath);
	/* スクリーンのレイアウト構築 */
	int32_t SetLayoutScreen(const int32_t screenId, const int32_t width, const int32_t height);

	/* ブラインダーのレイアウト初期設定 */
	int32_t ConfigureBlinder(const std::string &name, const int32_t surfaceId, const int32_t width, const int32_t height);
	
	/* 動画再生のレイアウト初期設定 */
	int32_t ConfigureMovie(const std::string &name, const int32_t surfaceId, const int32_t width, const int32_t height) const;

	/* パラメータからレイヤープロパティ取得 */
	int32_t GetLayerFromParams(std::map<std::string, std::string> &params, CVhalLayoutLayerProperty &layer_prop) const;

	/* 映像path設定（共通） */
	int32_t SetVideoPathCommon(const std::string& video_path_prev, const std::string& video_path_next, const std::string& mute_surface_name, const std::string& bg_surface_name, const VideoOutputTarget output_target, const bool visibility);
	/* 映像Mute解除設定（共通） */
	int32_t SetVideoMuteOffCommon(const std::string& video_path_prev, const std::string& video_path_next, const std::string& mute_surface_name);
	/* 映像pathの入力矩形取得（共通） */
	int32_t GetSourceRectangleCommon(bool &clipping_enable, int32_t &x, int32_t &y, int32_t &w, int32_t &h, const std::string& video_path_current, const VideoOutputTarget output_target) const;
	/* 映像pathの入力矩形設定（共通） */
	int32_t SetSourceRectangleCommon(const bool clipping_enable, const int32_t x, const int32_t y, const int32_t w, const int32_t h, const std::string& video_path_current, const VideoOutputTarget output_target);
	/* 映像pathの出力矩形取得（共通） */
	int32_t GetDestRectangleCommon(int32_t &x, int32_t &y, int32_t &w, int32_t &h, const std::string& video_path_current, const VideoOutputTarget output_target) const;
	/* 映像pathの出力矩形設定（共通） */
	int32_t SetDestRectangleCommon(const int32_t x, const int32_t y, const int32_t w, const int32_t h, const std::string& video_path_current, const std::string& mute_surface_name, const std::string& bg_surface_name, const VideoOutputTarget output_target);
	/* 映像pathの可視状態取得（共通） */
	int32_t GetVisibilityCommon(bool &visibility, const std::string& video_path_current, const VideoOutputTarget output_target) const;
	/* 映像pathの可視状態設定（共通） */
	int32_t SetVisibilityCommon(const bool visibility, const std::string& video_path_current, const VideoOutputTarget output_target);
	/* 映像pathの不透明度設定（共通） */
	int32_t SetOpacityCommon(const uint32_t opacity, const std::string& video_path_current, const VideoOutputTarget output_target);
	/* 映像pathのワイド設定（共通） */
	int32_t SetWideModeCommon(const uint32_t wide_mode, const std::string& video_path_current, const VideoOutputTarget output_target);
	/* 映像スケーリングの表示矩形取得（共通） */
	int32_t GetVideoScalingRectangleCommon(const std::string& video_path_current, const VideoOutputTarget output_target, struct CVhalLayoutVideoSetting::VideoRectangleData &out_src, struct CVhalLayoutVideoSetting::VideoRectangleData &out_dest) const;
	/* 映像ブラインドサーフェス矩形設定 */
	int32_t SetVideoBlindSurfaceRectangle(const std::string& surface_name, const std::string& video_path_next, const VideoOutputTarget output_target);
	/* 映像ブラインドサーフェス表示設定 */
	int32_t SetVideoBlindSurfaceVisibility(const std::string& surface_name, const std::string& video_path_current, const std::string& video_path_next, const VideoOutputTarget output_target);

	/* 映像出力パス設定 */
	int32_t GetOutputTargetFromLayerName(const std::string& layer_name, VideoOutputTarget& output_target) const;

	// スクリーンリスト<スクリーンID>
	using CScreenIdList = std::vector<int32_t>;

	// スクリーン情報データ構造体
	struct ScrInfo { /* MISRA C-2012 Rule 5.1対策により構造体名変更 */
	public:
		ScrInfo(const std::string &screen_name, const std::string &model_name, const std::string &make_name)
		:screen_name_(screen_name)
		,model_name_(model_name)
		,make_name_(make_name)
		{
		}

		std::string GetScreenName(void) const noexcept
		{
			return screen_name_;
		}

		std::string GetModelName(void) const noexcept
		{
			return model_name_;
		}

		std::string GetMakeName(void) const noexcept
		{
			return make_name_;
		}

	private:
		std::string		screen_name_;
		std::string		model_name_;
		std::string		make_name_;
	};
	// スクリーン情報リスト<スクリーン名, スクリーン情報>
	using CScreenInfoList = std::vector<struct ScrInfo>;
	static CScreenInfoList		screen_info_list_;

	// 映像パス各種情報データ構造体
	struct videoPathInfo {
	public:
		videoPathInfo(const std::string &surface_front, const std::string &surface_rear, const std::string &surface_camera,
						const std::string &surface_ic, const uint32_t output_target)
		:surface_front_(surface_front)
		,surface_rear_(surface_rear)
		,surface_camera_(surface_camera)
		,surface_ic_(surface_ic)
		,output_target_(output_target)
		{
		}

		std::string GetSurfaceFront(void) const noexcept
		{
			return surface_front_;
		}

		std::string GetSurfaceRear(void) const noexcept
		{
			return surface_rear_;
		}

		std::string GetSurfaceCamera(void) const noexcept
		{
			return surface_camera_;
		}

		std::string GetSurfaceIc(void) const noexcept
		{
			return surface_ic_;
		}

		uint32_t GetOutputTarget(void) const noexcept
		{
			return output_target_;
		}

	private:
		std::string		surface_front_;		/* サーフェス名 前席 */
		std::string		surface_rear_;		/* サーフェス名 後席 */
		std::string		surface_camera_;	/* サーフェス名 カメラ */
		std::string		surface_ic_;		/* サーフェス名 IC */
		uint32_t		output_target_;		/* 映像出力先(VideoOutputTarget) ※OR指定可 */
	};
	// 映像パス⇒各種情報変換テーブル<映像パス, 各種情報>
	using CVideoPathInfoList = std::map<std::string, struct videoPathInfo>;
	static CVideoPathInfoList	videopath_info_list_;

	//current映像パス
	std::string		video_front_path_current_;
	std::string		video_rear_path_current_;
	std::string		camera_path_current_;
	std::string		video_icluster_path_current_;

	//コンフィグファイルインスタンス
	std::unique_ptr<CVhalLayoutConfigFile>	p_layout_file_;

	CVhalLayoutVideoSetting	video_setting_;		/* 映像設定状態 */
	CScreenIdList		screen_id_list_;

	std::unique_ptr<CVhalIviController> p_ivi_controller_;
	std::unique_ptr<CVhalIviEventListener> p_ivi_listener_;
	CVhalMainControl *p_main_;
	CVhalLayoutManagerEventListenerBase* p_layoutManager_listener_;

	enum class ScreenBuildType : int32_t {
		SCREEN_BUILD_TYPE_INVALID,		/* 画面構築方法未判定 */
		SCREEN_BUILD_TYPE_NOTIFY,		/* 通知順番で画面構築 */
		SCREEN_BUILD_TYPE_MODEL_MAKE	/* model名、make名で画面構築 */
	};
	ScreenBuildType   screen_build_type_;

	int32_t	connected_rse_;	/* 接続されているRSE種別     */
	
	/* 動画再生パラメータ */
	int32_t		movie_x_;
	int32_t		movie_y_;
	int32_t		movie_width_;
	int32_t		movie_height_;

	/* 前席ディスプレイMUTE初期表示状態 */
	bool	front_disp_blinder_init_;
	/* HUDディスプレイMUTE初期表示状態 */
	bool	hud_disp_blinder_init_;

	/* 前席映像同期面MUTE理由 */
	std::bitset<static_cast<uint32_t>(VhalMuteFrontReason::VHAL_MUTE_FRONT_REASON_MAX)>	mute_front_reason_;

};


} /* namespace videohal */


#endif	/* #ifndef	VHAL_LAYOUT_MNG_H */
