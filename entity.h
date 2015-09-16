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
	std::unordered_map<std::string, std::string> userdata; // TODO: Make better
	u32 id;

	// Whether or not this should recieve updates
	bool enabled;
	// Whether or not this is in use (flag to aid pooling)
	bool active;

	// TODO: Add reference to ogre entity somewhere here
	// TODO: Add methods for manipulating ogre entity
	//	entity name, transform

	// Init resets Entity to default state and cleans up previous data.
	// 	To help with pooling
	void Init();

	// Destroy cleans up components and children
	void Destroy();

	// Update updates all attached and active components
	void Update();

	// AddComponent<C,A...> constructs a component with type C with arguments
	//	of types A... or void. Returns new component
	template<class C, class... A>
	C* AddComponent(A...);

	// AddComponent attaches an already constructed component
	void AddComponent(Component*);
	// RemoveComponent removes a component if it is attached
	void RemoveComponent(Component*);

	// FindComponent returns a component of type C or nullptr
	template<class C>
	C* FindComponent();

	// SendMessage broadcasts a message to attached components
	template<class... A>
	void SendMessage(std::string /* ??? */, A...);

	// SendMessageRecurse broadcasts a message to attached components
	//	and child entities
	template<class... A>
	void SendMessageRecurse(std::string /* ??? */, A...);
};

#include "entity.inl"

#endif