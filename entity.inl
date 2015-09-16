#include "opaquetype.h"

template<class C, class... A>
C* Entity::AddComponent(A...) {
	return nullptr;
}

template<class C>
C* Entity::FindComponent(){
	return nullptr;
}

template<class... A>
void Entity::SendMessage(std::string type, A... args){

}

template<class... A>
void Entity::SendMessageRecurse(std::string type, A... args){

}