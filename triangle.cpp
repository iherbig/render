#include "triangle.h"

Vec3f Triangle::barycentric_coefficients_of(const int x, const int y) const {
	auto vec_1 = Vec3f{ p3.x - p1.x, p2.x - p1.x, p1.x - x };
	auto vec_2 = Vec3f{ p3.y - p1.y, p2.y - p1.y, p1.y - y };
	
	auto non_normalized_coefficients = vec_1.cross(vec_2);

	if (fabsf(non_normalized_coefficients.z) < 0.01f) return Vec3f{ -1, 1, 1 };

	auto normalized_x = non_normalized_coefficients.x / non_normalized_coefficients.z;
	auto normalized_y = non_normalized_coefficients.y / non_normalized_coefficients.z;

	// This is NOT equivalent to 1.0f - barycentric_coefficients.x - barycentric_coefficients.y
	// Floating point is hard. Moreover, it seems like the implementation right now
	// actually sort of relies on the fuzziness.
	auto bary_x = 1.0f - (normalized_x + normalized_y);
	auto bary_y = normalized_y;
	auto bary_z = normalized_x;

	if ((bary_x + bary_y + bary_z) - 1.0f > 0.001f) return Vec3f{ -1, 1, 1 };

	return Vec3f { bary_x, bary_y, bary_z };
}