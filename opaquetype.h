#ifndef OPAQUE_TYPE_H
#define OPAQUE_TYPE_H

#include "common.h"

// OpaqueType is a safe way to pass around pointers to data of different types
//	It assumes that you know the type of the data when you access it
//	If you try to get the wrong type of data, it will either return nullptr
//		or throw
//
// Note: OpaqueType does not claim ownership of pointers. It is your responsibility
//	to ensure data is valid during the lifetime of OpaqueType and that it is freed
//	when necessary. OpaqueType will *not* free memory
//
// Note: Type information is NOT portable.
// It *may* be used so long as you compare it with typeid(T)
// Do NOT use hardcoded strings or hashes to compare type information

struct OpaqueType {
	std::string name = "nullptr";
	size_t hash = 0;
	void* data = nullptr;

	// Initialises to nullptr
	OpaqueType() = default;

	// See Set
	template<class T>
	OpaqueType(T*);

	// Check for a type match
	template<class T>
	bool CheckType();

	// This will set the type and data
	// Any previous information will be overwritten
	template<class T>
	void Set(T*);

	// If fatal, Get will throw with a type mismatch
	//	else it will return nullptr
	template<class T>
	T* Get(bool fatal = true) const;
};

#include "opaquetype.inl"

#endif