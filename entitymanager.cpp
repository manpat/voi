#include "entitymanager.h"
#include "component.h"
#include "entity.h"
#include "pool.h"

template<>
EntityManager* Singleton<EntityManager>::instance = nullptr;

EntityManager::EntityManager(): entityIdCounter{0} {
	// 1MB per framebuffer, 1MB swap
	Entity::messagePool = new FramePool{1u<<20};
	instance = this;
}

EntityManager::~EntityManager(){
	// TODO: Fix segfault here
	for(auto e: entities){
		if(!e) continue;

		e->Destroy();
		delete e;
	}

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

	e->Destroy();
	delete e;
}

Entity* EntityManager::FindEntity(const std::string& name){
	(void)name;
	// TODO: Implement
	throw "Not Implemented";
}

void EntityManager::Update(){
	Entity::messagePool->Update();

	newComponents.erase(std::remove(newComponents.begin(), newComponents.end(), nullptr), newComponents.end());
	for(auto c: newComponents){
		c->OnAwake();
	}
	newComponents.clear();

	for(auto e: entities){
		// Don't bother checking active because it's guaranteed that
		//	all entities will be.
		// Pooling not implemented
		if(e->enabled /*&& e->id != 0*/){
			e->Update();
		}
	}
}