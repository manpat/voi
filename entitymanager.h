#ifndef ENTITYMANAGER_H
#define ENTITYMANAGER_H

#include <vector>

#include "common.h"

// Handles updating
// Entity queries

// List/Pool of entities

struct Entity;

struct EntityManager {
	std::vector<Entity*> entities;

	EntityManager();
	~EntityManager();

	// Update updates all active entites
	void Update();
};

#endif