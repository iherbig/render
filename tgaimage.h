#pragma once

#include "types.h"
#include "color.h"

#pragma pack(push, 1)
struct TgaImageHeader {
	u8 id_length;
	u8 color_map_type;
	u8 image_type;

	struct {
		u16 first_entry_index;
		u16 color_map_length;
		u8 color_map_pixel_depth;
	} color_map_spec;

	struct {
		u16 x_origin;
		u16 y_origin;
		u16 image_width;
		u16 image_height;
		u8 pixel_depth;
		u8 image_descriptor;
	} image_spec;
};
#pragma pack(pop)

struct TgaImage {
	TgaImageHeader *header;
	u8 *pixel_packets;
};

// The result of loading the tga image. The file contents are not freed.
// Data are shared between the file contents and the image, so the image
// is unavailable after freeing.
struct TgaImageLoadResult {
	u8 *file_contents;
	TgaImage image;
	bool loaded;
};

struct TgaImagePixelCursor {
	u8 *next_packet;
	Color current_pixel_color;
	u8 num_pixels_in_run;
	bool raw_packet;
};

struct TextureMap;

TgaImageLoadResult load_tga_image(const char *file_name);
Color get_next_pixel(TgaImagePixelCursor *pixel_data);
TextureMap decompress_tga_image(const TgaImage *texture);