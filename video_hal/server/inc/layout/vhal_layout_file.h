/*******************************************************************************
    機能名称    ：  レイアウトファイルモジュール
    ファイル名称：  vhal_layout_file.h
*******************************************************************************/
#ifndef	VHAL_LAYOUT_FILE_H
#define	VHAL_LAYOUT_FILE_H

#include <string>
#include <vector>
#include <map>
#include <memory>


namespace videohal
{

/*****************************************************************************
 クラス名称：CVhalLayoutPropertyCommon
 処理概要  ：レイアウト構成共通プロパティクラス
*****************************************************************************/
class CVhalLayoutPropertyCommon {
public:
	CVhalLayoutPropertyCommon(void) noexcept = default;
	virtual ~CVhalLayoutPropertyCommon(void) = default;
  	CVhalLayoutPropertyCommon(const CVhalLayoutPropertyCommon& src) = delete;
	CVhalLayoutPropertyCommon(CVhalLayoutPropertyCommon&& src) = delete;

	// iviid取得
	virtual int32_t GetIviId(void) const = 0;

	// 名前取得
	virtual int32_t GetName(std::string& name) const = 0;

protected:
	CVhalLayoutPropertyCommon& operator=(const CVhalLayoutPropertyCommon& src) & = default;
	CVhalLayoutPropertyCommon& operator=(CVhalLayoutPropertyCommon&& src) & = delete;
};

/*****************************************************************************
 クラス名称：CVhalLayoutScreenProperty
 処理概要  ：レイアウト構成スクリーンプロパティクラス
*****************************************************************************/
class CVhalLayoutScreenProperty : public CVhalLayoutPropertyCommon {
public:
	CVhalLayoutScreenProperty(void) noexcept;
	CVhalLayoutScreenProperty(const int32_t iviId, const std::string& name);
	~CVhalLayoutScreenProperty(void) override = default;
	CVhalLayoutScreenProperty(const CVhalLayoutScreenProperty& src) = delete;
	CVhalLayoutScreenProperty& operator=(const CVhalLayoutScreenProperty& src) & = default;
	CVhalLayoutScreenProperty(CVhalLayoutScreenProperty&& src) = delete;
	CVhalLayoutScreenProperty& operator=(CVhalLayoutScreenProperty&& src) & = delete;

	// スクリーンiviid取得
	int32_t GetIviId(void) const noexcept override;

	// スクリーン名取得
	int32_t GetName(std::string& name) const override;

	// スクリーン上のレイヤiviidリスト取得
	int32_t GetLayerIds(std::vector<int32_t>& ids) const;

	// スクリーン上のレイヤiviidリスト設定
	void SetLayerIds(const std::vector<int32_t>& ids);

private:
	int32_t ivi_id_;
	std::string screen_name_;
	std::vector<int32_t> layer_ids_;	// レイヤiviidリスト
};

/*****************************************************************************
 クラス名称：CVhalLayoutLayerProperty
 処理概要  ：レイアウト構成レイヤプロパティクラス
*****************************************************************************/
class CVhalLayoutLayerProperty : public CVhalLayoutPropertyCommon {
public:
	CVhalLayoutLayerProperty(void) noexcept;
	CVhalLayoutLayerProperty(const int32_t iviId, const int32_t parentId, const std::string& name, const int32_t order);

	~CVhalLayoutLayerProperty(void) override = default;
  	CVhalLayoutLayerProperty(const CVhalLayoutLayerProperty& src) = delete;
	CVhalLayoutLayerProperty& operator=(const CVhalLayoutLayerProperty& src) & = default;
	CVhalLayoutLayerProperty(CVhalLayoutLayerProperty&& src) = delete;
	CVhalLayoutLayerProperty& operator=(CVhalLayoutLayerProperty&& src) & = delete;
	
	// レイヤiviid取得
	int32_t GetIviId(void) const noexcept override;

	/* レイヤーorder値取得 */
	int32_t GetOrder(void) const noexcept;

	/* レイヤーorder値設定 */
	int32_t SetOrder(const int32_t &order) noexcept;

	// レイヤ名取得
	int32_t GetName(std::string& name) const override;

	// レイヤの親スクリーンiviid取得
	int32_t GetParentScreenIviId(void) const noexcept;

	// レイヤ上のサーフェスiviidリスト設定
	void SetSurfaceIds(const std::vector<int32_t>& ids);

private:
	int32_t ivi_id_;
	int32_t ivi_screen_id_;
	std::string layer_name_;
	int32_t order_;
	std::vector<int32_t> surface_ids_;		// サーフェスiviidリスト
};

/*****************************************************************************
 クラス名称：CVhalLayoutSurfaceProperty
 処理概要  ：レイアウト構成サーフェスプロパティクラス
*****************************************************************************/
class CVhalLayoutSurfaceProperty : public CVhalLayoutPropertyCommon {
public:
	/* サーフェス初期表示設定 */
	enum class InitialVisibility : int32_t {
		kVisible,			/* 可視         */
		kInvisible,			/* 不可視       */
		kNone				/* 可視設定なし */
	};

	CVhalLayoutSurfaceProperty(void) noexcept;
	CVhalLayoutSurfaceProperty(const int32_t iviId, const int32_t parentId, const std::string& name, const std::string& visibility);
	~CVhalLayoutSurfaceProperty(void) override = default;
  	CVhalLayoutSurfaceProperty(const CVhalLayoutSurfaceProperty& src) = delete;
	CVhalLayoutSurfaceProperty& operator=(const CVhalLayoutSurfaceProperty& src) & = default;
	CVhalLayoutSurfaceProperty(CVhalLayoutSurfaceProperty&& src) = delete;
	CVhalLayoutSurfaceProperty& operator=(CVhalLayoutSurfaceProperty&& src) & = delete;
	
	/* サーフェスiviid取得 */
	int32_t GetIviId(void) const noexcept override;

	/* サーフェス名取得 */
	int32_t GetName(std::string& name) const override;

	/* サーフェスの親レイヤiviid取得 */
	int32_t GetParentLayerIviId(void) const noexcept;

	/* サーフェスの初期表示設定取得 */
	InitialVisibility GetInitialVisibility(void) const noexcept;

private:
	int32_t ivi_id_;
	int32_t ivi_layer_id_;
	std::string surface_name_;
	InitialVisibility initial_visibility_;
};

/*****************************************************************************
 クラス名称：CVhalLayoutConfigFile
 処理概要  ：レイアウト構成コンフィグファイルクラス
*****************************************************************************/
class CVhalLayoutConfigFile {
public:
	CVhalLayoutConfigFile(void);
	virtual ~CVhalLayoutConfigFile(void);
  	CVhalLayoutConfigFile(const CVhalLayoutConfigFile& src) = delete;
	CVhalLayoutConfigFile& operator=(const CVhalLayoutConfigFile& src) & = delete;
	CVhalLayoutConfigFile(CVhalLayoutConfigFile&& src) = delete;
	CVhalLayoutConfigFile& operator=(CVhalLayoutConfigFile&& src) & = delete;

	// レイアウト構成コンフィグファイル読み込み
	int32_t ReadFile(const std::string& filePath);

	/* スクリーン内のレイヤの並べ替え */
	int32_t ReorderLayersOnScreen(const int32_t layer_id, const int32_t new_order);

	// スクリーンプロパティ取得
	int32_t GetScreenProperty(const int32_t iviId, CVhalLayoutScreenProperty& screenProp) const;
	int32_t GetScreenProperty(const std::string& name, CVhalLayoutScreenProperty& screenProp) const;

	// レイヤプロパティ取得
	int32_t GetLayerProperty(const int32_t iviId, CVhalLayoutLayerProperty& layerProp) const;
	int32_t GetLayerProperty(const std::string& name, CVhalLayoutLayerProperty& layerProp) const;

	// サーフェスプロパティ取得
	int32_t GetSurfaceProperty(const int32_t iviId, CVhalLayoutSurfaceProperty& surfaceProp) const;
	int32_t GetSurfaceProperty(const std::string& name, CVhalLayoutSurfaceProperty& surfaceProp) const;

	// スクリーンiviIDリスト取得
	int32_t GetScreenIviidList(std::vector<int32_t>& ids) const;


	// 各プロパティリスト型<iviid, プロパティインスタンス>
	using CScreenPropList = std::map<int32_t, std::unique_ptr<CVhalLayoutScreenProperty>>;
	using CLayerPropList = std::map<int32_t, std::unique_ptr<CVhalLayoutLayerProperty>>;
	using CSurfacePropList = std::map<int32_t, std::unique_ptr<CVhalLayoutSurfaceProperty>>;

private:

	// オーダーソート用構造体
	struct LayoutOrder {
		int32_t 	iviid_;
		int32_t 	order_;
	};
	// オーダーソート（昇順）
	static bool SortOrderAsc(const LayoutOrder& a, const LayoutOrder& b) noexcept;

	// 全プロパティリストクリア
	int32_t ClearPropertyListAll(void) noexcept;


	CScreenPropList		screen_list_;		// スクリーンプロパティリスト
	CLayerPropList		layer_list_;		// レイヤプロパティリスト
	CSurfacePropList	surface_list_;		// サーフェスプロパティリスト
	
	bool valid_json_data_{false};			// JSONデータ有効判定

};


} /* namespace videohal */


#endif	/* #ifndef	VHAL_LAYOUT_FILE_H */
