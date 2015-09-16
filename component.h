#ifndef COMPONENT_H
#define COMPONENT_H

#include <string>

#include "common.h"
#include "opaquetype.h"

// Is an interface
// Updatable
// Can recieve messages

// Note: Cannot be pooled easily because it relies on polymorphism

struct Entity;

struct Component {
	// This is the owning entity
	Entity* entity;
	// Unique identifier
	u32 id;
	// This determines whether OnUpdate is triggered
	bool enabled;


	// OnAwake is called after the component has been initialised and attached to 
	//	an entity
	virtual void OnAwake() {};
	
	// OnDestroy is called after the owning entity calls RemoveComponent or is destroyed
	virtual void OnDestroy() {};

	// OnUpdate is called once per frame if the component is enabled
	// TODO: figure out exactly what needs to be passed to OnUpdate, maybe nothing
	virtual void OnUpdate(/*f32 dt*/) {};

	// OnMessage is called when SendMessage is called on the owning entity 
	virtual void OnMessage(std::string, const OpaqueType&) {};
};

#endif