#include <math.h>
#include <algorithm>

#include "types.h"
#include "world.h"
#include "color.h"
#include "texture.h"

void World::render(HDC context) {
	// Could probably actually handle resizing and such, but whatever.
	auto width = buffer.width;
	auto height = buffer.height;
	auto memory = buffer.memory;
	auto info = buffer.info;

	StretchDIBits(context, 0, 0, width, height, 0, 0, width, height, memory, &info, DIB_RGB_COLORS, SRCCOPY);
}

void World::clear(const Color &color) {
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

void World::set(int x, int y, const Color &color) {
	auto width = buffer.width;
	auto height = buffer.height;
	auto bytes_per_pixel = buffer.bytes_per_pixel;
	auto memory = buffer.memory;
	auto stride = buffer.stride;

	if (x < 0 || y < 0 || x >= width || y >= height) {
		return;
	}

	auto x_offset = x * bytes_per_pixel;
	auto pixel = (u32 *)&(memory[(y * stride) + x_offset]);

	// ARGB
	auto red = color.r;
	auto green = color.g;
	auto blue = color.b;
	*pixel = 0xFF << 24 | red << 16 | green << 8 | blue;
}

void World::draw_line(int x0, int y0, int x1, int y1, const Color &color) {
	auto steep = false;

	if (abs(x0 - x1) < abs(y0 - y1)) {
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;
	}

	if (x0 > x1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
	}

	// I haven't really seen this math thing before.
	// Generating an error value and tracking it over time.
	// Sort of...I guess, numerical approximation of the line.
	auto dx = x1 - x0;
	auto dy = y1 - y0;
	auto derr = abs(dy) * 2;
	auto error = 0.0f;

	auto y = y0;

	for (auto x = x0; x <= x1; ++x) {
		if (steep) {
			set(y, x, color);
		}
		else {
			set(x, y, color);
		}

		error += derr;
		if (error > dx) {
			y += (y1 > y0 ? 1 : -1);
			error -= dx * 2;
		}
	}
}

void World::draw_line(Vec2i p1, Vec2i p2, const Color &color) {
	draw_line(p1.x, p1.y, p2.x, p2.y, color);
}

void World::draw_triangle(Vec2i t0, Vec2i t1, Vec2i t2, const Color &color) {
	if (t0.y == t1.y && t0.y == t2.y) return;

	if (t0.y > t1.y) std::swap(t0, t1);
	if (t0.y > t2.y) std::swap(t0, t2);
	if (t1.y > t2.y) std::swap(t1, t2);

	auto highest = t2;
	auto middle = t1;
	auto lowest = t0;

	auto total_height = (highest.y - lowest.y);

	for (auto y = 0; y < total_height; y++) {
		auto second_half = y > middle.y - lowest.y || middle.y == lowest.y;
		auto segment_height = second_half ? highest.y - middle.y : middle.y - lowest.y;

		auto total_percent = (f32)y / total_height;
		auto segment_percent = (f32)(y - (second_half ? middle.y - lowest.y : 0)) / segment_height;

		auto left_end_point = lowest + (highest - lowest) * total_percent;
		auto right_end_point = second_half ? middle + (highest - middle) * segment_percent : lowest + (middle - lowest) * segment_percent;

		if (left_end_point.x > right_end_point.x) std::swap(left_end_point, right_end_point);

		for (auto x = left_end_point.x; x <= right_end_point.x; ++x) {
			set(x, (lowest.y + y), color);
		}
	}
}

void World::draw_triangle(const Triangle &triangle, f32 *z_buffer, const Color &color) {
	auto min_x = clamp(min(triangle.p1.x, min(triangle.p2.x, triangle.p3.x)), -1.0f, 1.0f);
	auto max_x = clamp(max(triangle.p1.x, max(triangle.p2.x, triangle.p3.x)), -1.0f, 1.0f);
	auto min_y = clamp(min(triangle.p1.y, min(triangle.p2.y, triangle.p3.y)), -1.0f, 1.0f);
	auto max_y = clamp(max(triangle.p1.y, max(triangle.p2.y, triangle.p3.y)), -1.0f, 1.0f);

	auto min_point = to_screen_coords(buffer.width, buffer.height, Vec2f{ min_x, min_y });
	auto max_point = to_screen_coords(buffer.width, buffer.height, Vec2f{ max_x, max_y });

	auto screen_p1 = to_screen_coords(buffer.width, buffer.height, triangle.p1.v2);
	auto screen_p2 = to_screen_coords(buffer.width, buffer.height, triangle.p2.v2);
	auto screen_p3 = to_screen_coords(buffer.width, buffer.height, triangle.p3.v2);

	Triangle screen_triangle =
	{
		Vec3f{ (f32)screen_p1.x, (f32)screen_p1.y, triangle.p1.z },
		Vec3f{ (f32)screen_p2.x, (f32)screen_p2.y, triangle.p2.z },
		Vec3f{ (f32)screen_p3.x, (f32)screen_p3.y, triangle.p3.z },
	};

	for (auto y = min_point.y; y < max_point.y; ++y) {
		for (auto x = min_point.x; x < max_point.x; ++x) {
			auto barycentric_coefficients = screen_triangle.barycentric_coefficients_of(x, y);
			if (barycentric_coefficients.x < 0 || barycentric_coefficients.y < 0 || barycentric_coefficients.z < 0) continue;

			// Not exactly sure why the depth has to be non-negative. 
			// Actually, I'm not even sure why some z-values are negative.
			// I have no idea what coordinate-space the obj is in.
			auto depth =
				triangle.p1.z * barycentric_coefficients.x +
				triangle.p2.z * barycentric_coefficients.y +
				triangle.p3.z * barycentric_coefficients.z;

			if (z_buffer[y * buffer.width + x] > depth) continue;

			z_buffer[y * buffer.width + x] = depth;
			set((int)x, (int)y, color);
		}
	}
}

static inline void apply_lighting(Color &color, f32 light_intensity) {
	color.r = (u8)(color.r * light_intensity);
	color.g = (u8)(color.g * light_intensity);
	color.b = (u8)(color.b * light_intensity);
}

/*
Why does this result in the default constructors and operator= being deleted?

struct SomeType;

struct Foo {
	SomeType &foo;
};
*/

void World::draw_triangle(const Triangle &triangle, const TextureMap &texture_map, const Vec2f *uvs, f32 *z_buffer, f32 light_intensity) {
	auto min_x = clamp(min(triangle.p1.x, min(triangle.p2.x, triangle.p3.x)), -1.0f, 1.0f);
	auto max_x = clamp(max(triangle.p1.x, max(triangle.p2.x, triangle.p3.x)), -1.0f, 1.0f);
	auto min_y = clamp(min(triangle.p1.y, min(triangle.p2.y, triangle.p3.y)), -1.0f, 1.0f);
	auto max_y = clamp(max(triangle.p1.y, max(triangle.p2.y, triangle.p3.y)), -1.0f, 1.0f);

	auto min_point = to_screen_coords(buffer.width, buffer.height, Vec2f{ min_x, min_y });
	auto max_point = to_screen_coords(buffer.width, buffer.height, Vec2f{ max_x, max_y });

	auto screen_p1 = to_screen_coords(buffer.width, buffer.height, triangle.p1.v2);
	auto screen_p2 = to_screen_coords(buffer.width, buffer.height, triangle.p2.v2);
	auto screen_p3 = to_screen_coords(buffer.width, buffer.height, triangle.p3.v2);

	Triangle screen_triangle =
	{
		Vec3f{ (f32)screen_p1.x, (f32)screen_p1.y, triangle.p1.z },
		Vec3f{ (f32)screen_p2.x, (f32)screen_p2.y, triangle.p2.z },
		Vec3f{ (f32)screen_p3.x, (f32)screen_p3.y, triangle.p3.z },
	};

	// This loop could be pretty easily parallelized.
	// foreach coordinate (x, y):
	//    new thread(coordinate, screen_triangle, triangle, z_value_place, color_place)
	// So rather than indexing into the buffers themselves, each thread directly inserts
	// the z-buffer and pixel color values.
	// I really just don't want to have to figure out the thread API. There are some things
	// that you can't easily do in C++.
	for (auto y = min_point.y; y < max_point.y; ++y) {
		for (auto x = min_point.x; x < max_point.x; ++x) {
			auto barycentric_coefficients = screen_triangle.barycentric_coefficients_of(x, y);
			if (barycentric_coefficients.x < 0 || barycentric_coefficients.y < 0 || barycentric_coefficients.z < 0) continue;

			// Not exactly sure why the depth has to be non-negative. 
			// Actually, I'm not even sure why some z-values are negative.
			// I have no idea what coordinate-space the obj is in.
			auto depth =
				triangle.p1.z * barycentric_coefficients.x +
				triangle.p2.z * barycentric_coefficients.y +
				triangle.p3.z * barycentric_coefficients.z;

			if (z_buffer[y * buffer.width + x] > depth) continue;

			// We have the barycentric coefficients, so we can use them to find out where to index into the texture map.
			// I spent way too long trying to figure out how to do this. But I'd forgotten that barycentric coefficients literally
			// are the value that you want. "What percentage of each vertex is a given point?"
			auto texture_map_bary_coord_x = barycentric_coefficients.x * uvs[0].x + barycentric_coefficients.y * uvs[1].x + barycentric_coefficients.z * uvs[2].x;
			auto texture_map_bary_coord_y = barycentric_coefficients.x * uvs[0].y + barycentric_coefficients.y * uvs[1].y + barycentric_coefficients.z * uvs[2].y;
			auto texture_map_coord_x = (int)(texture_map_bary_coord_x * texture_map.width);
			auto texture_map_coord_y = (int)(texture_map_bary_coord_y * texture_map.height);

			auto texture_color_index = texture_map_coord_y * texture_map.width + texture_map_coord_x;

			auto color = texture_map.pixel_data[texture_color_index];
			//auto color = WHITE;
			apply_lighting(color, light_intensity);

			z_buffer[y * buffer.width + x] = depth;
			set(x, y, color);
		}
	}
}
