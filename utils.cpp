#include <windows.h>
#include <assert.h>

#include "utils.h"

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

	auto result = (char *)malloc(file_size_32);
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
