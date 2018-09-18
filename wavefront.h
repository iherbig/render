#pragma once

#include "types.h"
#include "vectors.h"

struct Face {
	Vec3i vertex_indices;
	Vec3i texture_indices;
	Vec3i normal_indices;
};

struct WavefrontObj {
	Vec4f *verts;
	Vec3f *text_coords;
	Vec3f *vert_normals;
	Face         *faces;
};

WavefrontObj load_obj(const char *file_name);