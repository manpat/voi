#ifndef PLAYER_H
#define PLAYER_H

#include "component.h"

struct ColliderComponent;

struct Player : Component {
	ColliderComponent* collider;
	f32 cameraYaw = 0.f;
	f32 cameraPitch = 0.f;

	Player() : Component{this} {}

	void OnAwake() override;
	void OnUpdate() override;
};

#endif