/**
 * \file vhal_gst_player.h
 * \brief gstreamer を使用した video player API
 */

#ifndef VHAL_GST_PLAYER_H
#define VHAL_GST_PLAYER_H

#ifdef  __cplusplus
extern "C" {
#endif

struct vhal_player;

/**
 * \struct vhal_player_cb
 * \brief player からのイベント通知を受け取るためのコールバック
 *
 * これらのコールバックは gstreamer での動画再生のためのスレッドから呼ばれるため、これらのコールバック内で vhal_player_stop() を呼ばないでください。
 */
struct vhal_player_cb {
	/**
	 * gstreamer の再生が完了したときに呼ばれるコールバック関数
	 *
	 * \param [in] user_data vhal_player_start() の user_data 引数に渡したポインタ
	 */
	void (*completion)(const void * const user_data);
	/**
	 * gstreamer でエラーが発生したときに呼ばれるコールバック関数
	 *
	 * \param [in] user_data vhal_player_start() の user_data 引数に渡したポインタ
	 */
	void (*error)(const void * const user_data);
};

/**
 * \brief player の初期化を行い、再生を開始する。
 *
 * \param [in] file_path 再生するファイルのパス
 * \param [in] callback player からのイベント通知を受けるためのコールバックへのポインタ。イベント処理を行わない場合は NULL を渡す。
 * \param [in] user_data イベント通知を受けるためのコールバックに返されるオブジェクトのポインタ。各コールバックの user_data 引数に渡される。不要な場合は NULL を渡す。
 * \return player context
 */
struct vhal_player *vhal_player_start(const char *file_path,
				      const struct vhal_player_cb *callback,
				      void *user_data);

/**
 * \brief player を停止して画面を消去する。
 *
 * \param [in] player vhal_player_start() で返された player context
 */
void vhal_player_stop(struct vhal_player *player);
#ifdef TEST_ERROR_OCCUR
void generate_gst_error(struct vhal_player *player);
#endif

#ifdef  __cplusplus
}
#endif

#endif
