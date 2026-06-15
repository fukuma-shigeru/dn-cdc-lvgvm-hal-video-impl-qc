/*******************************************************************************
    機能名称    ：  レイアウトファイルモジュール
    ファイル名称：  vhal_layout_file.cpp
*******************************************************************************/
#include "vhal_layout_file.h"

#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <json/json.h>
#include <json/reader.h>

#include "vhal_define.h"
#include "vhal_log.h"
#include "vhal_round_cast.h"
#include "vhal_debug_system.h"

namespace videohal
{

namespace
{
constexpr bool debug_print{false};
}

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalLayoutScreenProperty::CVhalLayoutScreenProperty(void) noexcept
	:CVhalLayoutScreenProperty(0, "")
{
}

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	int32_t				iviId	スクリーンiviid
         ：	const std::string&	name	スクリーン名
 戻り値  ：	なし
*****************************************************************************/
CVhalLayoutScreenProperty::CVhalLayoutScreenProperty(const int32_t iviId, const std::string& name)
	:ivi_id_(iviId)
	,screen_name_(name)
{
}

/*****************************************************************************
 処理概要：	スクリーンiviid取得
 引数    ：	なし
 戻り値  ：	スクリーンiviid
*****************************************************************************/
int32_t CVhalLayoutScreenProperty::GetIviId(void) const noexcept
{
	return ivi_id_;
}

/*****************************************************************************
 処理概要：	スクリーン名取得
 引数    ：	std::string&	name	(o)スクリーン名
 戻り値  ：	処理結果
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutScreenProperty::GetName(std::string& name) const
{
	name = screen_name_;
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	スクリーン上のレイヤiviidリスト取得
 引数    ：	std::vector<int32_t>&	ids	(o)iviidリスト
 戻り値  ：	処理結果
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutScreenProperty::GetLayerIds(std::vector<int32_t>& ids) const
{
	ids = layer_ids_;
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	スクリーン上のレイヤiviidリスト設定
 引数    ：	std::vector<int32_t>&	ids	(i)iviidリスト
 戻り値  ：	なし
*****************************************************************************/
void CVhalLayoutScreenProperty::SetLayerIds(const std::vector<int32_t>& ids)
{
	layer_ids_ = ids;
}



/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalLayoutLayerProperty::CVhalLayoutLayerProperty(void) noexcept
	:CVhalLayoutLayerProperty(0, 0, "", 0)
{
}

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	int32_t				iviId		レイヤーiviid
         ：	int32_t				parentId	親スクリーンiviid
         ：	const std::string&	name		レイヤー名
         ：	int32_t				order		order値
 戻り値  ：	なし
*****************************************************************************/
CVhalLayoutLayerProperty::CVhalLayoutLayerProperty(const int32_t iviId, const int32_t parentId, const std::string& name, const int32_t order)
	:ivi_id_(iviId)
	,ivi_screen_id_(parentId)
	,layer_name_(name)
	,order_(order)
{
}

/*****************************************************************************
 処理概要：	レイヤーiviid取得
 引数    ：	なし
 戻り値  ：	レイヤーiviid
*****************************************************************************/
int32_t CVhalLayoutLayerProperty::GetIviId(void) const noexcept
{
	return ivi_id_;
}

/*****************************************************************************
 処理概要：	レイヤーorder値取得
 引数    ：	なし
 戻り値  ：	レイヤーiviid
*****************************************************************************/
int32_t CVhalLayoutLayerProperty::GetOrder(void) const noexcept
{
	return order_;
}

/*****************************************************************************
 処理概要：	レイヤーorder値設定
 引数    ：	int32_t&	order	(i)order値
 戻り値  ：	処理結果
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutLayerProperty::SetOrder(const int32_t &order) noexcept
{
	order_ = order;
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	レイヤー名取得
 引数    ：	std::string&	name	(o)レイヤー名
 戻り値  ：	処理結果
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutLayerProperty::GetName(std::string& name) const
{
	name = layer_name_;
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	レイヤーの親スクリーンiviid取得
 引数    ：	なし
 戻り値  ：	親レイヤiviid
*****************************************************************************/
int32_t CVhalLayoutLayerProperty::GetParentScreenIviId(void) const noexcept
{
	return ivi_screen_id_;
}

/*****************************************************************************
 処理概要：	レイヤ上のサーフェスiviidリスト設定
 引数    ：	std::vector<int32_t>&	ids	(i)iviidリスト
 戻り値  ：	なし
*****************************************************************************/
void CVhalLayoutLayerProperty::SetSurfaceIds(const std::vector<int32_t>& ids)
{
	surface_ids_ = ids;
}



/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalLayoutSurfaceProperty::CVhalLayoutSurfaceProperty(void) noexcept
	:CVhalLayoutSurfaceProperty(0, 0, "", "visible")
{
}

/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	int32_t				iviId		サーフェスiviid
         ：	int32_t				parentId	親レイヤーiviid
         ：	const std::string&	name		サーフェス名
         ：	const std::string&	visibility	可視設定文字列
 戻り値  ：	なし
*****************************************************************************/
CVhalLayoutSurfaceProperty::CVhalLayoutSurfaceProperty(const int32_t iviId, const int32_t parentId, const std::string& name, const std::string& visibility)
	:ivi_id_(iviId)
	,ivi_layer_id_(parentId)
	,surface_name_(name)
{
	if ( "visible" == visibility )
	{
		initial_visibility_ = InitialVisibility::kVisible;
	}
	else if ( "invisible" == visibility )
	{
		initial_visibility_ = InitialVisibility::kInvisible;
	}
	else if ( "none" == visibility )
	{
		initial_visibility_ = InitialVisibility::kNone;
	}
	else
	{
		initial_visibility_ = InitialVisibility::kVisible;
	}
}

/*****************************************************************************
 処理概要：	サーフェスiviid取得
 引数    ：	なし
 戻り値  ：	サーフェスiviid
*****************************************************************************/
int32_t CVhalLayoutSurfaceProperty::GetIviId(void) const noexcept
{
	return ivi_id_;
}

/*****************************************************************************
 処理概要：	サーフェス名取得
 引数    ：	std::string&	name	(o)サーフェス名
 戻り値  ：	処理結果
           		VHAL_SUCCESS		正常終了
*****************************************************************************/
int32_t CVhalLayoutSurfaceProperty::GetName(std::string& name) const
{
	name = surface_name_;
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	サーフェスの親レイヤiviid取得
 引数    ：	なし
 戻り値  ：	親レイヤiviid
*****************************************************************************/
int32_t CVhalLayoutSurfaceProperty::GetParentLayerIviId(void) const noexcept
{
	return ivi_layer_id_;
}

/*****************************************************************************
 処理概要：	サーフェスの初期表示設定取得
 引数    ：	なし
 戻り値  ：	サーフェスの初期表示設定
*****************************************************************************/
CVhalLayoutSurfaceProperty::InitialVisibility CVhalLayoutSurfaceProperty::GetInitialVisibility(void) const noexcept
{
	return initial_visibility_;
}



/*****************************************************************************
 処理概要：	コンストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalLayoutConfigFile::CVhalLayoutConfigFile(void)
{
}

/*****************************************************************************
 処理概要：	デストラクタ
 引数    ：	なし
 戻り値  ：	なし
*****************************************************************************/
CVhalLayoutConfigFile::~CVhalLayoutConfigFile(void)
{
	if( debug_print ) {
		VHAL_LOGD("screen :%ld", screen_list_.size());
		VHAL_LOGD("layer  :%ld", layer_list_.size());
		VHAL_LOGD("surface:%ld", surface_list_.size());
	}

	// 内部データクリア
	const int32_t ret{ClearPropertyListAll()};
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("ClearPropertyListAll error. ret=%d", ret);
	}
}

/*****************************************************************************
 処理概要：	オーダーソート（昇順）
 引数    ：	const LayoutOrder& a	比較対象a
           	const LayoutOrder& b	比較対象b
 戻り値  ：	なし
*****************************************************************************/
bool CVhalLayoutConfigFile::SortOrderAsc(const LayoutOrder& a, const LayoutOrder& b) noexcept
{
	return (a.order_ < b.order_);

}

/*****************************************************************************
 処理概要：	レイアウト構成コンフィグファイル読み込み
 引数    ：	const std::string&	filePath	(i)ファイルパス
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS			正常終了
 フェールセーフNo  ：	F-VHAL-N-160
                   ：	F-VHAL-N-161
*****************************************************************************/
int32_t CVhalLayoutConfigFile::ReadFile(const std::string& filePath)
{
	// 内部データクリア
	int32_t ret{ClearPropertyListAll()};
	if (VHAL_SUCCESS != ret)
	{
		VHAL_LOGE("ClearPropertyListAll error. ret=%d", ret);
	}

	// jsonファイル読み込み
	std::string strjson{};
	struct stat orig_st;

	ret = lstat(filePath.c_str(), &orig_st);
	if (0 != ret)
	{
		VHAL_LOGE("lstat error. filePath=%s ret=%d", filePath.c_str(), ret);
	}
	else
	{
		if (!S_ISREG(orig_st.st_mode))
		{
			VHAL_LOGE("File is not regular file. filePath=%s", filePath.c_str());
		}
		else
		{
			const int32_t fd{open(filePath.c_str(), O_NOFOLLOW | O_NONBLOCK | O_RDONLY)};
			if (-1 == fd)
			{
				VHAL_LOGE("open error. filePath=%s", filePath.c_str());
			}
			else
			{
				struct stat open_st;
				ret = fstat(fd, &open_st);
				if (0 != ret)
				{
					VHAL_LOGE("fstat error. ret=%d", ret);
				}
				else
				{
					if ((0 >= open_st.st_size) || (LONG_MAX == open_st.st_size))
					{
						VHAL_LOGE("file size error. st_size=%ld", open_st.st_size);
					}
					else
					{
						const std::size_t buf_size{I64ToUI64(open_st.st_size + 1)};
						const std::unique_ptr<char[]>	buf{std::make_unique<char[]>(buf_size)};
						(void)memset(&buf[0], 0x00, buf_size);
						const ssize_t read_size{read(fd, &buf[0], static_cast<size_t>(open_st.st_size))};
						if (0 >= read_size)
						{
							VHAL_LOGE("file read error. read_size=%ld", read_size);
						}
						else
						{
							strjson = &buf[0];
						}
					}
				}
				ret = close(fd);
				if (-1 == ret)
				{
					VHAL_LOGE("close error. ret=%d", ret);
				}
			}
		}
	}

	int32_t result{VHAL_SUCCESS};
	if (true == strjson.empty())
	{
		VHAL_LOGE("strjson is empty.");
		result = VHAL_ERR_LAYOUT_INFO;
	}
	else
	{
		// jsonパース
		Json::CharReaderBuilder builder{};
		builder["rejectDupKeys"] = true;
		std::unique_ptr<Json::CharReader> reader{builder.newCharReader()};
		Json::Value root{};
		std::string errors{};
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//		int32_t fail_ret{0};
//		bool parse_rslt{reader->parse(strjson.c_str(), strjson.c_str() + strjson.size(), &root, &errors)};
//		bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-160",fail_ret)};
//		if(true == fail)
//		{
//			parse_rslt = false;
//		}
//#else
		const bool parse_rslt{reader->parse(strjson.c_str(), strjson.c_str() + strjson.size(), &root, &errors)};
//#endif
		reader = nullptr;
		if(!parse_rslt) {
			VHAL_LOGD("parse failed. %s", errors.c_str());
			result = VHAL_ERR_LAYOUT_INFO;
		}
		else
		{	
	// jsonデータ解析
	// スクリーンループ
			for( Json::Value::iterator itscreen{root.begin()}; itscreen != root.end(); ++itscreen ) {
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//				int32_t fail_ret{0};
//				Json::ValueType valtype0{(*itscreen).type()};
//				bool fail{CVhalDebugSystem::GetInstance().CheckFailSystem("F-VHAL-N-161",fail_ret)};
//				if(true == fail)
//				{
//					valtype0 = Json::nullValue;
//				}
//#else
				const Json::ValueType valtype0{(*itscreen).type()};
//#endif
				if( Json::objectValue == valtype0 )
				{
					valid_json_data_ = true;	/* jsonデータ有効 */
					if( debug_print ) {
						VHAL_LOGD("=================");
					}
					int32_t ivi_id_screen{UI32ToI32((*itscreen)["screen-id"].asUInt())};
					std::string obj_name{(*itscreen)["screen-name"].asString()};
					if( debug_print ) {
						VHAL_LOGD("screen:name=%s, id=%d", obj_name.c_str(), ivi_id_screen);
					}
					std::unique_ptr<CVhalLayoutScreenProperty> screen_prop{std::make_unique<CVhalLayoutScreenProperty>(ivi_id_screen, obj_name)};
					CVhalLayoutScreenProperty* const p_screen_prop{screen_prop.get()};
					(void)screen_list_.insert(std::make_pair(ivi_id_screen, std::move(screen_prop)));

					// レイヤループ
					std::vector<LayoutOrder> layer_order{};
					for( Json::Value::iterator itlayer{(*itscreen).begin()}; itlayer!= (*itscreen).end(); ++itlayer ) {
						const Json::ValueType valtype1{(*itlayer).type()};
						if( Json::objectValue == valtype1 ) {
							const int32_t ivi_id_layer{UI32ToI32((*itlayer)["layer-id"].asUInt())};
							obj_name     = (*itlayer)["layer-name"].asString();
							const int32_t order{(*itlayer)["layer-order"].asInt()};
							if( debug_print ) {
								VHAL_LOGD("  layer:name=%s, id=%d", obj_name.c_str(), ivi_id_layer);
							}
							std::unique_ptr<CVhalLayoutLayerProperty> layer_prop{std::make_unique<CVhalLayoutLayerProperty>(ivi_id_layer, ivi_id_screen, obj_name, order)};
							CVhalLayoutLayerProperty* const p_layer_prop{layer_prop.get()};
							(void)layer_list_.insert(std::make_pair(ivi_id_layer, std::move(layer_prop)));
							layer_order.push_back(LayoutOrder{ivi_id_layer, order});

							// サーフェスループ
							std::vector<int32_t> surface_ids{};
							for( Json::Value::iterator itsurface{(*itlayer).begin()}; itsurface!= (*itlayer).end(); ++itsurface ) {
								const Json::ValueType valtype2{(*itsurface).type()};
								if( Json::objectValue == valtype2 ) {
									const int32_t ivi_id_surface{UI32ToI32((*itsurface)["surface-id"].asUInt())};
									obj_name       = (*itsurface)["surface-name"].asString();
									std::string visibility{(*itsurface)["surface-visibility"].asString()};
									if (visibility == "") {
										visibility = "visible";
									}
									if ((*itsurface).isMember("surface-id-auto-num")) {
										const int32_t count{(*itsurface)["surface-id-auto-num"].asInt()};
										for (int32_t i{0}; i < count; i++) {
											int32_t ivi_id_current{ivi_id_surface + i};
											const std::string name_current{obj_name + "-" + std::to_string(i+1)};
											if( debug_print ) {
												VHAL_LOGD("    surface:name=%s, id=%d", name_current.c_str(), ivi_id_current);
											}
											std::unique_ptr<CVhalLayoutSurfaceProperty> surface_prop{std::make_unique<CVhalLayoutSurfaceProperty>(ivi_id_current, ivi_id_layer, name_current, visibility)};
											(void)surface_list_.insert(std::make_pair(ivi_id_current, std::move(surface_prop)));
											surface_ids.push_back(ivi_id_current);
										}
									} else {
										if( debug_print ) {
											VHAL_LOGD("    surface:name=%s, id=%d", obj_name.c_str(), ivi_id_surface);
										}
										std::unique_ptr<CVhalLayoutSurfaceProperty> surface_prop{std::make_unique<CVhalLayoutSurfaceProperty>(ivi_id_surface, ivi_id_layer, obj_name, visibility)};
										(void)surface_list_.insert(std::make_pair(ivi_id_surface, std::move(surface_prop)));
										surface_ids.push_back(ivi_id_surface);
									}
								}
							}
							// レイヤに各サーフェスのiviidを登録
							p_layer_prop->SetSurfaceIds(surface_ids);
						}
					}

					// スクリーンに各レイヤのiviidを登録（order値で昇順にソートしてから）
					std::stable_sort(layer_order.begin(), layer_order.end(), &SortOrderAsc);
					std::vector<int32_t> layer_ids{};
					std::vector<LayoutOrder>::const_iterator itorder{layer_order.cbegin()};
					const std::vector<LayoutOrder>::const_iterator itorderend{layer_order.cend()};
					for( ; itorder!=itorderend; ++itorder) {
						layer_ids.push_back(itorder->iviid_);
					}
					p_screen_prop->SetLayerIds(layer_ids);
				}
			}

			if (false == valid_json_data_)
			{
				result = VHAL_ERR_LAYOUT_INFO;
				VHAL_LOGE("invalid json data");
			}
		}
	}

	return result;
}

/*****************************************************************************
 処理概要：	スクリーン内のレイヤの並べ替え
 引数    ：	int32_t		layer_id	(i)レイヤiviid
           	int32_t		new_order	(i)レイヤーの新しいorder値
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutConfigFile::ReorderLayersOnScreen(const int32_t layer_id, const int32_t new_order)
{
	std::vector<LayoutOrder> layer_order{};
	std::vector<int32_t> layer_ids{};

	/* 新しいorder値をレイヤーに設定 */
	const CLayerPropList::iterator it_layer{layer_list_.find(layer_id)};
	if( it_layer == layer_list_.cend() )
	{
		VHAL_LOGE("error not found layer_id=%d", layer_id);
		return VHAL_ERR_LAYOUT_INFO;
	}
	CVhalLayoutLayerProperty &layer_prop{*(it_layer->second)};
	(void)layer_prop.SetOrder(new_order);

	/* 親スクリーン上のレイヤiviidリストを取得 */
	const int32_t screen_id{layer_prop.GetParentScreenIviId()};
	const CScreenPropList::iterator it_screen{screen_list_.find(screen_id)};
	if( it_screen == screen_list_.cend() )
	{
		VHAL_LOGE("error not found screen_id=%d, layer_id=%d", screen_id, layer_id);
		return VHAL_ERR_LAYOUT_INFO;
	}
	CVhalLayoutScreenProperty &screen_prop{*(it_screen->second)};
	(void)screen_prop.GetLayerIds(layer_ids);

	/* 各レイヤーのorder値取得 */
	std::vector<int32_t>::const_iterator itlayer{layer_ids.cbegin()};
	const std::vector<int32_t>::const_iterator itlayerend{layer_ids.cend()};
	for( ; itlayer!=itlayerend; ++itlayer)
	{
		CVhalLayoutLayerProperty layer_prop2{};
		const int32_t ret{GetLayerProperty(*itlayer, layer_prop2)};
		if (VHAL_SUCCESS != ret)
		{
			VHAL_LOGE("error not found layer id=%d, screen_id=%d", *itlayer, screen_id);
			return ret;
		}

		const int32_t order{layer_prop2.GetOrder()};
		VHAL_LOGD("layer[%d] order[%d]", *itlayer, order);
		layer_order.push_back(LayoutOrder{*itlayer, order});
	}

	/* order値で並べ替え（安定ソート） */
	VHAL_LOGV("sort layers by order value");
	std::stable_sort(layer_order.begin(), layer_order.end(), &SortOrderAsc);

	/* スクリーン上のレイヤiviidリストへ設定 */
	layer_ids.clear();
	std::vector<LayoutOrder>::const_iterator itorder{layer_order.cbegin()};
	const std::vector<LayoutOrder>::const_iterator itorderend{layer_order.cend()};
	for( ; itorder!=itorderend; ++itorder)
	{
		VHAL_LOGD("layer[%d]", itorder->iviid_);
		layer_ids.push_back(itorder->iviid_);
	}

	screen_prop.SetLayerIds(layer_ids);
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	スクリーンプロパティ取得
 引数    ：	int32_t						iviId		(i)スクリーンiviid
           	CVhalLayoutScreenProperty&	screenProp	(o)スクリーンプロパティ
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutConfigFile::GetScreenProperty(const int32_t iviId, CVhalLayoutScreenProperty& screenProp) const
{
	const CScreenPropList::const_iterator it{screen_list_.find(iviId)};
	if( it == screen_list_.end() ) {
		return VHAL_ERR_LAYOUT_INFO;
	}
	screenProp = *(it->second);
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	スクリーンプロパティ取得
 引数    ：	const std::string&			name		(i)スクリーン名
           	CVhalLayoutScreenProperty&	screenProp	(o)スクリーンプロパティ
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutConfigFile::GetScreenProperty(const std::string& name, CVhalLayoutScreenProperty& screenProp) const
{
	class CFindScreen {
	    std::string name_;
	public:
	    CFindScreen(const std::string& name)
	    	:name_(name)
	    {}
	    bool operator()( const std::pair<int32_t, const std::unique_ptr<CVhalLayoutScreenProperty>&>prop ) const {
	    	std::string screen_name{};
	    	(void)prop.second->GetName(screen_name);
	        return (0 == screen_name.compare(name_));
	    }
	};
	const CScreenPropList::const_iterator itFind{std::find_if(screen_list_.begin(), screen_list_.end(), CFindScreen(name))};
	if (itFind == screen_list_.end()) {
		return VHAL_ERR_LAYOUT_INFO;
	}
	screenProp = *(itFind->second);

	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	レイヤープロパティ取得
 引数    ：	int32_t						iviId		(i)レイヤーiviid
           	CVhalLayoutLayerProperty&	layerProp	(o)レイヤープロパティ
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutConfigFile::GetLayerProperty(const int32_t iviId, CVhalLayoutLayerProperty& layerProp) const
{
	const CLayerPropList::const_iterator it{layer_list_.find(iviId)};
	if( it == layer_list_.end() ) {
		return VHAL_ERR_LAYOUT_INFO;
	}
	layerProp = *(it->second);
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	レイヤープロパティ取得
 引数    ：	const std::string&			name		(i)レイヤー名
           	CVhalLayoutLayerProperty&	layerProp	(o)レイヤープロパティ
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutConfigFile::GetLayerProperty(const std::string& name, CVhalLayoutLayerProperty& layerProp) const
{
	class CFindLayer {
	    std::string name_;
	public:
	    CFindLayer(const std::string& name)
	    	:name_(name)
	    {}
	    bool operator()( const std::pair<int32_t, const std::unique_ptr<CVhalLayoutLayerProperty>&>prop ) const {
	    	std::string layer_name{};
	    	(void)prop.second->GetName(layer_name);
	        return (0 == layer_name.compare(name_));
	    }
	};
	const CLayerPropList::const_iterator itFind{std::find_if(layer_list_.begin(), layer_list_.end(), CFindLayer(name))};
	if (itFind == layer_list_.end()) {
		return VHAL_ERR_LAYOUT_INFO;
	}
	layerProp = *(itFind->second);

	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	サーフェスプロパティ取得
 引数    ：	int32_t						iviId		(i)サーフェスiviid
           	CVhalLayoutSurfaceProperty&	surfaceProp	(o)サーフェスプロパティ
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutConfigFile::GetSurfaceProperty(const int32_t iviId, CVhalLayoutSurfaceProperty& surfaceProp) const
{
	const CSurfacePropList::const_iterator it{surface_list_.find(iviId)};
	if( it == surface_list_.end() ) {
		return VHAL_ERR_LAYOUT_INFO;
	}
	surfaceProp = *(it->second);
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	サーフェスプロパティ取得
 引数    ：	const std::string&			name		(i)サーフェス名
           	CVhalLayoutSurfaceProperty&	surfaceProp	(o)サーフェスプロパティ
 戻り値  ：	処理結果
           		VHAL_ERR_LAYOUT_INFO	レイアウト情報不正
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutConfigFile::GetSurfaceProperty(const std::string& name, CVhalLayoutSurfaceProperty& surfaceProp) const
{
	class CFindSurface {
	    std::string name_;
	public:
	    CFindSurface(const std::string& name)
	    	:name_(name)
	    {}
	    bool operator()( const std::pair<int32_t, const std::unique_ptr<CVhalLayoutSurfaceProperty>&>prop ) const {
	    	std::string surf_name{};
	    	(void)prop.second->GetName(surf_name);
	        return (0 == surf_name.compare(name_));
	    }
	};
	const CSurfacePropList::const_iterator itFind{std::find_if(surface_list_.begin(), surface_list_.end(), CFindSurface(name))};
	if (itFind == surface_list_.end()) {
		return VHAL_ERR_LAYOUT_INFO;
	}
	surfaceProp = *(itFind->second);

	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	スクリーンiviIDリスト取得
 引数    ：	std::vector<int32_t>&	ids	(o)スクリーンiviIDリスト
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutConfigFile::GetScreenIviidList(std::vector<int32_t>& ids) const
{
	CScreenPropList::const_iterator it{screen_list_.begin()};
	const CScreenPropList::const_iterator itEnd{screen_list_.end()};

	ids.clear();
	for(; it!=itEnd; ++it ) {
		ids.push_back(it->second->GetIviId());
	}
	return VHAL_SUCCESS;
}

/*****************************************************************************
 処理概要：	全プロパティリストクリア
 引数    ：	なし
 戻り値  ：	処理結果
           		VHAL_SUCCESS			正常終了
*****************************************************************************/
int32_t CVhalLayoutConfigFile::ClearPropertyListAll(void) noexcept
{
	// 各mapの解放
	screen_list_.clear();
	layer_list_.clear();
	surface_list_.clear();

	return VHAL_SUCCESS;
}



} /* namespace videohal */

