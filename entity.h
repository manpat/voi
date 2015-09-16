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

	// TODO: Add reference to ogre entity somewhere here

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
	void SendMessage(std::string /* temp */, A...);

	// SendMessageRecurse broadcasts a message to attached components
	//	and child entities
	template<class... A>
	void SendMessageRecurse(std::string /* temp */, A...);
};

#endif