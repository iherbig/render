#pragma once

#include "types.h"
#include "vectors.h"

struct Color;

struct TextureMap {
	Color *pixel_data;
	union {
		Vec2i dimensions;
		struct {
			int width;
			int height;
		};
	};
};