#include <windows.h>
#include <stdio.h>

#include "types.h"
#include "color.h"
#include "world.h"
#include "array.h"
#include "vectors.h"
#include "wavefront.h"
#include "utils.h"

static bool GlobalRunning = true;

static World Global_World;

static LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
	// If I put this in a custom proc, then the WM_DESTROY, WM_CLOSE, and WM_QUIT messages are never sent to that proc.
	// I wonder what's going on there...
	switch (message)
	{
	case WM_DESTROY:
	case WM_CLOSE:
	case WM_QUIT:
	{
		GlobalRunning = false;
	} break;

	case WM_PAINT:
	{
		PAINTSTRUCT paint;
		auto context = BeginPaint(window, &paint);
		Global_World.render(context);
		EndPaint(window, &paint);
	} break;

	default:
	{
		return DefWindowProcA(window, message, w_param, l_param);
	} break;
	}

	return 0;
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

	auto obj = load_obj("data/african_head.obj");

	Global_World.buffer.width = client_width;
	Global_World.buffer.height = client_height;
	Global_World.buffer.bytes_per_pixel = 4;
	Global_World.buffer.stride = client_width * Global_World.buffer.bytes_per_pixel;
	Global_World.buffer.memory = (u8 *)VirtualAlloc(0, client_width * client_height * Global_World.buffer.bytes_per_pixel, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	Global_World.buffer.info.bmiHeader.biSize = sizeof(Global_World.buffer.info.bmiHeader);
	Global_World.buffer.info.bmiHeader.biWidth = client_width;
	Global_World.buffer.info.bmiHeader.biHeight = client_height;
	Global_World.buffer.info.bmiHeader.biPlanes = 1;
	Global_World.buffer.info.bmiHeader.biBitCount = 32;
	Global_World.buffer.info.bmiHeader.biCompression = BI_RGB;

	auto light_dir = Vector3<f32>{ 0, 0, -1 };

	while (GlobalRunning) {
		MSG message;
		while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&message);
			DispatchMessageA(&message);
		}

		Global_World.clear(BLACK);

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
				Global_World.draw_triangle(triangle, Color{ grey, grey, grey, 255 });
			}
		}

		auto context = GetDC(window);
		Global_World.render(context);
		ReleaseDC(window, context);
	}

	return 0;
}
