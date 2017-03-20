#pragma once
#include <algorithm>

template<class T, class F>
void RemoveFromVectorIf(std::vector<T>* v, F&& f) {
	auto end = v->end();
	auto it = std::remove_if(v->begin(), end, std::forward<F>(f));
	v->erase(it, end);
}

template<class T, u64 size>
constexpr u64 GetArraySize(const T (&)[size]) {
	return size;
}