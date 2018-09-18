#pragma once

#include <math.h>

#include "types.h"
#include "vectors.h"

struct Triangle {
	Vec3f p1;
	Vec3f p2;
	Vec3f p3;

	// Basically all of my time is being spent in this method, with most of _that_
	// being spent divided between cross and fabsf. I'm not sure how to make that any better.
	Vec3f barycentric_coefficients_of(const int x, const int y) const;
};