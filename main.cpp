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

// At the moment, I don't really understand this math.
// Maybe this is good enough reading? http://www.thecodecrate.com/opengl-es/opengl-viewport-matrix/
Mat4f make_viewport(int x, int y, int width, int height) {
	const int depth = 255;
	Mat4f result = Mat4_Identity;

	set_matrix_entry(result, 0, 3, x + width * 0.5f);
	set_matrix_entry(result, 1, 3, y + height * 0.5f);
	set_matrix_entry(result, 2, 3, depth * 0.5f);

	set_matrix_entry(result, 0, 0, width * 0.5f);
	set_matrix_entry(result, 1, 1, height * 0.5f);
	set_matrix_entry(result, 2, 2, depth * 0.5f);

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

	auto client_width = 800;
	auto client_height = 800;
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

	auto camera = Vec3f{ 0, 0, 3 };
	auto view = make_viewport((int)(client_width * 0.125f), (int)(client_width * 0.125f), (int)(client_width * 0.75f), (int)(client_height * 0.75f));
	auto proj = Mat4_Identity;
	set_matrix_entry(proj, 3, 2, -1.0f / camera.z);

	auto light_dir = Vec3f{ 0, 0, -1 };
	auto z_buffer_length = client_height * client_width;

	// I have a feeling there's something wrong with how I'm handling the z-buffer that might become apparent when I move the camera behind the model.
	auto z_buffer = (f32 *)malloc(z_buffer_length * sizeof(f32));

	auto image_load_result = load_tga_image("data/african_head_diffuse.tga");

	if (!image_load_result.loaded) return -1;

	assert(image_load_result.loaded);

	auto image = image_load_result.image;
	auto texture_map = decompress_tga_image(&image);

	timeBeginPeriod(1);

	auto last_time = timeGetTime();
	while (GlobalRunning) {
		MSG message;
		while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
			handle_message(window, buffer, message);
		}

		// At the moment, it's taking about a half second to render each frame.
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

		memset(z_buffer, -1, z_buffer_length);

		for (auto index = 0; index < sb_count(obj.faces); ++index) {
			auto face = &obj.faces[index];

			Vec3f vertices[] = {
				obj.verts[face->vertex_indices.x].v3,
				obj.verts[face->vertex_indices.y].v3,
				obj.verts[face->vertex_indices.z].v3
			};

			Vec3f transformed_verts[] = 
			{
				project_to_vec3f(view * proj * vertices[0]),
				project_to_vec3f(view * proj * vertices[1]),
				project_to_vec3f(view * proj * vertices[2]),
			};

			Vec2f uvs[3] =
			{
				obj.text_coords[face->texture_indices.x].v2,
				obj.text_coords[face->texture_indices.y].v2,
				obj.text_coords[face->texture_indices.z].v2
			};

			Triangle triangle = { transformed_verts[0], transformed_verts[1], transformed_verts[2] };

			auto normal = (vertices[2] - vertices[0]).cross(vertices[1] - vertices[0]);
			auto transformed_normal = (transformed_verts[2] - transformed_verts[0]).cross(transformed_verts[1] - transformed_verts[0]);
			auto light_intensity = normalize(normal).dot(light_dir);

			if (light_intensity > 0) draw_triangle(buffer, triangle, texture_map, uvs, z_buffer, light_intensity);
		}

		auto context = GetDC(window);
		render(buffer, context);
		ReleaseDC(window, context);
	}

	return 0;
}
