/*******************************************************************************
    機能名称    ：  値丸め有りキャストモジュール
    ファイル名称：  vhal_round_cast.h
*******************************************************************************/
#ifndef	VHAL_ROUND_CAST_H
#define	VHAL_ROUND_CAST_H

#include <cfloat>
#include <climits>
#include <type_traits>

#include "vhal_log.h"

namespace videohal
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
		VHAL_LOGW("x is out of range. x=%u return value=%d", x, INT32_MAX);
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
		VHAL_LOGW("x is out of range. x=%d return value=%d", x, 0);
		x = 0;
	}

	return static_cast<uint32_t>(x);
}

/*****************************************************************************
 処理概要：	符号付きから符号無しへのキャスト
 引数    ：	x (i)キャスト前
 戻り値  ：	キャスト後
*****************************************************************************/
static inline uint64_t I32ToUI64(int32_t x) noexcept
{
	if (0 > x)
	{
		VHAL_LOGW("x is out of range. x=%d return value=%d", x, 0);
		x = 0;
	}

	return static_cast<uint64_t>(x);
}

/*****************************************************************************
 処理概要：	符号付き同士の高精度から低精度へのキャスト
 引数    ：	x (i)キャスト前
 戻り値  ：	キャスト後
*****************************************************************************/
static inline int32_t I64ToI32(int64_t x) noexcept
{
	if (INT32_MAX < x)
	{
		VHAL_LOGW("x is out of range. x=%ld return value=%d", x, INT32_MAX);
		x = INT32_MAX;
	}

	if (INT32_MIN > x)
	{
		VHAL_LOGW("x is out of range. x=%ld return value=%d", x, INT32_MIN);
		x = INT32_MIN;
	}

	return static_cast<int32_t>(x);
}

/*****************************************************************************
 処理概要：	符号付き同士の高精度から低精度へのキャスト
 引数    ：	x (i)キャスト前
 戻り値  ：	キャスト後
*****************************************************************************/
static inline uint32_t UI64ToUI32(uint64_t x) noexcept
{
	if (UINT32_MAX < x)
	{
		VHAL_LOGW("x is out of range. x=%lu return value=%d", x, UINT32_MAX);
		x = UINT32_MAX;
	}

	return static_cast<uint32_t>(x);
}

/*****************************************************************************
 処理概要：	符号無し同士の高精度から低精度へのキャスト
 引数    ：	x (i)キャスト前
 戻り値  ：	キャスト後
*****************************************************************************/
static inline uint32_t SizeToUI32(std::size_t x) noexcept
{
	if (UINT32_MAX < x)
	{
		VHAL_LOGW("x is out of range. x=%lu return value=%u", x, UINT32_MAX);
		x = UINT32_MAX;
	}

	return static_cast<uint32_t>(x);
}

/*****************************************************************************
 処理概要：	符号無しから符号付きへのキャスト
 引数    ：	x (i)キャスト前
 戻り値  ：	キャスト後
*****************************************************************************/
static inline int32_t UI64ToI32(uint64_t x) noexcept
{
	if (static_cast<uint64_t>(INT32_MAX) < x)
	{
		VHAL_LOGW("x is out of range. x=%lu return value=%d", x, INT32_MAX);
		x = static_cast<uint32_t>(INT32_MAX);
	}

	return static_cast<int32_t>(x);
}

/*****************************************************************************
 処理概要：	符号付きから符号無しへのキャスト
 引数    ：	x (i)キャスト前
 戻り値  ：	キャスト後
*****************************************************************************/
static inline uint8_t I32ToUI8(int32_t x) noexcept
{
	if (0 > x)
	{
		VHAL_LOGW("x is out of range. x=%d return value=%d", x, 0);
		x = 0;
	}

	if (UINT8_MAX < x)
	{
		VHAL_LOGW("x is out of range. x=%d return value=%d", x, UINT8_MAX);
		x = UINT8_MAX;
	}

	return static_cast<uint8_t>(x);
}

/*****************************************************************************
 処理概要：	符号付きから符号なし、高精度から低精度へのキャスト
 引数    ：	x (i)キャスト前
 戻り値  ：	キャスト後
*****************************************************************************/
static inline uint32_t I64ToUI32(int64_t x) noexcept
{
	if (static_cast<int64_t>(UINT32_MAX) < x)
	{
		VHAL_LOGW("x is out of range. x=%ld return value=%d", x, UINT32_MAX);
		x = static_cast<int64_t>(UINT32_MAX);
	}

	if (0 > x)
	{
		VHAL_LOGW("x is out of range. x=%ld return value=%d", x, 0);
		x = 0;
	}

	return static_cast<uint32_t>(x);
}

/*****************************************************************************
 処理概要：	符号付きから符号なしへのキャスト
 引数    ：	x (i)キャスト前
 戻り値  ：	キャスト後
*****************************************************************************/
static inline uint64_t I64ToUI64(int64_t x) noexcept
{
	if (0 > x)
	{
		VHAL_LOGW("x is out of range. x=%ld return value=%d", x, 0);
		x = 0;
	}

	return static_cast<uint64_t>(x);
}

/*****************************************************************************
 処理概要：	boolから符号なしへのキャスト
 引数    ：	x (i)キャスト前
 戻り値  ：	キャスト後
*****************************************************************************/
static inline uint8_t BToUI8(const bool x) noexcept
{
	return (true == x) ? 1U : 0U;
}

/*****************************************************************************
 処理概要：	boolから文字列変換
 引数    ：	x (i)bool値
 戻り値  ：	文字列
*****************************************************************************/
static inline const char* BToCh(const bool x) noexcept
{
	return (true == x) ? "true" : "false";
}

/*****************************************************************************
 処理概要：	各数値型からboolへのキャスト
 引数    ：	x (i)キャスト前
 戻り値  ：	キャスト後
*****************************************************************************/
template<typename T>
static bool ToB(const T x) noexcept
{
	return (0 == x) ? false : true;
}

/*****************************************************************************
 処理概要：	各数値型からuint8_tへのキャスト
 引数    ：	x (i)キャスト前
 戻り値  ：	キャスト後
*****************************************************************************/
template<typename T>
static uint8_t ToUI8(const T x) noexcept
{
	uint64_t ux{static_cast<uint64_t>(x)};
	constexpr uint64_t uint8_max{UINT8_MAX};

	if (uint8_max < ux)
	{
		VHAL_LOGW("x is out of range. x=%llu return value=llu", ux, uint8_max);
		ux = uint8_max;
	}

	return static_cast<uint8_t>(ux);
}

/*****************************************************************************
 処理概要：	各数値型からuint32_tへのキャスト
 引数    ：	x (i)キャスト前
 戻り値  ：	キャスト後
*****************************************************************************/
template<typename T>
static uint32_t ToUI32(const T x) noexcept
{
	uint64_t ux{static_cast<uint64_t>(x)};
	constexpr uint64_t uint32_max{UINT32_MAX};

	if (uint32_max < ux)
	{
		VHAL_LOGW("x is out of range. x=%llu return value=%llu", ux, uint32_max);
		ux = uint32_max;
	}

	return static_cast<uint32_t>(ux);
}

/*****************************************************************************
 処理概要：	各数値型からint32_tへのキャスト
 引数    ：	x (i)キャスト前
 戻り値  ：	キャスト後
 注意    ： 各整数型、enum型、および浮動小数点型からint32_tへのキャスト
*****************************************************************************/
template<typename T>
static inline int32_t ToI32(const T x) noexcept
{
	using RawT = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
	if constexpr (std::is_enum<RawT>::value)
	{
		using UnderlyingT = typename std::underlying_type<RawT>::type;
		return ToI32(static_cast<UnderlyingT>(x));	/* 次回は必ずelse側に飛ぶためスタック不足にならない */
	}
	else if constexpr (std::is_integral<RawT>::value && std::is_unsigned<RawT>::value)
	{
		return UI64ToI32(static_cast<uint64_t>(x));
	}
	else
	{
		return I64ToI32(static_cast<int64_t>(x));
	}
}

/*****************************************************************************
 処理概要：	FLP34-C
 引数    ：	x	(i)キャスト前
 戻り値  ：	キャスト後
*****************************************************************************/
static inline uint32_t DToUI32(const double x) noexcept
{
	if ((x >= (static_cast<double>(UINT32_MAX) - 1.0)) || ((x >= 0.0F) && (x < FLT_MIN)))
	{
		VHAL_LOGW("x is unrepresentable value in uint32_t. x=%lf", x);
	}

	return static_cast<uint32_t>(x);
}

/*****************************************************************************
 処理概要：	CERT INT32-C 加算
 引数    ：	a	(i)オペランド1
			b	(i)オペランド2
 戻り値  ：	計算結果
*****************************************************************************/
static inline int32_t AddI32(const int32_t a, const int32_t b) noexcept
{
	int32_t ret{};

	if ((b > 0) && (a > (INT32_MAX - b)))
	{
		VHAL_LOGW("Overflow occur(a + b). a=%d b=%d return value=%d", a, b, INT32_MAX);
		ret = INT32_MAX;
	}
	else if ((b < 0) && (a < (INT32_MIN - b)))
	{
		VHAL_LOGW("Overflow occur(a + b). a=%d b=%d return value=%d", a, b, INT32_MIN);
		ret = INT32_MIN;
	}
	else
	{
    	ret = a + b;
	}

	return ret;
}

/*****************************************************************************
 処理概要：	CERT INT32-C 減算
 引数    ：	a	(i)オペランド1
			b	(i)オペランド2
 戻り値  ：	計算結果
*****************************************************************************/
static inline int32_t SubI32(const int32_t a, const int32_t b) noexcept
{
	int32_t ret{};

	if ((b > 0) && (a < INT32_MIN + b))
	{
		VHAL_LOGW("Overflow occur(a - b). a=%d b=%d return value=%d", a, b, INT32_MIN);
		ret = INT32_MIN;
	}
	else if ((b < 0) && (a > INT32_MAX + b))
	{
		VHAL_LOGW("Overflow occur(a - b). a=%d b=%d return value=%d", a, b, INT32_MAX);
		ret = INT32_MAX;
	}
	else
	{
		ret = a - b;
	}

	return ret;
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
		VHAL_LOGW("Overflow occur(a * b). a=%d b=%d return value=%d", a, b, INT32_MAX);
		ret = INT32_MAX;
	}
	else if (INT32_MIN > ret)
	{
		VHAL_LOGW("Overflow occur(a * b). a=%d b=%d return value=%d", a, b, INT32_MIN);
		ret = INT32_MIN;
	}
	else
	{
		/* 処理なし */
	}
	
	return static_cast<int32_t>(ret);
}

/*****************************************************************************
 処理概要：	CERT INT32-C 除算
 引数    ：	a	(i)オペランド1
			b	(i)オペランド2
 戻り値  ：	計算結果
*****************************************************************************/
static inline int32_t DivI32(const int32_t a, const int32_t b) noexcept
{
	int32_t ret{};

	if (b == 0)
	{
		VHAL_LOGW("Overflow occur(a / b). a=%d b=%d return value=%d", a, b, a);
		ret = a;
	}
	else if ((a == INT32_MIN) && (b == -1))
	{
		VHAL_LOGW("Overflow occur(a / b). a=%d b=%d return value=%d", a, b, a);
		ret = a;
	}
	else
	{
		ret = a / b;
	}

	return ret;
}

} /* namespace videohal */

#endif	/* #ifndef	VHAL_ROUND_CAST_H */