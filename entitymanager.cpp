#include "entitymanager.h"
#include "entity.h"
#include "pool.h"

EntityManager::EntityManager(): entityIdCounter{0} {
	// 1MB per framebuffer, 1MB swap
	Entity::messagePool = new FramePool{1u<<20};
}

EntityManager::~EntityManager(){
	delete Entity::messagePool;
	Entity::messagePool = nullptr;

	for(auto e: entities){
		e->Destroy();
		delete e;
	}
}

Entity* EntityManager::CreateEntity(){
	auto e = new Entity{};
	e->Init();
	e->id = ++entityIdCounter; // Smallest valid id is 1
	entities.push_back(e);
	return e;
}

void EntityManager::DestroyEntity(Entity*){
	throw "Not Implemented";
}

Entity* EntityManager::FindEntity(const std::string& name){
	throw "Not Implemented";
}

void EntityManager::Update(){
	Entity::messagePool->Update();

	for(auto e: entities){
		// Don't bother checking active because it's guaranteed that
		//	all entities will be.
		// Pooling not implemented
		if(e->enabled /*&& e->id != 0*/){
			e->Update();
		}
	}
}