#ifndef GENERICTRIGGER_H
#define GENERICTRIGGER_H

#include "component.h"

struct Entity;

struct GenericTrigger : Component {
	// If target is specified, 'action' is sent to that, 
	//	otherwise it is sent to the owning entity
	Entity* target;

	// If this is set, target is found on Awake
	std::string targetName;

	// Sent as a message to either 'target' or the
	//	owning entity
	std::string action; 

	GenericTrigger(Entity* t, const std::string& a = "trigger") 
		: Component(this), target(t), targetName(), action(a) {}

	GenericTrigger(const std::string& tn, const std::string& a) 
		: Component(this), target(nullptr), targetName(tn), action(a) {}
		
	GenericTrigger(const std::string& a = "trigger") 
		: Component(this), target(nullptr), targetName(), action(a) {}

	void OnAwake() override;
	void OnTriggerEnter(ColliderComponent*) override;
};

#endif