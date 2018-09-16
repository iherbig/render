#pragma once

#include "types.h"
#include "vectors.h"

struct Color;

struct TextureMap {
	Color *pixel_data;
	Vector2<f32> uvs[3];
	union {
		Vector2<int> dimensions;
		struct {
			int width;
			int height;
		};
	};
};