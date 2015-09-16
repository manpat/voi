#include "entitymanager.h"
#include "entity.h"
#include "pool.h"

EntityManager::EntityManager() {
	// 1MB per framebuffer, 1MB swap
	Entity::messagePool = new FramePool{1u<<20};
}

EntityManager::~EntityManager(){
	delete Entity::messagePool;
	Entity::messagePool = nullptr;
}

void EntityManager::Update(){
	Entity::messagePool->Update();

	for(auto e: entities){
		// Don't bother checking active because it's guaranteed that
		//	all entities will be.
		// Pooling not implemented
		if(e->enabled /*&& e->active*/){
			e->Update();
		}
	}
}