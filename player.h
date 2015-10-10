#ifndef PLAYER_H
#define PLAYER_H

#include "component.h"

struct Portal;

struct Player : Component {
	f32 cameraYaw = 0.f;
	f32 cameraPitch = 0.f;

	Player() : Component{this} {}

	void OnAwake() override;
	void OnUpdate() override;
	void OnLayerChange() override;
	void OnTriggerEnter(ColliderComponent*) override;
	void OnTriggerLeave(ColliderComponent*) override;

protected:
	void EnterPortal(Portal*);
	void LeavePortal(Portal*);
};

#endif