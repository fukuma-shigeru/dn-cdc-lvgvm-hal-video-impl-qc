/*******************************************************************************
    機能名称    ：  HDMI制御モジュール
    ファイル名称：  vhal_capture_hdmi.h
*******************************************************************************/
#ifndef	VHAL_CAPTURE_HDMI_H
#define	VHAL_CAPTURE_HDMI_H

#include <string>
#include <vector>
#include <map>

#include "vhal_capture_control.h"

namespace videohal
{

class CVhalLayoutManager;
class CVhalCaptureControl;

/*****************************************************************************
 クラス名称：CVhalCaptureHdmi
 処理概要  ：HDMI映像の制御を行う。
*****************************************************************************/
class CVhalCaptureHdmi {
public:
	CVhalCaptureHdmi(void);
	virtual ~CVhalCaptureHdmi(void);
	CVhalCaptureHdmi(const CVhalCaptureHdmi& src) = delete;
	CVhalCaptureHdmi& operator=(const CVhalCaptureHdmi& src) & = delete;
	CVhalCaptureHdmi(CVhalCaptureHdmi&& src) = delete;
	CVhalCaptureHdmi& operator=(CVhalCaptureHdmi&& src) & = delete;

	int32_t Initialize(CVhalLayoutManager * const p_layout_mng, CVhalCaptureControl * const p_capture_control, const uint32_t width, const uint32_t height);
	void Finalize(void);

private:
	CVhalCaptureControl*  p_capture_control_{nullptr};
	bool running_{false};

};

} /* namespace videohal */


#endif	/* #ifndef	VHAL_CAPTURE_HDMI_H */
