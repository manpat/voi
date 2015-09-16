#include "entity.h"
#include "component.h"
#include "pool.h"

#include <algorithm>

FramePool* Entity::messagePool = nullptr;

void Entity::Init(){
	throw "Not implemented";
}

void Entity::Destroy(){
	throw "Not implemented";
}

void Entity::Update(){
	throw "Not implemented";
}

void Entity::AddComponent(Component* c){
	if(!c) return;

	components.push_back(c);
	c->entity = this;
	c->enabled = true;
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
