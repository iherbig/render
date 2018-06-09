#include <math.h>
#include <algorithm>

#include "backbuffer.h"
#include "color.h"

void Backbuffer::render(HDC context) {
	// Could probably actually handle resizing and such, but whatever.
	StretchDIBits(context, 0, 0, width, height, 0, 0, width, height, memory, &info, DIB_RGB_COLORS, SRCCOPY);
}

void Backbuffer::clear(const Color &color) {
	for (auto row = 0; row < width; ++row) {
		for (auto col = 0; col < height; ++col) {
			auto row_byte_offset = row * bytes_per_pixel;
			auto pixel = (u32 *)&(memory[col * stride + row_byte_offset]);
			
			// ARGB
			auto red = color.r;
			auto green = color.g;
			auto blue = color.b;
			*pixel = 0xFF << 24 | red << 16 | green << 8 | blue;
		}
	}
}

void Backbuffer::set(int x, int y, const Color &color) {
	if (x < 0 || y < 0 || x >= width || y >= height) {
		return;
	}

	auto x_byte_offset = x * bytes_per_pixel;
	auto pixel = (u32 *)&(memory[(y * stride) + x_byte_offset]);

	// ARGB
	auto red = color.r;
	auto green = color.g;
	auto blue = color.b;
	*pixel = 0xFF << 24 | red << 16 | green << 8 | blue;
}

void Backbuffer::draw_line(int x0, int y0, int x1, int y1, const Color &color) {
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

void Backbuffer::draw_line(Vector2<int> p1, Vector2<int> p2, const Color &color) {
	draw_line(p1.x, p1.y, p2.x, p2.y, color);
}

void Backbuffer::draw_triangle(Vector2<int> t0, Vector2<int> t1, Vector2<int> t2, const Color &color) {
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
			// This might be the culprit for the artifacting?
			// This is really one of those things that I wish I had someone to ask.
			set(x, (lowest.y + y), color);
		}
	}
}

void Backbuffer::draw_triangle(const Triangle<int> &triangle, int width, int height, const Color &color) {
	auto min_x = max(min(triangle.p1.x, min(triangle.p2.x, triangle.p3.x)), 0);
	auto max_x = min(max(triangle.p1.x, max(triangle.p2.x, triangle.p3.x)), width);
	auto min_y = max(min(triangle.p1.y, min(triangle.p2.y, triangle.p3.y)), 0);
	auto max_y = min(max(triangle.p1.y, max(triangle.p2.y, triangle.p3.y)), height);

	for (auto x = min_x; x < max_x; ++x) {
		for (auto y = min_y; y < max_y; ++y) {
			auto barycentric_coefficients = triangle.barycentric_coefficients_of(Vector2<int>{ x, y });
			if (barycentric_coefficients.x < 0 || barycentric_coefficients.y < 0 || barycentric_coefficients.z < 0) continue;
			set(x, y, color);
		}
	}
}