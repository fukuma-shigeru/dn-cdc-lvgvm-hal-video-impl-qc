/*******************************************************************************
    機能名称    ：  HDCP認証モジュール
    ファイル名称：  vhal_hdcp_auth_mng.h
*******************************************************************************/
#ifndef	VHAL_HDCP_AUTH_MNG_H
#define	VHAL_HDCP_AUTH_MNG_H

#include <string>
#include "vhal_hdcp_auth_control.h"

namespace videohal
{
class CVhalMainControl;
class CVhalHdcpAuthControl;

/*****************************************************************************
 クラス名称：CVhalHdcpAuthManager
 処理概要  ：HDCP認証制御を行う。
*****************************************************************************/
class CVhalHdcpAuthManager {
public:

	static constexpr uint64_t	khdcpAuthMaxDevCnt{3U};		/* 最大接続デバイス数 */

	CVhalHdcpAuthManager(void);
	virtual ~CVhalHdcpAuthManager(void);
  	CVhalHdcpAuthManager(const CVhalHdcpAuthManager& src) = delete;
	CVhalHdcpAuthManager& operator=(const CVhalHdcpAuthManager& src) & = default;
	CVhalHdcpAuthManager(CVhalHdcpAuthManager&& src) = delete;
	CVhalHdcpAuthManager& operator=(CVhalHdcpAuthManager&& src) & = delete;

	int32_t Initialize(void);
	void Finalize(void);

	/* HDCP認証開始(RSE) */
	int32_t StartAuthRse(void);
	/* HDCP認証停止(RSE) */
	void StopAuthRse(void);
	/* RSE種別設定 */
	void SetConnectedRse(const int32_t type) noexcept;

	/* HDCP認証状態初期値取得 */
	int32_t GetHdcpInitStatus(void) const noexcept;

	/* HDCP認証キー取得 */
	int32_t GetHdcpAuthKey(const hdcpAuthType auth_type, CVhalHdcpAuthRsltData& hdcp_key) const noexcept;
	/* HDCP認証キー更新 */
	int32_t UpdateHdcpAuthResult(const hdcpAuthType auth_type, const CVhalHdcpAuthRsltData& hdcp_key) noexcept;
	/* HDCP認証キークリア */
	void ClearHdcpAuthKey(void) noexcept;

private:
	std::unique_ptr<CVhalHdcpAuthControl>		p_hdcp_auth_rse_{nullptr};
	std::unique_ptr<CVhalHdcpAuthControl>		p_hdcp_auth_cdisp_{nullptr};

	int32_t						rse_type_{VHAL_CONNECTED_RSE_INVALID};				/* RSE種別 */
	int32_t						hdcp_init_status_{VHAL_HDCP_FIRST_AUTH_STS_NONE};	/* HDCP認証状態初期値 */

	/* HDCP状態監視クラス取得 */
	CVhalHdcpAuthControl* GetHdcpAuthControl(const hdcpAuthType auth_type) const noexcept;
};

} /* namespace videohal */


#endif	/* #ifndef	VHAL_HDCP_AUTH_MNG_H */
