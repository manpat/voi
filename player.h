#ifndef PLAYER_H
#define PLAYER_H

#include "component.h"

struct Player : Component {
	f32 cameraYaw = 0.f;
	f32 cameraPitch = 0.f;

	Player() : Component{this} {}

	void OnInit() override;
	void OnUpdate() override;
};

#endif