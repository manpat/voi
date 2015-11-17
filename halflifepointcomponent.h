#ifndef HALFLIFEPOINTCOMPONENT_H
#define HALFLIFEPOINTCOMPONENT_H

#include "component.h"

struct HalfLifePointComponent : Component {
	std::string toLevel;

	HalfLifePointComponent(const std::string& lvl) : Component(this), toLevel(lvl) {}

	void OnUpdate() override;
	void OnTriggerEnter(ColliderComponent*) override;
};

#endif