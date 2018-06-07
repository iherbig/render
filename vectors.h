#pragma once

#include <math.h>

template <typename T>
struct Vector2 {
	T x, y;

	inline Vector2<T> operator+(const Vector2<T> &rhs) const {
		Vector2<T> result;
		result.x = this->x + rhs.x;
		result.y = this->y + rhs.y;

		return result;
	}

	inline Vector2<T> operator*(T rhs) const {
		Vector2<T> result;
		result.x = this->x * rhs;
		result.y = this->y * rhs;

		return result;
	}

	inline Vector2<T> operator-(const Vector2<T> &rhs) const {
		Vector2<T> result;
		result.x = this->x - rhs.x;
		result.y = this->y - rhs.y;
		
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
		result.x = this->x + rhs.x;
		result.y = this->y + rhs.y;
		result.z = this->z + rhs.z;

		return result;
	}

	inline Vector3<T> operator-(const Vector3<T> &rhs) const {
		Vector3<T> result;
		result.x = this->x - rhs.x;
		result.y = this->y - rhs.y;
		result.z = this->z - rhs.z;

		return result;
	}

	inline Vector3<T> operator*(T rhs) const {
		Vector3<T> result;
		result.x = this->x * rhs;
		result.y = this->y * rhs;
		result.z = this->z * rhs;

		return result;
	}

	inline Vector3<T> operator/(T rhs) const {
		Vector3<T> result;
		result.x = this->x / rhs;
		result.y = this->y / rhs;
		result.z = this->z / rhs;

		return result;
	}

	inline T dot(const Vector3<T> &rhs) const {
		return this->x * rhs.x + this->y * rhs.y + this->z * rhs.z;
	}

	inline Vector3<T> cross(const Vector3<T> &rhs) {
		Vector3<T> result;
		result.x = this->y * rhs.z - this->z * rhs.y;
		result.y = this->z * rhs.x - this->x * rhs.z;
		result.z = this->x * rhs.y - this->y * rhs.x;

		return result;
	}

	T length() const { return (T)sqrt(x * x + y * y + z * z); }
};

template <typename T>
inline Vector3<T> normalize(const Vector3<T> &vec) {
	return vec * (1 / vec.length());
}

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
		result.x = this->x + rhs.x;
		result.y = this->y + rhs.y;
		result.z = this->z + rhs.z;
		result.w = this->w + rhs.w;

		return result;
	}

	inline Vector4<T> operator*(T rhs) const {
		Vector4<T> result;
		result.x = this->x * rhs;
		result.y = this->y * rhs;
		result.z = this->z * rhs;
		result.w = this->w * rhs;

		return result;
	}
};
