#include "entity.h"
#include "component.h"
#include "pool.h"

#include <algorithm>

FramePool* Entity::messagePool = nullptr;

void Entity::Init(){
	
}

void Entity::Destroy(){

}

void Entity::Update(){

}

void Entity::AddComponent(Component* c){
	if(!c) return;

	components.push_back(c);
	c->OnAwake();
}

void Entity::RemoveComponent(Component* c){
	if(!c) return;

	auto it = std::find(components.begin(), components.end(), c);
	if(it == components.end()) return;

	// TODO: TEST THIS!!
	c->OnDestroy();

	*it = components.back();
	components.pop_back();
	delete c;
}
