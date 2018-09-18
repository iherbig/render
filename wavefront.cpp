#include <windows.h>
#include <stdio.h>

#include "types.h"
#include "utils.h"
#include "wavefront.h"
#include "stretchy_buffer.h"

WavefrontObj load_obj(const char *file_name) {
	WavefrontObj result = {};
	auto file_contents = read_entire_file(file_name);
	auto cursor = file_contents.result;

	if (!file_contents.read) {
		return result;
	}

	defer { VirtualFree(file_contents.result, 0, MEM_RELEASE); };

	char *cursor_tracker;
	char *line_tracker;
	auto line = strtok_s(cursor, "\n", &cursor_tracker);

	do {
		auto token = strtok_s(line, " \t", &line_tracker);

		if (!token) {
			// Just an empty line.
		}
		else if (strcmp(token, "v") == 0) {
			Vec4f vert = {};
			vert.w = 1;

			token = strtok_s(NULL, " \t", &line_tracker);
			vert.x = (f32)atof(token);
			token = strtok_s(NULL, " \t", &line_tracker);
			vert.y = (f32)atof(token);
			token = strtok_s(NULL, " \t", &line_tracker);
			vert.z = (f32)atof(token);
			token = strtok_s(NULL, " \t", &line_tracker);

			if (token) {
				vert.w = (f32)atof(token);
			}

			sb_push(result.verts, vert);
		}
		else if (strcmp(token, "vn") == 0) {
			Vec3f normal = {};

			token = strtok_s(NULL, " \t", &line_tracker);
			normal.x = (f32)atof(token);
			token = strtok_s(NULL, " \t", &line_tracker);
			normal.y = (f32)atof(token);
			token = strtok_s(NULL, " \t", &line_tracker);
			normal.z = (f32)atof(token);

			sb_push(result.vert_normals, normal);
		}
		else if (strcmp(token, "vt") == 0) {
			Vec3f text = {};

			token = strtok_s(NULL, " \t", &line_tracker);
			text.x = (f32)atof(token);
			token = strtok_s(NULL, " \t", &line_tracker);
			text.y = (f32)atof(token);
			token = strtok_s(NULL, " \t", &line_tracker);

			if (token) {
				text.z = (f32)atof(token);
			}

			sb_push(result.text_coords, text);
		}
		else if (strcmp(token, "f") == 0) {
			Face face = {};
			auto has_texture = false;
			auto has_normal = false;

			token = strtok_s(NULL, " \t", &line_tracker);
			auto token_cursor = token;
			char *first_slash = 0;
			char *last_slash = 0;

			while (*token_cursor != '\0') {
				if (*token_cursor == '/') {
					if (!has_normal) {
						first_slash = token_cursor;
					}
					else {
						last_slash = token_cursor;
					}

					has_normal = true;
				}

				token_cursor++;
			}

			if (has_normal && last_slash - first_slash > 1) {
				has_texture = true;
			}

			if (has_normal) {
				{
					char *index_tracker;
					auto index = strtok_s(token, "/", &index_tracker);
					face.vertex_indices.x = atoi(index) - 1; // Just assume it's fine...

					if (has_texture) {
						index = strtok_s(NULL, "/", &index_tracker);
						face.texture_indices.x = atoi(index) - 1;
					}

					index = strtok_s(NULL, "/", &index_tracker);
					face.normal_indices.x = atoi(index) - 1;
				}

				token = strtok_s(NULL, " \t", &line_tracker);

				{
					char *index_tracker;
					auto index = strtok_s(token, "/", &index_tracker);
					face.vertex_indices.y = atoi(index) - 1; // Just assume it's fine...

					if (has_texture) {
						index = strtok_s(NULL, "/", &index_tracker);
						face.texture_indices.y = atoi(index) - 1;
					}

					index = strtok_s(NULL, "/", &index_tracker);
					face.normal_indices.y = atoi(index) - 1;
				}

				token = strtok_s(NULL, " \t", &line_tracker);

				{
					char *index_tracker;
					auto index = strtok_s(token, "/", &index_tracker);
					face.vertex_indices.z = atoi(index) - 1; // Just assume it's fine...

					if (has_texture) {
						index = strtok_s(NULL, "/", &index_tracker);
						face.texture_indices.z = atoi(index) - 1;
					}

					index = strtok_s(NULL, "/", &index_tracker);
					face.normal_indices.z = atoi(index) - 1;
				}
			}
			else {
				face.vertex_indices.x = atoi(token) - 1;
				token = strtok_s(NULL, " \t", &line_tracker);
				face.vertex_indices.y = atoi(token) - 1;
				token = strtok_s(NULL, " \t", &line_tracker);
				face.vertex_indices.z = atoi(token) - 1;
			}

			sb_push(result.faces, face);
		}
		else if (strcmp(token, "#") == 0) {
			// This is just a comment, discard.
		}
		else {
			char buf[4096] = {};
			snprintf(buf, 4096, "Discarded: %s\n", line);
			OutputDebugString(buf);
		}

		line = strtok_s(NULL, "\n", &cursor_tracker);
	} while (line);

	return result;
}
