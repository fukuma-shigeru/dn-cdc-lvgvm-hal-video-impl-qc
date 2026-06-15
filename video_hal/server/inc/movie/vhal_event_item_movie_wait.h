/*******************************************************************************
    機能名称    ：  動画再生処理待ちイベントモジュール
    ファイル名称：  vhal_event_item_movie_wait.h
*******************************************************************************/
#ifndef	VHAL_EVENT_ITEM_MOVIE_WAIT_H
#define	VHAL_EVENT_ITEM_MOVIE_WAIT_H

#include "vhal_event_item_base.h"

namespace videohal
{

class CVhalMovieControl;

/*****************************************************************************
 クラス名称：CVhalEventItemMovieWait
 処理概要  ：動画再生処理待ちイベントクラス
*****************************************************************************/
class CVhalEventItemMovieWait : public CVhalEventItemBase {
public:

	CVhalEventItemMovieWait(void);
	~CVhalEventItemMovieWait(void) override;
  	CVhalEventItemMovieWait(const CVhalEventItemMovieWait& src) = delete;
	CVhalEventItemMovieWait& operator=(const CVhalEventItemMovieWait& src) & = delete;
	CVhalEventItemMovieWait(CVhalEventItemMovieWait&& src) = delete;
	CVhalEventItemMovieWait& operator=(CVhalEventItemMovieWait&& src) & = delete;

	/* イベント処理の実施 */
	int32_t Exec(void) const override;

	/* データ設定 */
	void SetData(CVhalMovieControl* const p_movie, const int32_t result) noexcept;

private:
	CVhalMovieControl* p_movie_{nullptr};
	int32_t result_{};
};

} /* namespace videohal */

#endif	/* #ifndef	VHAL_EVENT_ITEM_MOVIE_WAIT_H */
