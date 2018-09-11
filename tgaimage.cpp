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

	assert(result.image.header->image_type == 10);
	assert(result.image.header->color_map_type == 0);

	return result;
}

Color get_next_pixel(TgaImagePixel *pixel_data) {
	bool need_pixel = false;
	if (pixel_data->num_pixels_in_run == 0) {
		auto pixel_header = *pixel_data->next_packet++;

		pixel_data->raw_packet = (pixel_header & 0x80) == 0;
		pixel_data->num_pixels_in_run = (pixel_header & 0x7F) + 1; // EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE "7 bit repetition count minus 1." WHY? WHY? WHY?

		need_pixel = true;
	}

	if (pixel_data->raw_packet || need_pixel) {
		auto b = *pixel_data->next_packet++;
		auto g = *pixel_data->next_packet++;
		auto r = *pixel_data->next_packet++;

		pixel_data->current_pixel_color = Color{ r, g, b, 255 };
	}

	pixel_data->num_pixels_in_run--;

	return pixel_data->current_pixel_color;
}