#pragma once

#include <math.h>

#include "types.h"
#include "vectors.h"

template <typename T>
struct Triangle {
	Vector3<T> p1;
	Vector3<T> p2;
	Vector3<T> p3;

	Vector3<f32> barycentric_coefficients_of(int x, int y) const {
		auto vec_1 = Vector3<f32>{ p3.x - p1.x, p2.x - p1.x, p1.x - x };
		auto vec_2 = Vector3<f32>{ p3.y - p1.y, p2.y - p1.y, p1.y - y };
		auto non_normalized_coefficients = vec_1.cross(vec_2);

		if (fabs(non_normalized_coefficients.z) < 0.01) return Vector3<f32>{ -1, 1, 1 };

		auto barycentric_coefficients = Vector3<f32>
		{
			non_normalized_coefficients.x / non_normalized_coefficients.z,
			non_normalized_coefficients.y / non_normalized_coefficients.z
		};

		auto result = Vector3<f32>
		{
			// This is NOT equivalent to 1.0f - barycentric_coefficients.x - barycentric_coefficients.y
			// Floating point is hard. Moreover, it seems like the implementaiton right now
			// actually sort of relies on the fuzziness.
			1.0f - (barycentric_coefficients.x + barycentric_coefficients.y),
			barycentric_coefficients.y,
			barycentric_coefficients.x
		};

		return result;
	}
};