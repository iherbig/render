#pragma once

#include "types.h"

struct Color {
	u8 r;
	u8 g;
	u8 b;
	u8 a;
};

const Color WHITE = Color{ 255, 255, 255, 255 };
const Color BLUE  = Color{   0,   0, 255, 255 };
const Color RED   = Color{ 255,   0,   0, 255 };
const Color BLACK = Color{   0,   0,   0, 255 };
const Color GREEN = Color{   0, 255,   0, 255 };
