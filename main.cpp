#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <complex>

typedef float f32;
typedef uint8_t u8;
typedef uint32_t u32;

static bool GlobalRunning = true;

struct Color {
	f32 r;
	f32 g;
	f32 b;
};

const Color WHITE = Color{ 1.f, 1.f, 1.f };
const Color RED   = Color{ 1.f,   0,   0 };
const Color BLACK = Color{   0,   0,   0 };

struct Backbuffer {
	Backbuffer() {};

	BITMAPINFO info;

	int width;
	int height;
	int bytesPerPixel;
	int stride;

	u8 *memory;

	void set(int x, int y, const Color &color);
	void draw_line(int x0, int y0, int x1, int y1, const Color &color);
	void render(HDC context);
	void clear(const Color &color);
};

static Backbuffer GlobalBuffer;

void Backbuffer::render(HDC context) {
	StretchDIBits(context, 0, 0, width, height, 0, 0, width, height, memory, &info, DIB_RGB_COLORS, SRCCOPY);
}

void Backbuffer::clear(const Color &color) {
	for (auto row = 0; row < width; ++row) {
		for (auto col = 0; col < height; ++col) {
			auto row_byte_offset = row * bytesPerPixel;
			auto pixel = (u32 *)&(memory[col * stride + row_byte_offset]);
			*pixel = 255 << 24 | (int)(255 * color.r) << 16 | (int)(255 * color.g) << 8 | (int)color.b;
		}
	}
}

void Backbuffer::set(int x, int y, const Color &color) {
	if (x < 0 || y < 0 || x >= width || y >= height) {
		return;
	}

	auto x_byte_offset = x * bytesPerPixel;
	auto pixel = (u32 *)&(memory[(y * stride) + x_byte_offset]);

	// ARGB
	auto red = (int)(255 * color.r);
	auto green = (int)(255 * color.g);
	auto blue = (int)(255 * color.b);
	*pixel = 0xFF << 24 | red << 16 | green << 8 | blue;
}

void Backbuffer::draw_line(int x0, int y0, int x1, int y1, const Color &color) {
	auto steep = false;
	
	if (abs(x0 - x1) < abs(y0 - y1)) {
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;
	}

	if (x0 > x1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
	}

	auto dx = x1 - x0;
	auto dy = y1 - y0;
	auto derr = abs(dy) * 2;
	auto error = 0.0f;

	auto y = y0;

	for (auto x = x0; x <= x1; ++x) {
		if (steep) { 
			set(y, x, color);
		}
		else { 
			set(x, y, color);
		}

		error += derr;
		if (error > dx) {
			y += (y1 > y0 ? 1 : -1);
			error -= dx * 2;
		}
	}
}

template <typename T>
struct Array {
	const int initial_capacity = 5;

	Array() {};

	int count = 0;
	int capacity = initial_capacity;
	T *arr = NULL;

	bool add(T elem);
	bool resize(int new_size);
	T& operator[](int index);
};

template <typename T>
bool Array<T>::resize(int new_size) {
	assert(new_size >= capacity);

	capacity = new_size;
	auto temp = (T*)VirtualAlloc(0, sizeof(T) * capacity, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	if (!temp) {
		return false;
	}

	CopyMemory(temp, arr, count * sizeof(T));
	VirtualFree(arr, 0, MEM_RELEASE);

	arr = temp;

	return arr;
}

template <typename T>
bool Array<T>::add(T elem) {
	if (!arr) {
		if (!resize(initial_capacity)) {
			return false;
		}
	}

	if (count + 1 >= capacity) {
		if (!resize(capacity * 2)) {
			return false;
		}
	}

	arr[count++] = elem;

	return true;
}

template <typename T>
T& Array<T>::operator[](int index) {
	return arr[index];
}

template <typename T>
struct Vector4 {
	T x;
	T y;
	T z;
	T w;
};

template <typename T>
struct Vector3 {
	T x;
	T y;
	T z;
};

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

static LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
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
	}break;

	default:
	{
		return DefWindowProcA(window, message, wParam, lParam);
	} break;
	}

	return 0;
}

struct FileReadResult {
	char *result;
	u32 result_size;
	bool read;
};

FileReadResult read_entire_file(const char *file_name) {
	FileReadResult read_result = {};

	auto handle = CreateFile(file_name, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if (handle == INVALID_HANDLE_VALUE) {
		return read_result;
	}

	LARGE_INTEGER file_size;
	if (!GetFileSizeEx(handle, &file_size)) {
		return read_result;
	}

	assert(file_size.QuadPart <= 0xFFFFFFFF);
	auto file_size_32 = (u32)file_size.QuadPart;

	char *result = (char *)VirtualAlloc(0, file_size_32, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (!result) {
		return read_result;
	}

	read_result.result = result;

	DWORD bytes_read;
	if (!ReadFile(handle, result, file_size_32, &bytes_read, 0) && file_size_32 != bytes_read) {
		VirtualFree(result, 0, MEM_RELEASE);
		read_result.result = NULL;
		return read_result;
	}

	read_result.read = true;
	read_result.result_size = bytes_read;

	return read_result;
}

WavefrontObj load_obj(const char *file_name) {
	WavefrontObj result = {};
	auto file_contents = read_entire_file(file_name);
	auto cursor = file_contents.result;

	if (!file_contents.read) {
		return result;
	}

	char *cursor_tracker;
	char *line_tracker;
	auto line = strtok_s(cursor, "\n", &cursor_tracker);

	do {
		auto token = strtok_s(line, " \t", &line_tracker);

		if (!token) {
			// Just an empty line.
		}
		else if (strcmp(token, "v") == 0) {
			Vector4<f32> vert = {};
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

			result.verts.add(vert);
		}
		else if (strcmp(token, "vn") == 0) {
			Vector3<f32> normal = {};

			token = strtok_s(NULL, " \t", &line_tracker);
			normal.x = (f32)atof(token);
			token = strtok_s(NULL, " \t", &line_tracker);
			normal.y = (f32)atof(token);
			token = strtok_s(NULL, " \t", &line_tracker);
			normal.z = (f32)atof(token);

			result.vert_normals.add(normal);
		}
		else if (strcmp(token, "vt") == 0) {
			Vector3<f32> text = {};

			token = strtok_s(NULL, " \t", &line_tracker);
			text.x = (f32)atof(token);
			token = strtok_s(NULL, " \t", &line_tracker);
			text.y = (f32)atof(token);
			token = strtok_s(NULL, " \t", &line_tracker);

			if (token) {
				text.z = (f32)atof(token);
			}

			result.text_coords.add(text);
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

			result.faces.add(face);
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

	VirtualFree(file_contents.result, 0, MEM_RELEASE);
	return result;
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

	for (auto index = 0; index < obj.faces.count; ++index) {
		auto face = obj.faces[index];
		Vector4<f32> vertices[] = { obj.verts[face.vertex_indices.x],
								    obj.verts[face.vertex_indices.y],
									obj.verts[face.vertex_indices.z] };

		for (auto jj = 0; jj < 3; ++jj) {
			auto x0 = (int)((vertices[jj].x + 1) * width / 2.0);
			auto y0 = (int)((vertices[jj].y + 1) * height / 2.0);

			auto x1 = (int)((vertices[(jj + 1) % 3].x + 1) * width / 2.0);
			auto y1 = (int)((vertices[(jj + 1) % 3].y + 1) * height / 2.0);

			GlobalBuffer.draw_line(x0, y0, x1, y1, WHITE);
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
