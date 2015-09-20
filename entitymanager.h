#ifndef ENTITYMANAGER_H
#define ENTITYMANAGER_H

#include <vector>
#include <queue>

#include "common.h"
#include "singleton.h"

// Handles updating
// Entity queries

// List/Pool of entities

struct Entity;
struct Component;

struct EntityManager : Singleton<EntityManager> {
	std::vector<Entity*> entities;
	std::vector<Component*> newComponents;
	u32 entityIdCounter;

	EntityManager();
	~EntityManager();

	Entity* CreateEntity();
	void DestroyEntity(Entity*);
	Entity* FindEntity(const std::string& name);

	// Update updates all active entites
	void Update();
};

#endif