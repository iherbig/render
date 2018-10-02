#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <limits>

#include "stretchy_buffer.h"
#include "types.h"
#include "color.h"
#include "render.h"
#include "vectors.h"
#include "wavefront.h"
#include "utils.h"
#include "tgaimage.h"
#include "texture.h"
#include "matrix_math.h"

static bool GlobalRunning = true;

static LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
	// If I put this in a custom proc, then the WM_DESTROY, WM_CLOSE, and WM_QUIT messages are never sent to that proc.
	// I wonder what's going on there...
	switch (message)
	{
	case WM_CLOSE:
	{
		GlobalRunning = false;
	} break;

	default:
	{
		return DefWindowProcA(window, message, w_param, l_param);
	} break;
	}

	return 0;
}

static void handle_message(HWND window, Backbuffer &buffer, MSG message) {
	switch (message.message) {
	case WM_QUIT:
		GlobalRunning = false;
		break;

	case WM_PAINT:
	{
		PAINTSTRUCT paint;
		auto context = BeginPaint(window, &paint);
		render(buffer, context);
		EndPaint(window, &paint);
	} break;

	default:
		TranslateMessage(&message);
		DispatchMessage(&message);
		break;
	}
}

Mat4f make_viewport(int x, int y, int width, int height) {
	const int near_clip = 1;
	const int far_clip = 255;
	const int depth = far_clip - near_clip;

	Mat4f result = Mat4_Identity;

	set_matrix_entry(result, 0, 0, width * 0.5f);
	set_matrix_entry(result, 0, 3, x + width * 0.5f);

	set_matrix_entry(result, 1, 1, height * 0.5f);
	set_matrix_entry(result, 1, 3, y + height * 0.5f);

	set_matrix_entry(result, 2, 2, depth * 0.5f);
	set_matrix_entry(result, 2, 3, depth * 0.5f);

	return result;
}

inline Vec2i get_window_dimensions(int client_width, int client_height) {
	Vec2i result = { client_width, client_height };

	RECT window_rect = {};
	window_rect.bottom = client_height;
	window_rect.right = client_width;

	if (AdjustWindowRect(&window_rect, WS_OVERLAPPEDWINDOW, false)) {
		result.x = window_rect.right - window_rect.left;
		result.y = window_rect.bottom - window_rect.top;
	}

	return result;
}

inline Mat4f look_at(Vec3f eye, Vec3f center, Vec3f up) {
	auto z = normalize(eye - center);
	auto x = normalize(up.cross(z));
	auto y = normalize(z.cross(x));

	auto view = Mat4_Identity;
	auto model = Mat4_Identity;

	for (auto index = 0; index < 3; ++index) {
		set_matrix_entry(view, 0, index, x.dim[index]);
		set_matrix_entry(view, 1, index, y.dim[index]);
		set_matrix_entry(view, 2, index, z.dim[index]);
		set_matrix_entry(model, index, 3, -center.dim[index]);
	}

	return view * model;
}

//int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR command_line, int show_code) {
int main() {
	auto instance = GetModuleHandle(NULL);
	WNDCLASSEX window_class = {};

	window_class.cbSize = sizeof(WNDCLASSEX);
	window_class.style = CS_HREDRAW | CS_VREDRAW;
	window_class.lpfnWndProc = window_proc;
	window_class.hInstance = instance;
	window_class.lpszClassName = "LearnRenderWindowClass";

	if (!RegisterClassEx(&window_class)) {
		OutputDebugString("Bad register.\n");
		return -1;
	}

	auto client_width = 1024;
	auto client_height = 1024;
	auto window_dimensions = get_window_dimensions(client_width, client_height);

	auto window = CreateWindowEx(0, window_class.lpszClassName, "Renderer", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, window_dimensions.x, window_dimensions.y, 0, 0, instance, 0);

	if (!window) {
		OutputDebugString("Bad window creation.\n");
		return -2;
	}

	Backbuffer buffer = {};
	buffer.width = client_width;
	buffer.height = client_height;
	buffer.bytes_per_pixel = 4;
	buffer.stride = client_width * buffer.bytes_per_pixel;
	buffer.memory = (u8 *)malloc(client_width * client_height * buffer.bytes_per_pixel);

	buffer.info.bmiHeader.biSize = sizeof(buffer.info.bmiHeader);
	buffer.info.bmiHeader.biWidth = client_width;
	buffer.info.bmiHeader.biHeight = client_height;
	buffer.info.bmiHeader.biPlanes = 1;
	buffer.info.bmiHeader.biBitCount = 32;
	buffer.info.bmiHeader.biCompression = BI_RGB;

	auto obj = load_obj("data/african_head.wfo");
	auto image_load_result = load_tga_image("data/african_head_diffuse.tga");
	if (!image_load_result.loaded) return -1;
	assert(image_load_result.loaded);

	auto image = image_load_result.image;
	auto texture_map = decompress_tga_image(&image);

	auto camera = Vec3f{ 1, 1, 3 };
	
	// Something is still not right here. I'm pretty sure the math for the viewport is correct, and it makes sense to me,
	// but moving the camera farther into positive z stops having an effect very quickly, and moving it into the negative z warps it.
	// I have a feeling there's something wrong with how I'm handling the z-buffer that might become apparent when I move the camera behind the model.
	// But it's possible that it's something here.
	//auto viewport = make_viewport((int)(client_width * 0.125f), (int)(client_width * 0.125f), (int)(client_width * 0.75f), (int)(client_height * 0.75f));
	auto viewport = make_viewport(0, 0, client_width, client_height);
	auto proj = Mat4_Identity;
	set_matrix_entry(proj, 3, 2, -1.0f / camera.z);
	auto model_view = look_at(camera, Vec3f{ 0, 0, 0 }, Vec3f{ 0, 1, 0 });

	auto light_dir = normalize(Vec3f{ 1, -1, 1 });
	auto z_buffer_length = client_height * client_width;
	auto z_buffer = (f32 *)malloc(z_buffer_length * sizeof(f32));

	timeBeginPeriod(1);

	auto last_time = timeGetTime();
	while (GlobalRunning) {
		MSG message;
		while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
			handle_message(window, buffer, message);
		}

		// At the moment, it's taking about a second to render each frame.
		// That's crazy long. I might try to parallelize the draw_triangle call as much
		// as possible at some point, but I don't want to get stuck figuring out how
		// to use whatever threading API that Windows has.
		auto current_time = timeGetTime();
		auto delta_t = (current_time - last_time);
		if (delta_t != 0) {
			printf("FPS %ld\n", 1000 / delta_t);
		}

		last_time = current_time;

		clear(buffer, BLACK);

		for (auto index = 0; index < z_buffer_length; ++index) {
			z_buffer[index] = FLT_MIN;
		}

		for (auto index = 0; index < sb_count(obj.faces); ++index) {
			auto face = &obj.faces[index];

			Vec3f vertices[] = {
				obj.verts[face->vertex_indices.x].v3,
				obj.verts[face->vertex_indices.y].v3,
				obj.verts[face->vertex_indices.z].v3
			};

			Vec3f transformed_verts[] = 
			{
				project_to_vec3f(viewport * proj * model_view * vertices[0]),
				project_to_vec3f(viewport * proj * model_view * vertices[1]),
				project_to_vec3f(viewport * proj * model_view * vertices[2]),
			};

			Vec3f normals[] = {
				obj.vert_normals[face->normal_indices.x],
				obj.vert_normals[face->normal_indices.y],
				obj.vert_normals[face->normal_indices.z],
			};
/*
			auto clip_coordinates = proj * vertices[0];
			auto screen_coordinates = viewport * project_to_vec3f(clip_coordinates);
			auto normalized_screen_coordinates = project_to_vec3f(screen_coordinates);
*/
			Vec2f uvs[3] =
			{
				obj.text_coords[face->texture_indices.x].v2,
				obj.text_coords[face->texture_indices.y].v2,
				obj.text_coords[face->texture_indices.z].v2
			};

			Triangle triangle = { transformed_verts[0], transformed_verts[1], transformed_verts[2] };
			draw_triangle(buffer, triangle, texture_map, uvs, z_buffer, normals, light_dir);
		}

		auto context = GetDC(window);
		render(buffer, context);
		ReleaseDC(window, context);
	}

	return 0;
}
