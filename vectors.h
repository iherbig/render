#pragma once

#include <math.h>

template <typename T>
struct Vector2 {
	T x, y;

	inline Vector2<T> operator+(const Vector2<T> &rhs) const {
		Vector2<T> result;
		result.x = x + rhs.x;
		result.y = y + rhs.y;

		return result;
	}

	inline Vector2<T> operator*(f32 rhs) const {
		Vector2<T> result;
		result.x = (T)(x * rhs);
		result.y = (T)(y * rhs);

		return result;
	}

	inline Vector2<T> operator*(int rhs) const {
		Vector2<T> result;
		result.x = (T)(x * rhs);
		result.y = (T)(y * rhs);

		return result;
	}

	inline Vector2<T> operator-(const Vector2<T> &rhs) const {
		Vector2<T> result;
		result.x = x - rhs.x;
		result.y = y - rhs.y;
		
		return result;
	}
};

template <typename T>
struct Vector3 {
	union {
		struct {
			T x, y;
		};

		Vector2<T> v2;
	};

	T z;

	inline Vector3<T> operator+(const Vector3<T> &rhs) const {
		Vector3<T> result;
		result.x = x + rhs.x;
		result.y = y + rhs.y;
		result.z = z + rhs.z;

		return result;
	}

	inline Vector3<T> operator-(const Vector3<T> &rhs) const {
		Vector3<T> result;
		result.x = x - rhs.x;
		result.y = y - rhs.y;
		result.z = z - rhs.z;

		return result;
	}

	inline Vector3<T> operator*(f32 rhs) const {
		Vector3<T> result;
		result.x = (T)(x * rhs);
		result.y = (T)(y * rhs);
		result.z = (T)(z * rhs);

		return result;
	}

	inline Vector3<T> operator*(f64 rhs) const {
		Vector3<T> result;
		result.x = (T)(x * rhs);
		result.y = (T)(y * rhs);
		result.z = (T)(z * rhs);

		return result;
	}

	inline Vector3<T> operator*(int rhs) const {
		Vector3<T> result;
		result.x = (T)(x * rhs);
		result.y = (T)(y * rhs);
		result.z = (T)(z * rhs);

		return result;
	}

	inline Vector3<T> operator/(f32 rhs) const {
		Vector3<T> result;
		result.x = (T)(x / rhs);
		result.y = (T)(y / rhs);
		result.z = (T)(z / rhs);

		return result;
	}

	inline Vector3<T> operator/(int rhs) const {
		Vector3<T> result;
		result.x = (T)(x / rhs);
		result.y = (T)(y / rhs);
		result.z = (T)(z / rhs);

		return result;
	}

	inline T dot(const Vector3<T> &rhs) const {
		return (x * rhs.x) + (y * rhs.y) + (z * rhs.z);
	}

	inline Vector3<T> cross(const Vector3<T> &rhs) {
		Vector3<T> result;
		result.x = (y * rhs.z) - (z * rhs.y);
		result.y = (z * rhs.x) - (x * rhs.z);
		result.z = (x * rhs.y) - (y * rhs.x);

		return result;
	}
	
	inline f64 length() const { 
		auto result = sqrt(x * x + y * y + z * z);
		return result;
	}

	inline Vector3<T> normalize() const {
		auto length_reciprocal = (1 / length());
		
		Vector3<T> result;
		result.x = (T)(x * length_reciprocal);
		result.y = (T)(y * length_reciprocal);
		result.z = (T)(z * length_reciprocal);

		return result;
	}
};

template <typename T>
struct Vector4 {
	union {
		struct {
			union {
				struct {
					T x, y;
				};

				Vector2<T> v2;
			};

			T z;
		};

		Vector3<T> v3;
	};

	T w;

	inline Vector4<T> operator+(const Vector4<T> &rhs) const {
		Vector4<T> result;
		result.x = x + rhs.x;
		result.y = y + rhs.y;
		result.z = z + rhs.z;
		result.w = w + rhs.w;

		return result;
	}

	inline Vector4<T> operator*(T rhs) const {
		Vector4<T> result;
		result.x = x * rhs;
		result.y = y * rhs;
		result.z = z * rhs;
		result.w = w * rhs;

		return result;
	}
};
