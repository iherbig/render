#pragma once

#include <stdint.h>

typedef float  f32;
typedef double f64;

typedef  uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef  int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

template <typename T>
inline T clamp(T value, T low, T high) {
	if (value < low) return low;
	if (value > high) return high;
	return value;
}