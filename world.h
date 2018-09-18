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

struct TextureMap;

struct World {
	Backbuffer buffer;

	void set(int x, int y, const Color &color);
	void draw_line(int x0, int y0, int x1, int y1, const Color &color);
	void draw_line(Vec2i p1, Vec2i p2, const Color &color);
	void render(HDC context);
	void clear(const Color &color);
	void draw_triangle(Vec2i t0, Vec2i t1, Vec2i t2, const Color &color);
	void draw_triangle(const Triangle &triangle, f32 *z_buffer, const Color &color);
	void draw_triangle(const Triangle &triangle, const TextureMap &texture_map, f32 *z_buffer, f32 light_intensity);
};

inline Vec2i to_screen_coords(int client_width, int client_height, Vec2f point) {
	Vec2i result = { (int)((point.x + 1) * client_width * 0.5f), (int)((point.y + 1) * client_height * 0.5f) };
	return result;
}
