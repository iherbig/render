#pragma once

#include <math.h>

#include "types.h"
#include "vectors.h"

template <typename T>
struct Triangle {
	Vector2<T> p1;
	Vector2<T> p2;
	Vector2<T> p3;

	Vector3<f32> barycentric_coefficients_of(Vector2<T> point) const {
		auto vec_1 = Vector3<f32>{ (f32)(p3.x - p1.x), (f32)(p2.x - p1.x), (f32)(p1.x - point.x) };
		auto vec_2 = Vector3<f32>{ (f32)(p3.y - p1.y), (f32)(p2.y - p1.y), (f32)(p1.y - point.y) };
		auto non_normalized_coefficients = vec_1.cross(vec_2);

		if (abs(non_normalized_coefficients.z) < 1) return Vector3<f32>{ -1, 1, 1 };

		auto barycentric_coefficients = Vector3<f32>
		{
			non_normalized_coefficients.x / non_normalized_coefficients.z,
			non_normalized_coefficients.y / non_normalized_coefficients.z
		};

		auto result = Vector3<f32>
		{
			1.0f - (barycentric_coefficients.x + barycentric_coefficients.y),
			barycentric_coefficients.x,
			barycentric_coefficients.y
		};

		return result;
	}
};