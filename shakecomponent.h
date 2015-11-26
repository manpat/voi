#ifndef SHAKECOMPONENT_H
#define SHAKECOMPONENT_H

#include "component.h"
#include "common.h"

struct Player;

struct ShakeComponent : Component {
	ShakeComponent() : Component(this) {}

	void OnInit() override;
	void OnUpdate() override;

	void SetEnabled(bool enabled) { isShaking = enabled; }
	bool GetEnabled() { return isShaking; }

	Player* player = nullptr;
	bool isShaking = false;
};

#endif // SHAKECOMPONENT_H