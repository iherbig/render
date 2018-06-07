#include <windows.h>
#include <stdio.h>

#include "types.h"
#include "color.h"
#include "backbuffer.h"
#include "array.h"
#include "vectors.h"
#include "wavefront.h"
#include "utils.h"

static bool GlobalRunning = true;

static Backbuffer GlobalBuffer;

static LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
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
		GlobalBuffer.render(context);
		EndPaint(window, &paint);
	} break;

	default:
	{
		return DefWindowProcA(window, message, wParam, lParam);
	} break;
	}

	return 0;
}

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, int showCode) {
	WNDCLASSEX windowClass = {};

	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = window_proc;
	windowClass.hInstance = instance;
	windowClass.lpszClassName = "LearnRenderWindowClass";

	if (!RegisterClassEx(&windowClass)) {
		OutputDebugString("Bad register.\n");
		return -1;
	}

	auto window = CreateWindowEx(0, windowClass.lpszClassName, "Renderer", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 800, 835, 0, 0, instance, 0);

	if (!window) {
		OutputDebugString("Bad window creation.\n");
		return -2;
	}

	auto obj = load_obj("data/african_head.obj");

	auto width = 800;
	auto height = 800;

	GlobalBuffer.width = width;
	GlobalBuffer.height = height;
	GlobalBuffer.bytesPerPixel = 4;
	GlobalBuffer.stride = width * GlobalBuffer.bytesPerPixel;
	GlobalBuffer.memory = (u8 *)VirtualAlloc(0, width * height * GlobalBuffer.bytesPerPixel, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	GlobalBuffer.info = {};
	GlobalBuffer.info.bmiHeader.biSize = sizeof(GlobalBuffer.info.bmiHeader);
	GlobalBuffer.info.bmiHeader.biWidth = width;
	GlobalBuffer.info.bmiHeader.biHeight = height;
	GlobalBuffer.info.bmiHeader.biPlanes = 1;
	GlobalBuffer.info.bmiHeader.biBitCount = 32;
	GlobalBuffer.info.bmiHeader.biCompression = BI_RGB;

	GlobalBuffer.clear(BLACK);

	auto light_dir = Vector3<f32>{ 0, 0, -1 };

	for (auto index = 0; index < obj.faces.count; ++index) {
		auto &face = obj.faces[index];
		Vector3<f32> vertices[] =
			{ obj.verts[face.vertex_indices.x].v3,
			  obj.verts[face.vertex_indices.y].v3,
			  obj.verts[face.vertex_indices.z].v3 };

		// What's going on here? Why do we want the resultant vector of the two subtractions?
		auto normal = (vertices[2] - vertices[0]).cross(vertices[1] - vertices[0]);

		auto intensity = normalize(normal).dot(light_dir);
		if (intensity > 0) {
			
			// The colors I get are off. They're brighter than the example. Not sure why. My cross product is fine.
			// I think my normalize is fine, and so is my dot product. Perhaps it's the cast here.
			auto grey = (u8)(intensity * 255);
			GlobalBuffer.draw_triangle(
				Vector2<f32>{ (vertices[0].x + 1) * width / 2.0f, (vertices[0].y + 1) * height / 2.0f },
				Vector2<f32>{ (vertices[1].x + 1) * width / 2.0f, (vertices[1].y + 1) * height / 2.0f },
				Vector2<f32>{ (vertices[2].x + 1) * width / 2.0f, (vertices[2].y + 1) * height / 2.0f },
				Color{ grey, grey, grey });
		}
	}

	while (GlobalRunning) {
		MSG message;
		while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&message);
			DispatchMessageA(&message);
		}

	}

	return 0;
}
