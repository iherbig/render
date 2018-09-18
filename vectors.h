#pragma once

#include <math.h>

#include "types.h"

struct Vec2f {
	union {
		struct {
			f32 x, y;
		};
		f32 dim[2];
	};

	inline Vec2f operator+(const Vec2f &rhs) const {
		Vec2f result = { x + rhs.x, y + rhs.y };
		return result;
	}

	inline Vec2f operator*(const f32 rhs) const {
		Vec2f result = { x * rhs, y * rhs };
		return result;
	}

	inline Vec2f operator-(const Vec2f &rhs) const {
		Vec2f result = { x - rhs.x, y - rhs.y };
		return result;
	}
};

struct Vec2i {
	s32 x, y;

	inline Vec2i operator+(const Vec2i &rhs) const {
		Vec2i result = { x + rhs.x, y + rhs.y };
		return result;
	}

	inline Vec2i operator*(const f32 rhs) const {
		Vec2i result = { (int)(x * rhs), (int)(y * rhs) };
		return result;
	}

	inline Vec2i operator-(const Vec2i &rhs) const {
		Vec2i result = { x - rhs.x, y - rhs.y };
		return result;
	}
};

struct Vec3f {
	union {
		struct {
			union {
				struct {
					f32 x, y;
				};
				Vec2f v2;
			};

			f32 z;
		};
		f32 dim[3];
	};

	inline Vec3f operator+(const Vec3f &rhs) const {
		Vec3f result = { x + rhs.x, y + rhs.y, z + rhs.z };
		return result;
	}

	inline Vec3f operator-(const Vec3f &rhs) const {
		Vec3f result = { x - rhs.x, y - rhs.y, z - rhs.z };
		return result;
	}

	inline Vec3f operator*(const f32 rhs) const {
		Vec3f result = { x * rhs, y * rhs, z * rhs };
		return result;
	}

	inline Vec3f operator/(const f32 rhs) const {
		Vec3f result = { x / rhs, y / rhs, z / rhs };
		return result;
	}

	inline f32 dot(const Vec3f &rhs) const {
		return (x * rhs.x) + (y * rhs.y) + (z * rhs.z);
	}

	inline Vec3f cross(const Vec3f &rhs) const {
		Vec3f result =
		{
			(y * rhs.z) - (z * rhs.y),
			(z * rhs.x) - (x * rhs.z),
			(x * rhs.y) - (y * rhs.x)
		};

		return result;
	}

	inline f64 length() const {
		auto result = sqrt(x * x + y * y + z * z);
		return result;
	}

	inline Vec3f normalize() const {
		auto length_reciprocal = (1.0f / length());
		Vec3f result = { (f32)(x * length_reciprocal), (f32)(y * length_reciprocal), (f32)(z * length_reciprocal) };
		return result;
	}
};

struct Vec3i {
	union {
		struct {
			union {
				struct {
					int x, y;
				};
				Vec2i v2;
			};

			int z;
		};
		int dim[3];
	};

	inline Vec3i operator+(const Vec3i &rhs) const {
		Vec3i result = { x + rhs.x, y + rhs.y, z + rhs.z };
		return result;
	}

	inline Vec3i operator-(const Vec3i &rhs) const {
		Vec3i result = { x - rhs.x, y - rhs.y, z - rhs.z };
		return result;
	}

	inline Vec3i operator*(const f32 rhs) const {
		Vec3i result = { (int)(x * rhs), (int)(y * rhs), (int)(z * rhs) };
		return result;
	}

	inline Vec3i operator/(const f32 rhs) const {
		Vec3i result = { (int)(x / rhs), (int)(y / rhs), (int)(z / rhs) };
		return result;
	}

	inline int dot(const Vec3i &rhs) const {
		return (x * rhs.x) + (y * rhs.y) + (z * rhs.z);
	}

	inline Vec3i cross(const Vec3i &rhs) const {
		Vec3i result =
		{
			(y * rhs.z) - (z * rhs.y),
			(z * rhs.x) - (x * rhs.z),
			(x * rhs.y) - (y * rhs.x)
		};

		return result;
	}

	inline f32 length() const {
		auto result = sqrt(x * x + y * y + z * z);
		return (f32)result;
	}

	inline Vec3f normalize() const {
		auto length_reciprocal = (1.0f / length());
		Vec3f result = { x * length_reciprocal, y * length_reciprocal, z * length_reciprocal };
		return result;
	}
};

struct Vec4f {
	union {
		struct {
			union {
				struct {
					union {
						struct {
							f32 x, y;
						};
						Vec2f v2;
					};

					f32 z;
				};

				Vec3f v3;
			};

			f32 w;
		};
		f32 dim[4];
	};

	inline Vec4f operator+(const Vec4f &rhs) const {
		Vec4f result = { x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w };
		return result;
	}

	inline Vec4f operator*(const f32 rhs) const {
		Vec4f result = { x * rhs, y * rhs, z * rhs, w * rhs };
		return result;
	}
};
