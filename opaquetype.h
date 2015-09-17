#ifndef OPAQUE_TYPE_H
#define OPAQUE_TYPE_H

#include "common.h"

struct OpaqueType {
	const char* name = "nullptr";
	size_t hash = 0;
	void* data = nullptr;

	OpaqueType() = default;

	template<class C>
	OpaqueType(C*);

	template<class C>
	void Set(C*);

	template<class C>
	C* Get() const;
};

#include "opaquetype.inl"

#endif