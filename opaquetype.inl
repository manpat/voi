#include <typeinfo>
#include "typehelpers.h"

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

template<class... Ts>
ArgumentPack<Ts...>* OpaqueType::Get(bool fatal) const{
	using RetType = ArgumentPack<Ts...>;

	if(typeid(RetType).hash_code() != hash){
		if(fatal){
			throw std::string("OpaqueType type mismatch: Get<") + getTypeName<RetType>() + "> {" + name + "}";
		}

		return nullptr;
	}

	return static_cast<RetType*>(data);
}