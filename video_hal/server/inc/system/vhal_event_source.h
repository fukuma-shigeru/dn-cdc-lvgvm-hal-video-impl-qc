/*******************************************************************************
    機能名称    ：  内部イベントソースモジュール
    ファイル名称：  vhal_event_source.h
*******************************************************************************/
#ifndef	VHAL_EVENT_SOURCE_H
#define	VHAL_EVENT_SOURCE_H

#include <cstdint>

namespace videohal
{

static constexpr uint32_t kSourceEventReadable{0x01U};
static constexpr uint32_t kSourceEventWritable{0x02U};
static constexpr uint32_t kSourceEventHungup{0x04U};
static constexpr uint32_t kSourceEventError{0x08U};

/*****************************************************************************
 クラス名称：CVhalEventSource
 処理概要  ：内部イベントの送信ベースクラス
*****************************************************************************/
class CVhalEventSource {
public:
	CVhalEventSource(void) noexcept = default;
	virtual ~CVhalEventSource(void) = default;
  	CVhalEventSource(const CVhalEventSource& src) = delete;
	CVhalEventSource(CVhalEventSource&& src) = delete;

	virtual int32_t Initialize(void) = 0;

	/* イベント受信用fdの取得 */
	virtual int32_t GetSourceFd(void) const = 0;
	/* イベントの読み込みと実行 */
	virtual int32_t ExecEvent(const uint32_t source_event) = 0;

protected:
	CVhalEventSource& operator=(const CVhalEventSource& src) & = delete;
	CVhalEventSource& operator=(CVhalEventSource&& src) & = delete;

private:

};

} /* namespace videohal */

#endif	/* #ifndef	VHAL_EVENT_SOURCE_H */
