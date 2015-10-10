#include "entitymanager.h"
#include "component.h"
#include "entity.h"
#include "pool.h"

#include <algorithm>

template<>
EntityManager* Singleton<EntityManager>::instance = nullptr;

EntityManager::EntityManager(): entityIdCounter{0} {
	// 1MB per framebuffer, 1MB swap
	Entity::messagePool = new FramePool{1u<<20};
	instance = this;
}

EntityManager::~EntityManager(){
	DestroyAllEntities();

	// We don't need checks here because deleting a nullptr
	//	is a nop
	delete Entity::messagePool;
	Entity::messagePool = nullptr;
	instance = nullptr;
}

Entity* EntityManager::CreateEntity(){
	auto e = new Entity{};
	e->id = ++entityIdCounter; // Smallest valid id is 1
	e->Init();
	entities.push_back(e);
	return e;
}

// TODO: Verify that this does what I think it does
void EntityManager::DestroyEntity(Entity* e){
	if(!e) return;

	auto end = entities.end();
	entities.erase(std::remove(entities.begin(), end, e), end);

	// Destroy self and all children
	e->DestroyRecurse();
	delete e;
}

Entity* EntityManager::FindEntity(const std::string& name){
	auto entit = std::find_if(entities.begin(), entities.end(), [&name](const Entity* e){
		return name == e->GetName();
	});

	if(entit == entities.end()) return nullptr;
	return *entit;
}

void EntityManager::Update(){
	Entity::messagePool->Update();

	while (!newComponents.empty()) {
		auto c = newComponents.front();
		if(c) {
			c->OnAwake();
		}
		newComponents.pop();
	}

	for(auto it = entities.begin(); it != entities.end(); ++it){
		auto e = *it;

		// Don't bother checking active because it's guaranteed that
		//	all entities will be.
		// Pooling not implemented
		if(e->enabled /*&& e->id != 0*/){
			e->Update();
		}
	}
}

void EntityManager::DestroyAllEntities(){
	for(auto it = entities.begin(); it != entities.end(); ++it){
		auto e = *it;
		if(!e) continue;

		// Don't try to destroy children
		e->Destroy();
		delete e;
	}

	entities.clear();
	// For some reason, std::queue doesn't have a clear
	//	This is equivalent
	newComponents = std::queue<Component*>{};

	entityIdCounter = 0;
	Component::componentIdCounter = 0;
}