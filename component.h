#ifndef COMPONENT_H
#define COMPONENT_H

#include <string>
#include <typeinfo>

#include "common.h"
#include "opaquetype.h"

// Is an interface
// Updatable
// Can recieve messages

// Note: Cannot be pooled easily because it relies on polymorphism

struct Entity;

struct Component {
	static u32 componentIdCounter;

	// This is the owning entity
	Entity* entity = nullptr;
	// Unique identifier, id 0 is invalid
	// TODO: Find somewhere to put the id counter
	u32 id = 0;
	// This is for fast type comparisons
	size_t typeHash = 0; 
	// This determines whether OnUpdate is triggered
	bool enabled = true;

	template<class C>
	Component(C* c) : id{++componentIdCounter}, typeHash{typeid(C).hash_code()} {}
	virtual ~Component() {}

	// OnInit is called after the component has been initialised and attached to 
	//	an entity
	virtual void OnInit() {};

	// OnAwake is called before the before the first update after initialisation
	virtual void OnAwake() {};
	
	// OnRemove is called after the owning entity calls RemoveComponent
	virtual void OnRemove() {};
	
	// OnDestroy is called before the component is destroyed
	virtual void OnDestroy() {};

	// OnUpdate is called once per frame if the component is enabled
	virtual void OnUpdate() {};

	// OnMessage is called when SendMessage is called on the owning entity 
	virtual void OnMessage(const std::string&, const OpaqueType&) {};

	template<class C>
	bool IsType() const { return typeid(C).hash_code() == typeHash; }

	template<class C>
	const C* As(bool fatal = true) const {
		if(!IsType<C>()) return nullptr;
		return static_cast<const C*>(this);
	}

	bool SameType(Component*) const;


};

#endif