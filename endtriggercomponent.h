#ifndef ENDTRIGGERCOMPONENT_H
#define ENDTRIGGERCOMPONENT_H

#include "common.h"
#include "component.h"

struct EndTriggerComponent : Component {
	bool hasTriggered = false;

	EndTriggerComponent() : Component(this) {}

	void OnTriggerEnter(ColliderComponent*) override;
};

#endif
