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

	void SetTimer(f32 shakeAmount, f64 time, f64 fadeTime = 0);

	Player* player = nullptr;
	bool isShaking = false;
	f64 timerSeconds = 0;
	f64 fadeSeconds = 0;
	f32 fadeShakeAmount = 0;
};

#endif // SHAKECOMPONENT_H