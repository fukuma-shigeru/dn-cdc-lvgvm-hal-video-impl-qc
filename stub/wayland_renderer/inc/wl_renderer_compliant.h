/*******************************************************************************
    機能名称    ：  静的解析対策
    ファイル名称：  wl_renderer_compliant.h
*******************************************************************************/
#ifndef	WL_RENDERER_COMPLIANT_H
#define	WL_RENDERER_COMPLIANT_H

#include <cfloat>
#include <climits>

#include "wl_renderer_log.h"

namespace wlrenderer
{

/*****************************************************************************
 処理概要：	符号無しから符号付きへのキャスト
 引数    ：	x (i)キャスト前
 戻り値  ：	キャスト後
*****************************************************************************/
static inline int32_t UI32ToI32(uint32_t x) noexcept
{
	if (static_cast<uint32_t>(INT32_MAX) < x)
	{
		WLRNDR_LOGW("x is out of range. x=%u return value=%d", x, INT32_MAX);
		x = static_cast<uint32_t>(INT32_MAX);
	}

	return static_cast<int32_t>(x);
}

/*****************************************************************************
 処理概要：	符号付きから符号無しへのキャスト
 引数    ：	x (i)キャスト前
 戻り値  ：	キャスト後
*****************************************************************************/
static inline uint32_t I32ToUI32(int32_t x) noexcept
{
	if (0 > x)
	{
		WLRNDR_LOGW("x is out of range. x=%d return value=%d", x, 0);
		x = 0;
	}

	return static_cast<uint32_t>(x);
}

/*****************************************************************************
 処理概要：	CERT INT32-C 乗算
 引数    ：	a	(i)オペランド1
			b	(i)オペランド2
 戻り値  ：	計算結果
*****************************************************************************/
static inline int32_t MulI32(const int32_t a, const int32_t b) noexcept
{
	int64_t ret{static_cast<int64_t>(a) * static_cast<int64_t>(b)};

	if (INT32_MAX < ret)
	{
		WLRNDR_LOGW("Overflow occur(a * b). a=%d b=%d return value=%d", a, b, INT32_MAX);
		ret = INT32_MAX;
	}
	else if (INT32_MIN > ret)
	{
		WLRNDR_LOGW("Overflow occur(a * b). a=%d b=%d return value=%d", a, b, INT32_MIN);
		ret = INT32_MIN;
	}
	else
	{
		/* 処理なし */
	}
	
	return static_cast<int32_t>(ret);
}

} /* namespace wlrenderer */

#endif	/* #ifndef	WL_RENDERER_COMPLIANT_H */