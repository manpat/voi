#include <typeinfo>

#ifdef __GNUC__
// This is for fancy type demangling in GCC
#	include <cxxabi.h>

template<class T>
std::string getTypeName() {
	static char nameBuff[100];
	static s32 status = 0;
	size_t buffLength = 100;	

	return std::string{abi::__cxa_demangle(typeid(T).name(), nameBuff, &buffLength, &status)};
}

#else

template<class T>
std::string getTypeName() {
	return std::string{typeid(T).name()};
}

#endif

template<class T>
OpaqueType::OpaqueType(T* d){
	if(!d){
		name = "nullptr";
		hash = 0;
		data = nullptr;
		return;
	}
	
	Set(d);
}

template<class T>
bool OpaqueType::CheckType(){
	return typeid(T).hash_code() == hash;
}

template<class T>
void OpaqueType::Set(T* d){
	name = getTypeName<T>();
	hash = typeid(T).hash_code();

	data = static_cast<void*>(d);
}

template<class T>
T* OpaqueType::Get(bool fatal) const{
	if(typeid(T).hash_code() != hash){
		if(fatal){
			throw std::string("OpaqueType type mismatch: Get<") + getTypeName<T>() + "> {" + name + "}";
		}

		return nullptr;
	}

	return static_cast<T*>(data);
}