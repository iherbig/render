#include <math.h>
#include <algorithm>

#include "types.h"
#include "render.h"
#include "color.h"
#include "texture.h"

void render(Backbuffer &buffer, HDC context) {
	// Could probably actually handle resizing and such, but whatever.
	auto width = buffer.width;
	auto height = buffer.height;
	auto memory = buffer.memory;
	auto info = buffer.info;

	StretchDIBits(context, 0, 0, width, height, 0, 0, width, height, memory, &info, DIB_RGB_COLORS, SRCCOPY);
}

void clear(Backbuffer &buffer, const Color &color) {
	auto width = buffer.width;
	auto height = buffer.height;
	auto bytes_per_pixel = buffer.bytes_per_pixel;
	auto memory = buffer.memory;
	auto stride = buffer.stride;

	for (auto y = 0; y < height; ++y) {
		for (auto x = 0; x < width; ++x) {
			auto x_offset = x * bytes_per_pixel;
			auto pixel = (u32 *)&(memory[y * stride + x_offset]);
			
			// ARGB
			auto red = color.r;
			auto green = color.g;
			auto blue = color.b;
			*pixel = 0xFF << 24 | red << 16 | green << 8 | blue;
		}
	}
}

void set_pixel(Backbuffer &buffer, int x, int y, const Color &color) {
	if (x < 0 || y < 0 || x >= buffer.width || y >= buffer.height) {
		return;
	}

	auto x_offset = x * buffer.bytes_per_pixel;
	auto pixel = (u32 *)&(buffer.memory[(y * buffer.stride) + x_offset]);

	// ARGB
	auto red = color.r;
	auto green = color.g;
	auto blue = color.b;
	*pixel = 0xFF << 24 | red << 16 | green << 8 | blue;
}

void draw_line(Backbuffer &buffer, Vec2i p1, Vec2i p2, const Color &color) {
	auto steep = false;

	if (abs(p1.x - p2.x) < abs(p1.y - p2.y)) {
		std::swap(p1.x, p1.y);
		std::swap(p2.x, p2.y);
		steep = true;
	}

	if (p1.x > p2.x) {
		std::swap(p1, p2);
	}

	// I haven't really seen this math thing before.
	// Generating an error value and tracking it over time.
	// Sort of...I guess, numerical approximation of the line.
	auto dxy = p2 - p1;
	auto derr = abs(dxy.y) * 2;
	auto error = 0.0f;

	auto y = p1.y;

	for (auto x = p1.x; x <= p2.x; ++x) {
		if (steep) {
			set_pixel(buffer, y, x, color);
		}
		else {
			set_pixel(buffer, x, y, color);
		}

		error += derr;
		if (error > dxy.x) {
			y += (p2.y > p1.y ? 1 : -1);
			error -= dxy.x * 2;
		}
	}
}

static inline void apply_lighting(Color &color, f32 light_intensity) {
	color.r = (u8)(color.r * light_intensity);
	color.g = (u8)(color.g * light_intensity);
	color.b = (u8)(color.b * light_intensity);
}

void draw_triangle(Backbuffer &buffer, const Triangle &triangle, const TextureMap &texture_map, const Vec2f uvs[3], f32 *z_buffer, const Vec3f normals[3], const Vec3f light_dir) {
	auto min_x = clamp(min(triangle.p1.x, min(triangle.p2.x, triangle.p3.x)), 0.0f, (f32)buffer.width);
	auto max_x = clamp(max(triangle.p1.x, max(triangle.p2.x, triangle.p3.x)), 0.0f, (f32)buffer.height);
	auto min_y = clamp(min(triangle.p1.y, min(triangle.p2.y, triangle.p3.y)), 0.0f, (f32)buffer.width);
	auto max_y = clamp(max(triangle.p1.y, max(triangle.p2.y, triangle.p3.y)), 0.0f, (f32)buffer.height);

	auto min_point = Vec2f{ min_x, min_y };
	auto max_point = Vec2f{ max_x, max_y };

	auto intensity = Vec3f{
		normalize(normals[0]).dot(light_dir),
		normalize(normals[1]).dot(light_dir),
		normalize(normals[2]).dot(light_dir)
	};

	// This loop could be pretty easily parallelized.
	// foreach coordinate (x, y):
	//    new thread(coordinate, screen_triangle, triangle, z_value_place, color_place)
	// So rather than indexing into the buffers themselves, each thread directly inserts
	// the z-buffer and pixel color values.
	// I really just don't want to have to figure out the thread API. There are some things
	// that you can't easily do in C++.
	for (auto y = (int)min_point.y; y < max_point.y; ++y) {
		for (auto x = (int)min_point.x; x < max_point.x; ++x) {
			auto barycentric_coefficients = triangle.barycentric_coefficients_of(x, y);
			if (barycentric_coefficients.x < 0 || barycentric_coefficients.y < 0 || barycentric_coefficients.z < 0) continue;

			auto depth =
				triangle.p1.z * barycentric_coefficients.x +
				triangle.p2.z * barycentric_coefficients.y +
				triangle.p3.z * barycentric_coefficients.z;

			if (z_buffer[y * buffer.width + x] >= depth) continue;

			auto light_intensity = barycentric_coefficients.dot(intensity);

			if (light_intensity <= 0) continue;

			// We have the barycentric coefficients, so we can use them to find out where to index into the texture map.
			// I spent way too long trying to figure out how to do this. But I'd forgotten that barycentric coefficients literally
			// are the value that you want. "What percentage of each vertex is a given point?"
			auto texture_map_bary_coord_x = barycentric_coefficients.x * uvs[0].x + barycentric_coefficients.y * uvs[1].x + barycentric_coefficients.z * uvs[2].x;
			auto texture_map_bary_coord_y = barycentric_coefficients.x * uvs[0].y + barycentric_coefficients.y * uvs[1].y + barycentric_coefficients.z * uvs[2].y;
			auto texture_map_coord_x = (int)(texture_map_bary_coord_x * texture_map.width);
			auto texture_map_coord_y = (int)(texture_map_bary_coord_y * texture_map.height);

			auto color = texture_map.pixel_data[texture_map_coord_y * texture_map.width + texture_map_coord_x];

			//auto color = WHITE;
			apply_lighting(color, light_intensity);

			z_buffer[y * buffer.width + x] = depth;
			set_pixel(buffer, x, y, color);
		}
	}
}
