/*******************************************************************************
    機能名称    ：  デバッグ機能モジュール
    ファイル名称：  vhal_debug_system.cpp
*******************************************************************************/
#include "vhal_debug_system.h"

//#ifdef VHAL_SUPPORT_FAIL_SYSTEM
//
//#include "vhal_log.h"
//#include <filesystem>
//
//namespace videohal
//{
//
///*****************************************************************************
// 処理概要：	コンストラクタ
// 引数    ：	なし
// 戻り値  ：	なし
//*****************************************************************************/
//CVhalDebugSystem::CVhalDebugSystem(void)
//{
//	VHAL_LOGV("CVhalDebugSystem is created. this=%p", this);
//}
//
///*****************************************************************************
// 処理概要：	デストラクタ
// 引数    ：	なし
// 戻り値  ：	なし
//*****************************************************************************/
//CVhalDebugSystem::~CVhalDebugSystem(void)
//{
//	VHAL_LOGV("CVhalDebugSystem is deleted. this=%p", this);
//}
//
///*****************************************************************************
// 処理概要：	デバッグ機能インスタンス取得
// 引数    ：	なし
// 戻り値  ：	デバッグ機能インスタンス
//*****************************************************************************/
//CVhalDebugSystem& CVhalDebugSystem::GetInstance(void) noexcept
//{
//	static CVhalDebugSystem debug_system{};
//	return debug_system;
//}
//
///*****************************************************************************
// 処理概要：	フルパス生成
// 引数    ：	std::string& fail_no	(i)フェールセーフNo
// 戻り値  ：	処理結果		フルパス名
//*****************************************************************************/
//std::string CVhalDebugSystem::GetFullPath(const std::string& fail_no) const noexcept
//{
//	std::string full_name{"/opt/dc-ivi-pf/bin/test/"};
//	return full_name.append(fail_no).append(".txt");
//}
//
///*****************************************************************************
// 処理概要：	フェールシステム発生判定
// 引数    ：	std::string& fail_no	(i)フェールセーフNo
//        	int32_t& fail_ret		(o)異常値
// 戻り値  ：	処理結果
//           		true		フェール発生あり
//           		false		フェール発生なし
//*****************************************************************************/
//bool CVhalDebugSystem::CheckFailSystem(const std::string& fail_no, int32_t& fail_ret) const
//{
//	bool fail{false};
//
//	const std::string fail_file{GetFullPath(fail_no)};
//	std::ifstream ifs(fail_file.c_str());
//	if (ifs.is_open())
//	{
//		ifs >> fail_ret;
//		fail = true;
//	}
//	return fail;
//}
//
///*****************************************************************************
// 処理概要：	フェールシステム発生判定
// 引数    ：	std::string& fail_no	(i)フェールセーフNo
//        	int32_t  loop			(i)ループ回数
//        	int32_t& fail_ret		(o)異常値
// 戻り値  ：	処理結果
//           		true		フェール発生あり
//           		false		フェール発生なし
//*****************************************************************************/
//bool CVhalDebugSystem::CheckFailSystem2(const std::string& fail_no, int32_t loop, int32_t& fail_ret) const
//{
//	bool fail{false};
//	int32_t	ret1,thresh;
//
//	const std::string fail_file{GetFullPath(fail_no)};
//	std::ifstream ifs(fail_file.c_str());
//	if (ifs.is_open())
//	{
//		ifs >> ret1 >> thresh;
//		if (loop < thresh)
//		{
//			fail_ret = ret1;
//			fail = true;
//		}
//	}
//	return fail;
//}
//
///*****************************************************************************
// 処理概要：	フェールシステム発生時のデータ更新
// 引数    ：	std::string& fail_no	(i)フェールセーフNo
//        	int32_t& value			(i/o)変更前後の値
//        	uint32_t loop			(i)ループ回数(省略時は0。"R"の場合以外は不要)
// 戻り値  ：	なし
//*****************************************************************************/
//void CVhalDebugSystem::UpdateFailData(const std::string& fail_no, int32_t& value, const uint32_t loop) const
//{
//	int32_t		fail_value{value};
//	uint32_t	loop_max{0x7FFFFFFFU};
//
//	const std::string fail_file{GetFullPath(fail_no)};
//	std::ifstream ifs(fail_file.c_str());
//	if (ifs.is_open())
//	{
//		ifs >> fail_value >> loop_max;
//		if (loop < loop_max)
//		{
//			value = fail_value;
//		}
//	}
//}
//
//} /* namespace videohal */
//
//#endif /* VHAL_SUPPORT_FAIL_SYSTEM */
