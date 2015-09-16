#ifndef ENTITY_H
#define ENTITY_H

#include <vector>
#include <unordered_map>

#include "common.h"

// Collection of Components
// Collection of Children
// Userdata
// Parent
// Id

// Updatable
// Transform operations (modifications to ogre transform)
// Component Queries/Operations
// Message Distribution

struct Component;

struct Entity {
	std::vector<Component*> components;
	std::vector<Entity*> children;
	Entity* parent;
	// std::unordered_map<std::string, Any> userdata;
	u32 id;

	
};

#endif