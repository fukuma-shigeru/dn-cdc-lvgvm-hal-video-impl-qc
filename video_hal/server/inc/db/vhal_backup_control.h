/*******************************************************************************
    機能名称    ：  バックアップ制御モジュール
    ファイル名称：  vhal_backup_control.h
*******************************************************************************/
#ifndef	VHAL_BACKUP_CONTROL_H
#define	VHAL_BACKUP_CONTROL_H

#include <cstdint>
#include <string>
#include <vector>

extern "C"
{
	#include "sys_bkup_mng_public.h"
}

namespace videohal
{

/*****************************************************************************
 クラス名称：CVhalBackupControl
 処理概要  ：バックアップの制御を行う。
*****************************************************************************/
class CVhalBackupControl final {
public:
	CVhalBackupControl(void);
	~CVhalBackupControl(void);
  	CVhalBackupControl(const CVhalBackupControl& src) = delete;
	CVhalBackupControl& operator=(const CVhalBackupControl& src) & = delete;
	CVhalBackupControl(CVhalBackupControl&& src) = delete;
	CVhalBackupControl& operator=(CVhalBackupControl&& src) & = delete;

	static int32_t Load(const std::string &name, std::vector<uint8_t> &backup_data);
	static int32_t Save(const std::string &name, std::vector<uint8_t> &backup_data, const bool checkAttr);

private:
	static int32_t GetAttr(const std::string &name, SysBkupMngAttr &attr);
};

} /* namespace videohal */

#endif	/* #ifndef	VHAL_BACKUP_CONTROL_H */
