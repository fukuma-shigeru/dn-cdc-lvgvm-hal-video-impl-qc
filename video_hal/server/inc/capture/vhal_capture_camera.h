/*******************************************************************************
    機能名称    ：  カメラ制御モジュール
    ファイル名称：  vhal_capture_camera.h
*******************************************************************************/
#ifndef	VHAL_CAPTURE_CAMERA_H
#define	VHAL_CAPTURE_CAMERA_H

#include <string>

#include "vhal_capture_control.h"

namespace videohal
{

class CVhalLayoutManager;
class CVhalCaptureControl;

/*****************************************************************************
 クラス名称：CVhalCaptureCamera
 処理概要  ：カメラ映像の制御を行う。
*****************************************************************************/
class CVhalCaptureCamera {
public:
	CVhalCaptureCamera(void);
	virtual ~CVhalCaptureCamera(void);
  	CVhalCaptureCamera(const CVhalCaptureCamera& src) = delete;
	CVhalCaptureCamera& operator=(const CVhalCaptureCamera& src) & = delete;
	CVhalCaptureCamera(CVhalCaptureCamera&& src) = delete;
	CVhalCaptureCamera& operator=(CVhalCaptureCamera&& src) & = delete;

	int32_t Initialize(CVhalLayoutManager * const p_layout_mng, CVhalCaptureControl * const p_capture_control);
	void Finalize(void);

	/* カメラpathの設定 */
	int32_t SetPath(const std::string& path);
	/* カメラキャプチャ開始 */
	int32_t Start(void) const;
	/* カメラキャプチャ停止 */
	int32_t Stop(void) const;
	/* カメラキャプチャ動作取得 */
	bool IsCapture(void) const;

private:
	CVhalCaptureControl*	p_capture_control_{nullptr};
	captureInputType		input_type_{captureInputType::VHAL_CAPTURE_INPUT_CAMERA};
	bool 					running_{false};
};

} /* namespace videohal */


#endif	/* #ifndef	VHAL_CAPTURE_CAMERA_H */
