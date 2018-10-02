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

void set_pixel(Backbuffer &buffer, int x, int y, const Color &color);
void render(Backbuffer &buffer, HDC context);
void clear(Backbuffer &buffer, const Color &color);
void draw_line(Backbuffer &buffer, Vec2i p1, Vec2i p2, const Color &color);
void draw_triangle(Backbuffer &buffer, const Triangle &triangle, const TextureMap &texture_map, const Vec2f uvs[3], f32 *z_buffer, const Vec3f normals[3], const Vec3f light_dir);
