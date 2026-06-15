/*******************************************************************************
    機能名称    ：  画面キャプチャ制御モジュール
    ファイル名称：  vhal_screen_shot.h
*******************************************************************************/
#ifndef	VHAL_SCREEN_SHOT_H
#define	VHAL_SCREEN_SHOT_H

#include "vhal_worker_thread.h"
#include "vhal_event_route.h"
#include "vhal_event_item_screen_shot_event.h"

/*****************************************************************************
 クラス名称：CVhalScreenShot
 処理概要  ：画面キャプチャ処理の制御を行う。
*****************************************************************************/
namespace videohal
{
class CVhalMainControl;
class CVhalLayoutManager;

struct VhalScreenShotData
{
public:
	VhalScreenShotData(void) noexcept
		:ivi_id_(0), type_(ScreenShotType::SCREEN)
	{}

	void SetType(const ScreenShotType type) noexcept
	{
		type_ = type;
	}

	void SetIviId(const int32_t ivi_id) noexcept
	{
		ivi_id_ = ivi_id;
	}

	void SetFilePath(const std::string &file_path)
	{
		file_path_ = file_path;
	}

	ScreenShotType GetType(void) const noexcept
	{
		return type_;
	}

	int32_t GetIviId(void) const noexcept
	{
		return ivi_id_;
	}

	std::string GetFilePath(void) const noexcept
	{
		return file_path_;
	}

private:
	int32_t ivi_id_;
	std::string file_path_;
	ScreenShotType type_;
};

class CVhalScreenShot : public CVhalWorkerThread<VhalScreenShotData> {
public:
	CVhalScreenShot(void) noexcept;
	~CVhalScreenShot(void) override;
  	CVhalScreenShot(const CVhalScreenShot& src) = delete;
	CVhalScreenShot& operator=(const CVhalScreenShot& src) & = delete;
	CVhalScreenShot(CVhalScreenShot&& src) = delete;
	CVhalScreenShot& operator=(CVhalScreenShot&& src) & = delete;

	int32_t Initialize(CVhalMainControl * const p_main, CVhalLayoutManager * const p_layout_mng);
	void Finalize(void);

private:
	int32_t Execute(const VhalScreenShotData& data, const bool terminate) const noexcept override;

	CVhalMainControl*		p_main_{nullptr};
	CVhalLayoutManager*		p_layout_mng_{nullptr};
	std::unique_ptr<CVhalEventRoute>	p_route_{nullptr};
};

} /* namespace videohal */


#endif	/* #ifndef	VHAL_SCREEN_SHOT_H */
