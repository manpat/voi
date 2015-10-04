#ifndef INTERACTABLE_H
#define INTERACTABLE_H

#include "component.h"

struct Interactable : Component {
	Interactable() : Component{this} {}

	void Activate();
};

#endif