#ifndef COMPONENT_H
#define COMPONENT_H

#include "common.h"

// Is an interface
// Updatable
// Can recieve messages

// Note: Cannot be pooled easily because it relies on polymorphism

struct Component {
	u32 id;

	// TODO: figure out exactly what needs to be passed
	virtual void OnUpdate(f32 dt) = 0;
};

#endif