#pragma once

#include "types.h"

struct Mat3f {
	f32 dim[9];

	inline Mat3f operator+(const Mat3f &rhs) const {
		Mat3f result;

		for (auto index = 0; index < 9; ++index) {
			result.dim[index] = dim[index] + rhs.dim[index];
		}

		return result;
	}

	inline Mat3f operator*(const Mat3f &rhs) const {
		Mat3f result;

		for (auto lhs_row = 0; lhs_row < 3; ++lhs_row) {
			for (auto rhs_row = 0; rhs_row < 3; ++rhs_row) {
				f32 row[] = { dim[lhs_row * 3], dim[lhs_row * 3 + 1], dim[lhs_row * 3 + 2] };
				f32 col[] = { rhs.dim[rhs_row], rhs.dim[3 + rhs_row], rhs.dim[6 + rhs_row] };

				result.dim[lhs_row * 3 + rhs_row] = (row[0] * col[0]) + (row[1] * col[1]) + (row[2] * col[2]);
			}
		}

		return result;
	}

	inline Vec3f operator*(const Vec3f &rhs) const {
		Vec3f result;

		for (auto row = 0; row < 3; ++row) {
			f32 mat_row[] = { dim[row * 3], dim[row * 3 + 1], dim[row * 3 + 2] };
			result.dim[row] = (rhs.dim[0] * mat_row[0]) + (rhs.dim[1] * mat_row[1]) + (rhs.dim[2] * mat_row[2]);
		}

		return result;
	}
};

struct Mat4f {
	f32 dim[16];

	inline Mat4f operator+(const Mat4f &rhs) const {
		Mat4f result;
		
		for (auto index = 0; index < 16; ++index) {
			result.dim[index] = dim[index] + rhs.dim[index];
		}

		return result;
	}

	inline Mat4f operator*(const Mat4f &rhs) const {
		Mat4f result;

		for (auto lhs_row = 0; lhs_row < 4; ++lhs_row) {
			for (auto rhs_row = 0; rhs_row < 4; ++rhs_row) {
				f32 row[] = { dim[lhs_row * 4], dim[lhs_row * 4 + 1], dim[lhs_row * 4 + 2], dim[lhs_row * 4 + 3] };
				f32 col[] = { rhs.dim[rhs_row], rhs.dim[4 + rhs_row], rhs.dim[8 + rhs_row], rhs.dim[12 + rhs_row] };
				
				result.dim[lhs_row * 4 + rhs_row] = (row[0] * col[0]) + (row[1] * col[1]) + (row[2] * col[2]) + (row[3] * col[3]);
			}
		}

		return result;
	}

	inline Vec4f operator*(const Vec4f &rhs) const {
		Vec4f result;
		
		for (auto row = 0; row < 4; ++row) {
			f32 mat_row[] = { dim[row * 4], dim[row * 4 + 1], dim[row * 4 + 2], dim[row * 4 + 3] };
			result.dim[row] = (rhs.dim[0] * mat_row[0]) + (rhs.dim[1] * mat_row[1]) + (rhs.dim[2] * mat_row[2]) + (rhs.dim[3] * mat_row[3]);
		}

		return result;
	}

	inline Vec4f operator*(const Vec3f &rhs) const {
		Vec4f result;
		result.x = rhs.x;
		result.y = rhs.y;
		result.z = rhs.z;
		result.w = 1;

		return *this * result;
	}
};

const Mat4f Mat4_Identity = { 
	{ 
		1, 0, 0, 0, 
	    0, 1, 0, 0, 
	    0, 0, 1, 0, 
	    0, 0, 0, 1,
	} 
};

const Mat3f Mat3_Identity = {
	{
		1, 0, 0,
		0, 1, 0,
		0, 0, 1,
	}
};

inline void set_matrix_entry(Mat4f &matrix, int row, int col, f32 value) {
	matrix.dim[row * 4 + col] = value;
}

inline void set_matrix_entry(Mat3f &matrix, int row, int col, f32 value) {
	matrix.dim[row * 3 + col] = value;
}

inline Vec3f project_to_vec3f(const Vec4f &vec) {
	Vec3f result;
	result.x = vec.x / vec.w;
	result.y = vec.y / vec.w;
	result.z = vec.z / vec.w;
	return result;
}

inline Vec3i truncate_to_vec3i(const Vec3f &vec) {
	Vec3i result;
	result.x = (int)vec.x;
	result.y = (int)vec.y;
	result.z = (int)vec.z;
	return result;
}