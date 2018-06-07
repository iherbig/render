#pragma once

#include "types.h"

struct FileReadResult {
	char *result;
	u32 result_size;
	bool read;
};

FileReadResult read_entire_file(const char *file_name);