#include <typeinfo>

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
void OpaqueType::Set(T* d){
	name = typeid(T).name();
	hash = typeid(T).hash_code();

	data = static_cast<void*>(d);
}

template<class T>
T* OpaqueType::Get(){
	if(typeid(T).hash_code() != hash) 
		throw std::string("OpaqueType type mismatch: Get<") + typeid(T).name() + "> {" + name + "}";

	return static_cast<T*>(data);
}