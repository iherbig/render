#pragma once

#include <windows.h>
#include <assert.h>

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
