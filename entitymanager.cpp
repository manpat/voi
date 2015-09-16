#include "entitymanager.h"
#include "entity.h"
#include "pool.h"

EntityManager::EntityManager() {
	// 1MB per framebuffer, 1MB swap
	Entity::messagePool = new FramePool{1u<<20};
}

EntityManager::~EntityManager(){
	
}

void EntityManager::Update(){
	Entity::messagePool->Update();

	// Update all entities
}