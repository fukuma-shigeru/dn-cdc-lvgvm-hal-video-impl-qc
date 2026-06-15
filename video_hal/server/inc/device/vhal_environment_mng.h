/*******************************************************************************
    機能名称    ：  環境制御モジュール
    ファイル名称：  vhal_environment_mng.h
*******************************************************************************/
#ifndef	VHAL_ENVIRONMENT_MNG_H
#define	VHAL_ENVIRONMENT_MNG_H

#include <string>
#include <fstream>

namespace videohal
{
/* ボード型番判定結果 */
enum class BoardModelNumber : uint32_t {
	kUnknown = 0U,	/* 不明 */
	kSa81,			/* SA81 */
	kSa61,			/* SA61 */
	kMax
};

/*****************************************************************************
 クラス名称：CVhalEnvironmentManager
 処理概要  ：実機環境の制御を行う。
*****************************************************************************/
class CVhalEnvironmentManager final {
public:

	CVhalEnvironmentManager(void);
	~CVhalEnvironmentManager(void);
  	CVhalEnvironmentManager(const CVhalEnvironmentManager& src) = delete;
	CVhalEnvironmentManager& operator=(const CVhalEnvironmentManager& src) & = default;
	CVhalEnvironmentManager(CVhalEnvironmentManager&& src) = delete;
	CVhalEnvironmentManager& operator=(CVhalEnvironmentManager&& src) & = delete;

	/* 初期化 */
	int32_t Initialize(void);

	/* ボード型番判定結果取得 */
	BoardModelNumber GetBoardModelNumber(void) const noexcept;

private:

	/* ボード型番判定結果 */
	BoardModelNumber board_model_{BoardModelNumber::kMax};
};

} /* namespace videohal */


#endif	/* #ifndef	VHAL_ENVIRONMENT_MNG_H */
