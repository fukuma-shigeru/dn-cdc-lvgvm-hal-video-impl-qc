/*******************************************************************************
    機能名称    ：  デバッグ機能モジュール
    ファイル名称：  vhal_debug_system.h
*******************************************************************************/
//#ifndef	VHAL_DEBUG_SYSTEM_H
//#define	VHAL_DEBUG_SYSTEM_H
//
//#define VHAL_SUPPORT_DEBUG_SYSTEM
//
//#ifdef VHAL_SUPPORT_DEBUG_SYSTEM
//
//#define VHAL_SUPPORT_FAIL_SYSTEM
//
//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//
//#include <string>
//#include <fstream>
//#include <vector>
//
//namespace videohal
//{
///*****************************************************************************
// クラス名称：CVhalDebugSystem
// 処理概要  ：デバッグ機能を実行する。
//*****************************************************************************/
//class CVhalDebugSystem {
//
//public:
//
//	static CVhalDebugSystem& GetInstance(void) noexcept;
//
//	bool CheckFailSystem(const std::string& fail_no, int32_t& fail_ret) const;
//	bool CheckFailSystem2(const std::string& fail_no, int32_t loop, int32_t& fail_ret) const;
//	void UpdateFailData(const std::string& fail_no, int32_t& value, const uint32_t loop=0U) const;
//
//protected:
//
//private:
//	CVhalDebugSystem(void);
//	~CVhalDebugSystem(void);
//	std::string GetFullPath(const std::string& fail_no) const noexcept;
//
//};
//
//}	/* videohal */
//
//#endif	/* VHAL_SUPPORT_FAIL_SYSTEM */
//#endif	/* VHAL_SUPPORT_DEBUG_SYSTEM */
//
//#endif	/* VHAL_DEBUG_SYSTEM_H */
