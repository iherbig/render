#pragma once

#include <windows.h>
#include "types.h"
#include "color.h"
#include "vectors.h"

struct Backbuffer {
	Backbuffer() {};

	BITMAPINFO info;

	int width;
	int height;
	int bytes_per_pixel;
	int stride;

	u8 *memory;

	void set(int x, int y, const Color &color);
	void draw_line(int x0, int y0, int x1, int y1, const Color &color);
	void draw_line(Vector2<int> p1, Vector2<int> p2, const Color &color);
	void render(HDC context);
	void clear(const Color &color);
	void draw_triangle(Vector2<int> t0, Vector2<int> t1, Vector2<int> t2, const Color &color);
};
