#ifndef INTERACTABLE_H
#define INTERACTABLE_H

#include "component.h"

struct Entity;

struct Interactable : Component {
	// If target is specified, 'action' is sent to that, 
	//	otherwise it is sent to the owning entity
	Entity* target;

	// Sent as a message to either 'target' or the
	//	owning entity
	// TODO: this *could* be reduced to an enum once we 
	//	know what we need
	std::string action; 

	Interactable(Entity* t, const std::string& a = "trigger") : Component{this}, target{t}, action{a} {}
	Interactable(const std::string& a = "interact") : Component{this}, target{nullptr}, action{a} {}

	void Activate();
};

#endif