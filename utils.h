#pragma once

#include <memory>

#include "types.h"

struct FileReadResult {
	char *result;
	u32 result_size;
	bool read;
};

FileReadResult read_entire_file(const char *file_name);

// ===============================================================
// Taken from: https://gist.github.com/p2004a/045726d70a490d12ad62
// I'm not really sure what std::forward or the macro magic bits do.
// ===============================================================

template <typename F>
class DeferFinalizer {
	F f;
	bool moved;

public:
	template<typename T>
	DeferFinalizer(T &&f_) : f(std::forward<T>(f_)), moved(false) {}

	DeferFinalizer(const DeferFinalizer &) = delete;

	DeferFinalizer(DeferFinalizer &&other) : f(std::move(other.f)), moved(other.moved) {
		other.moved = true;
	}

	~DeferFinalizer() {
		if (!moved) f();
	}
};

static struct {
	template <typename F>
	DeferFinalizer<F> operator<<(F &&f) {
		return DeferFinalizer<F>(std::forward<F>(f));
	}
} deferrer;

#define TOKENPASTE(x, y) x ## y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)
#define defer auto TOKENPASTE2(__deferred_lambda_call, __COUNTER__) = deferrer << [&]

// ===============================================================
// End taken section
// ===============================================================