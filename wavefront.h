#pragma once

#include "types.h"
#include "vectors.h"
#include "array.h"

struct Face {
	Vector3<int> vertex_indices;
	Vector3<int> texture_indices;
	Vector3<int> normal_indices;
};

struct WavefrontObj {
	Array<Vector4<f32>> verts;
	Array<Vector3<f32>> text_coords;
	Array<Vector3<f32>> vert_normals;
	Array<Face>    faces;
};

WavefrontObj load_obj(const char *file_name);