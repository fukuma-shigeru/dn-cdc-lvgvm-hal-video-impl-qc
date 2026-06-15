/*******************************************************************************
    機能名称    ：  画質調整データベースモジュール
    ファイル名称：  vhal_color_db.h
*******************************************************************************/
#ifndef	VHAL_COLOR_DB_H
#define	VHAL_COLOR_DB_H

#include <map>
#include <mutex>
#include <vector>
#include <memory>
#include "vhal_color_record.h"

namespace videohal
{

class CVhalBackupControl;

/*****************************************************************************
 クラス名称：CVhalColorDatabase
 処理概要  ：画質調整のデータ管理を行う。
*****************************************************************************/
class CVhalColorDatabase {
public:

	CVhalColorDatabase(void);
	~CVhalColorDatabase(void);
  	CVhalColorDatabase(const CVhalColorDatabase& src) = delete;
	CVhalColorDatabase& operator=(const CVhalColorDatabase& src) & = delete;
	CVhalColorDatabase(CVhalColorDatabase&& src) = delete;
	CVhalColorDatabase& operator=(CVhalColorDatabase&& src) & = delete;

	int32_t Initialize(void);
	void Finalize(void);

	/* color_id:呼び元でユニークIDを指定 */
	int32_t CreateColorRecord(const uint32_t color_id, const CVhalColorRecord::ColorType color_type, const CVhalColorRecord::ColorImgAdjStep& color_init, const bool backup);
	void DeleteColorRecord(const uint32_t color_id);
	int32_t LoadColorRecord(void);
	int32_t SaveColorRecord(void);
	int32_t ClearColorRecord(void);

	int32_t Write(const uint32_t color_id, const CVhalColorRecord& color_record);
	int32_t Read(const uint32_t color_id, CVhalColorRecord& color_record) const;


private:
	mutable std::mutex mtx_record_;

	/* 各レコードリスト型<color_id, レコードインスタンス> */
	using CColorRecordList = std::map<uint32_t, std::unique_ptr<CVhalColorRecord>>;

	/* レコードリスト */
	CColorRecordList		record_list_;
	/* バックアップデータ 初期化情報 */
	uint8_t					init_state_;

	std::unique_ptr<CVhalBackupControl>					p_backup_control_;

	/* 画質調整デフォルト値 */
	CVhalColorRecord::ColorImgAdjStep	theme_color_init_;
	CVhalColorRecord::ColorImgAdjStep	daynight_color_init_;

	/* レコードリスト(record_list_)から、バックアップデータへ設定 */
	int32_t SerializeBackupData(std::vector<uint8_t> &backup_data);
	/* バックアップデータから、レコードリスト(record_list_)へ設定 */
	int32_t DeserializeBackupData(std::vector<uint8_t> &backup_data);
};

} /* namespace videohal */

#endif	/* #ifndef	VHAL_COLOR_DB_H */
