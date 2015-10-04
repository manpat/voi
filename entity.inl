#include "opaquetype.h"
#include "component.h"
#include "pool.h"

template<class C, class... A>
C* Entity::AddComponent(A... args) {
	auto c = new C{args...};
	AddComponent(c);

	return c;
}

template<class C>
C* Entity::FindComponent(){
	auto it = std::find_if(components.begin(), components.end(), [](const Component* c){
		return c->IsType<C>();	
	});

	if(it == components.end()) return nullptr;

	return static_cast<C*>(*it);
}

template<size_t C, class F, class... T>
struct ArgumentPackImpl {
	using type = std::tuple<F, T...>;
};
template<class F, class... T>
struct ArgumentPackImpl<1, F, T...> {
	using type = F;
};

template<class F, class... T>
struct CountPack {
	enum{ count =  CountPack<T...>::count+1 };
};
template<class F>
struct CountPack<F> {
	enum{ count = 1 };
};

template<class... T>
using ArgumentPack = typename ArgumentPackImpl<CountPack<T...>::count, T...>::type;

template<class... A>
void Entity::SendMessage(const std::string& type, A... args){
	OpaqueType ot;

	// Packet valid till next frame
	auto packet = messagePool->New<ArgumentPack<A...>>(args...);
	ot.Set(packet);

	for(auto c: components){
		c->OnMessage(type, ot);
	}
}
// The no-argument definition in source entity.cpp

template<class... A>
void Entity::SendMessageRecurse(const std::string& type, A... args){
	// This could be optimised by preallocating the packet and passing it down

	SendMessage(type, args...);
	for(auto e: children){
		e->SendMessageRecurse(type, args...);
	}
}