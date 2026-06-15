/*******************************************************************************
    機能名称    ：  iviコントローラーモジュール
    ファイル名称：  vhal_ivi_controller.h
*******************************************************************************/
#ifndef	VHAL_IVI_CONTROLLER_H
#define	VHAL_IVI_CONTROLLER_H

#include <string>
#include <vector>
#include <mutex>
#include <memory>
#include <ilm/ilm_common.h>

#include "vhal_event_route.h"

namespace videohal
{

class CVhalMainControl;

/*****************************************************************************
 クラス名称：CVhalIviEventListenerBase
 処理概要  ：IVIイベントリスナベース。
*****************************************************************************/
class CVhalIviEventListenerBase {
public:
	enum class IviObjectType : int32_t {
		kIviObjectLayer,
		kIviObjectSurface
	};
	CVhalIviEventListenerBase(void) noexcept = default;
	virtual ~CVhalIviEventListenerBase(void) = default;
  	CVhalIviEventListenerBase(const CVhalIviEventListenerBase& src) = delete;
	CVhalIviEventListenerBase(CVhalIviEventListenerBase&& src) = delete;

	virtual void NotifyEvent(const IviObjectType objType, const int32_t id, const bool created) const noexcept = 0;
	virtual void NotifySurfaceConfigure(const int32_t surfaceId, const int32_t width, const int32_t height) const noexcept = 0;
private:
	CVhalIviEventListenerBase& operator=(const CVhalIviEventListenerBase& src) & = delete;
	CVhalIviEventListenerBase& operator=(CVhalIviEventListenerBase&& src) & = delete;
};

/*****************************************************************************
 クラス名称：CVhalIviController
 処理概要  ：ilmAPIを使ってwayland compositorへの設定を行う。
*****************************************************************************/
class CVhalIviController final {
public:
	// MISRA C++-2008 Rule 0-1-4 static constexpr int32_t kIlmObjectScreen{0};
	// MISRA C++-2008 Rule 0-1-4 static constexpr int32_t kIlmObjectLayer{1};
	// MISRA C++-2008 Rule 0-1-4 static constexpr int32_t kIlmObjectSurface{2};

	CVhalIviController(void);
	~CVhalIviController(void);
  	CVhalIviController(const CVhalIviController& src) = delete;
	CVhalIviController& operator=(const CVhalIviController& src) & = delete;
	CVhalIviController(CVhalIviController&& src) = delete;
	CVhalIviController& operator=(CVhalIviController&& src) & = delete;

	int32_t Initialize(CVhalMainControl * const p_main, CVhalIviEventListenerBase* const pIviEventListener);
	void Finalize(void);

	/* スクリーンIDの取得 */
	std::vector<uint32_t> GetScreenIDs(void) const;
	/* スクリーン解像度の取得 */
	int32_t GetScreenResolution(const int32_t screenId, int32_t& width, int32_t& height) const;

	/* ilmによる変更の反映 */
	static int32_t CommitChanges(void);

	struct LayerInfo {
		int32_t layer_id_;
		int32_t width_;
		int32_t height_;
	};

	/* レイヤーの作成 */
	static int32_t CreateLayers(const int32_t screenId, const std::vector<LayerInfo>& layerList);
	/* レイヤーの並べ替え */
	static int32_t SetLayerOrder(const int32_t screenId, std::vector<t_ilm_layer>& ivilayer_ids);
	/* サーフェスをレイヤーへ登録 */
	static int32_t AddSurfaceToLayer(const int32_t surfaceId, const int32_t layerId);

	/* サーフェスの可視状態取得 */
	int32_t GetSurfaceVisibility(const int32_t surfaceId, bool& visibility) const;
	/* サーフェスの可視状態設定 */
	static int32_t SetSurfaceVisibility(const int32_t surfaceId, const bool visible);

	/* サーフェスの不透明度設定 */
	static int32_t SetSurfaceOpacity(const int32_t surfaceId, uint32_t opacity);

	/* サーフェスのソース矩形の設定 */
	int32_t SetSurfaceSourceRectangle(const int32_t surfaceId, const int32_t x, const int32_t y, const int32_t w, const int32_t h) const;
	/* サーフェスのデスティネーション矩形の設定 */
	static int32_t SetSurfaceDestinationRectangle(const int32_t surfaceId, const int32_t x, const int32_t y, const int32_t w, const int32_t h);

	/* サーフェスのconfigureイベント通知 */
	void NotifyIviConfigureSurface(const int32_t surfaceId, const int32_t width, const int32_t height);
	/* IVIイベント通知 */
	void NotifyIviEvent(const bool object, const int32_t id, const bool created);

	/* サーフェスID一覧を取得 */
	int32_t GetSurfaceIds(std::vector<int32_t>& ivisurface_ids) const;

	/* スクリーンのiviプロパティ情報取得 */
	static std::string GetPropertiesOfScreen(const int32_t ivi_id, const std::string &name);
	/* レイヤーのiviプロパティ情報取得 */
	static std::string GetPropertiesOfLayer(const int32_t ivi_id, const std::string &name);
	/* サーフェスのiviプロパティ情報取得 */
	static std::string GetPropertiesOfSurface(const int32_t ivi_id, const std::string &name);
	/* サーフェスのiviプロパティ情報(FrameCounter)取得 */
	uint32_t GetPropertiesOfSurfaceFrameCounter(const int32_t ivi_id) const;
	/* スクリーンショット取得(screen) */
	int32_t GetScreenshot(const int32_t ivi_id, const std::string &path) const;
	/* スクリーンショット取得(surface) */
	int32_t GetSurfaceScreenshot(const int32_t ivi_id, const std::string &path) const;

protected:

private:
	CVhalMainControl *p_main_;
	std::unique_ptr<CVhalEventRoute> p_route_;
	CVhalIviEventListenerBase* p_ivi_listener_;

	mutable std::mutex mtx_screenshot_;
};

} /* namespace videohal */

#endif	/* #ifndef	VHAL_IVI_CONTROLLER_H */
