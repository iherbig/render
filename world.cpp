#include <math.h>
#include <algorithm>

#include "types.h"
#include "world.h"
#include "color.h"

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

void World::draw_line(Vector2<int> p1, Vector2<int> p2, const Color &color) {
	draw_line(p1.x, p1.y, p2.x, p2.y, color);
}

void World::draw_triangle(Vector2<int> t0, Vector2<int> t1, Vector2<int> t2, const Color &color) {
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

void World::draw_triangle(const Triangle<f32> &triangle, const Color &color) {
	auto min_x = clamp(min(triangle.p1.x, min(triangle.p2.x, triangle.p3.x)), -1.0f, 1.0f);
	auto max_x = clamp(max(triangle.p1.x, max(triangle.p2.x, triangle.p3.x)), -1.0f, 1.0f);
	auto min_y = clamp(min(triangle.p1.y, min(triangle.p2.y, triangle.p3.y)), -1.0f, 1.0f);
	auto max_y = clamp(max(triangle.p1.y, max(triangle.p2.y, triangle.p3.y)), -1.0f, 1.0f);

	auto min_point = point_to_screen(Vector3<f32>{ min_x, min_y });
	auto max_point = point_to_screen(Vector3<f32>{ max_x, max_y });

	auto screen_p1 = point_to_screen(triangle.p1);
	auto screen_p2 = point_to_screen(triangle.p2);
	auto screen_p3 = point_to_screen(triangle.p3);

	auto screen_triangle = Triangle<f32>
	{
		Vector3<f32>{ (f32)screen_p1.x, (f32)screen_p1.y, triangle.p1.z },
		Vector3<f32>{ (f32)screen_p2.x, (f32)screen_p2.y, triangle.p2.z },
		Vector3<f32>{ (f32)screen_p3.x, (f32)screen_p3.y, triangle.p3.z },
	};

	for (auto x = min_point.x; x < max_point.x; ++x) {
		for (auto y = min_point.y; y < max_point.y; ++y) {
			auto barycentric_coefficients = screen_triangle.barycentric_coefficients_of(Vector2<f32>{ (f32)x, (f32)y });
			if (barycentric_coefficients.x < 0 || barycentric_coefficients.y < 0 || barycentric_coefficients.z < 0) continue;

			set((int)x, (int)y, color);
		}
	}
}