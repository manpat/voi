#ifndef AREATRIGGERCOMPONENT_H
#define AREATRIGGERCOMPONENT_H

#include "component.h"

struct AreaTriggerComponent : Component {
	std::string toLevel;

	AreaTriggerComponent(const std::string& lvl) : Component(this), toLevel(lvl) {}

	void OnTriggerEnter(ColliderComponent*) override;
};

#endif