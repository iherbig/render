#pragma once

#include <windows.h>
#include "types.h"
#include "color.h"
#include "vectors.h"
#include "triangle.h"

struct Backbuffer {
	BITMAPINFO info;

	int width;
	int height;
	int bytes_per_pixel;
	int stride;

	u8 *memory;
};

struct World {
	Backbuffer buffer;

	void set(int x, int y, const Color &color);
	void draw_line(int x0, int y0, int x1, int y1, const Color &color);
	void draw_line(Vector2<int> p1, Vector2<int> p2, const Color &color);
	void render(HDC context);
	void clear(const Color &color);
	void draw_triangle(Vector2<int> t0, Vector2<int> t1, Vector2<int> t2, const Color &color);
	void draw_triangle(const Triangle<f32> &triangle, f32 *z_buffer, const Color &color);

	inline Vector2<int> point_to_screen(Vector3<f32> point) {
		Vector2<int> result;

		result.x = (int)((point.x + 1) * buffer.width / 2.0 + 0.5);
		result.y = (int)((point.y + 1) * buffer.height / 2.0 + 0.5);

		return result;
	}
};
