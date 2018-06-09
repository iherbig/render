#pragma once

#include <stdint.h>

typedef float f32;
typedef double f64;
typedef uint8_t u8;
typedef uint32_t u32;

template <typename T>
inline T clamp(T value, T low, T high) {
	if (value < low) return low;
	if (value > high) return high;
	return value;
}