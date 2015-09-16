#ifndef OPAQUE_TYPE_H
#define OPAQUE_TYPE_H

#include "common.h"

struct OpaqueType {
	const char* name;
	size_t hash;
	void* data;

	template<class C>
	OpaqueType(C* = nullptr);

	template<class C>
	void Set(C*);

	template<class C>
	C* Get();
};

#include "opaquetype.inl"

#endif