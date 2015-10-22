#ifndef MOVABLEOBJECT_H
#define MOVABLEOBJECT_H

#include "component.h"

struct Movable : Component {
	Movable() : Component(this) {}

	void NotifyPickup();
	void NotifyDrop();
};

#endif