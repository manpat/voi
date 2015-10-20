#ifndef PLAYER_H
#define PLAYER_H

#include "component.h"

struct Portal;
struct PortalTrigger;

struct Player : Component {
	f32 cameraYaw = 0.f;
	f32 cameraPitch = 0.f;
	PortalTrigger* portalTrigger;

	Player() : Component{this} {}

	void OnInit() override;
	void OnAwake() override;
	void OnUpdate() override;
	void OnLayerChange() override;
	
	void Respawn(vec3 pos, s32 layer);
	void SetToOrientation(const quat&);
};

#endif