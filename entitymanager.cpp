#include "entitymanager.h"

EntityManager::EntityManager() : messagePool{1u<<20 /*1MB per framebuffer, 1MB swap*/} {
	
}

EntityManager::~EntityManager(){
	
}

void EntityManager::Update(){
	messagePool.Update();

	// Update all entities
}