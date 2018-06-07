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
	int bytesPerPixel;
	int stride;

	u8 *memory;

	void set(int x, int y, const Color &color);
	void draw_line(int x0, int y0, int x1, int y1, const Color &color);
	void draw_line(Vector2<f32> p1, Vector2<f32> p2, const Color &color);
	void render(HDC context);
	void clear(const Color &color);
	void draw_triangle(Vector2<f32> t0, Vector2<f32> t1, Vector2<f32> t2, const Color &color);
};
