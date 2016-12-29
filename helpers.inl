#pragma once
#ifndef HELPERS_INL
#define HELPERS_INL

#include <algorithm>

template<class T, class F>
void RemoveFromVectorIf(std::vector<T>* v, F&& f) {
	auto end = v->end();
	auto it = std::remove_if(v->begin(), end, std::forward<F>(f));
	v->erase(it, end);
}

#endif