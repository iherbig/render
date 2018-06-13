#include <windows.h>
#include <assert.h>

#include "tgaimage.h"
#include "utils.h"

TgaImageLoadResult load_tga_image(const char *file_name) {
	TgaImageLoadResult result;

	auto file_contents = read_entire_file(file_name);

	if (!file_contents.read) {
		result.loaded = false;
		OutputDebugString("Could not read file.");
		return result;
	}

	assert(file_contents.result_size > 18);

	result.loaded = true;

	auto contents = file_contents.result;
	result.file_contents = (u8 *)contents;
	result.image.header = (TgaImageHeader *)contents;

	contents += 18;

	result.image.pixel_data = (u8 *)contents;

	return result;
}