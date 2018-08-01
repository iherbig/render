#include <windows.h>
#include <stdio.h>
#include <limits>

#include "types.h"
#include "color.h"
#include "world.h"
#include "array.h"
#include "vectors.h"
#include "wavefront.h"
#include "utils.h"
#include "tgaimage.h"

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

static void handle_message(HWND window, World *world, MSG message) {
	switch (message.message) {
	case WM_QUIT:
		GlobalRunning = false;
		break;

	case WM_PAINT:
	{
		PAINTSTRUCT paint;
		auto context = BeginPaint(window, &paint);
		world->render(context);
		EndPaint(window, &paint);
	} break;

	default:
		TranslateMessage(&message);
		DispatchMessage(&message);
		break;
	}
}

inline Vector2<int> to_screen_coords(int client_width, int client_height, Vector2<f32> point) {
	Vector2<int> result;
	result.x = (int)((point.x + 1) * client_width / 2.0);
	result.y = (int)((point.y + 1) * client_height / 2.0);
	return result;
}

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR command_line, int show_code) {
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
	auto window_width = client_width;
	auto window_height = client_height;

	RECT window_rect = {};
	window_rect.bottom = client_height;
	window_rect.right = client_width;

	if (AdjustWindowRect(&window_rect, WS_OVERLAPPEDWINDOW, false)) {
		window_width = window_rect.right - window_rect.left;
		window_height = window_rect.bottom - window_rect.top;
	}

	auto window = CreateWindowEx(0, window_class.lpszClassName, "Renderer", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, window_width, window_height, 0, 0, instance, 0);

	if (!window) {
		OutputDebugString("Bad window creation.\n");
		return -2;
	}

	World world = {};

	world.buffer.width = client_width;
	world.buffer.height = client_height;
	world.buffer.bytes_per_pixel = 4;
	world.buffer.stride = client_width * world.buffer.bytes_per_pixel;
	world.buffer.memory = (u8 *)VirtualAlloc(0, client_width * client_height * world.buffer.bytes_per_pixel, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	world.buffer.info.bmiHeader.biSize = sizeof(world.buffer.info.bmiHeader);
	world.buffer.info.bmiHeader.biWidth = client_width;
	world.buffer.info.bmiHeader.biHeight = client_height;
	world.buffer.info.bmiHeader.biPlanes = 1;
	world.buffer.info.bmiHeader.biBitCount = 32;
	world.buffer.info.bmiHeader.biCompression = BI_RGB;

	auto obj = load_obj("data/african_head.wfo");

	auto light_dir = Vector3<f32>{ 0, 0, -1 };

	auto z_buffer_length = client_height * client_width;

	// Why bother free it, it'll live throughout the lifetime of the program.
	auto z_buffer = (f32 *)VirtualAlloc(0, z_buffer_length * sizeof(f32), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	auto texture_load_result = load_tga_image("data/african_head_diffuse.tga");

	if (!texture_load_result.loaded) {
		return -1;
	}

	assert(texture_load_result.loaded);

	auto texture = texture_load_result.image;

	while (GlobalRunning) {
		MSG message;
		while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
			handle_message(window, &world, message);
		}

		world.clear(BLACK);

		for (auto index = 0; index < z_buffer_length; ++index) {
			z_buffer[index] = -1;
		}

		for (auto index = 0; index < obj.faces.count; ++index) {
			auto &face = obj.faces[index];
			Vector3<f32> vertices[] =
			{
				obj.verts[face.vertex_indices.x].v3,
				obj.verts[face.vertex_indices.y].v3,
				obj.verts[face.vertex_indices.z].v3
			};

			Triangle<f32> triangle = { vertices[0], vertices[1], vertices[2] };

			auto normal = (vertices[2] - vertices[0]).cross(vertices[1] - vertices[0]);

			auto intensity = normal.normalize().dot(light_dir);
			if (intensity > 0) {
				// The colors I get are off. They're brighter than the example. Not sure why.
				// I know that we're not actually handling brightness properly here (128 is not half as bright as 255),
				// but then I would expect my image to be dark (as the reference image is) not very bright (as mine is).
				auto grey = (u8)(intensity * 255);
				world.draw_triangle(triangle, z_buffer, Color{ grey, grey, grey, 255 });
			}
		}

		auto context = GetDC(window);
		world.render(context);
		ReleaseDC(window, context);
	}

	return 0;
}
