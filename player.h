#ifndef PLAYER_H
#define PLAYER_H

#include "component.h"

struct Portal;
struct Movable;
struct PortalTrigger;

struct Player : Component {
	f32 cameraYaw = 0.f;
	f32 cameraPitch = 0.f;
	Movable* heldObject = nullptr;
	PortalTrigger* portalTrigger = nullptr;
	bool isNearGround = false;
	bool isGrounded = true;
	bool isJumping = false;
	vec3 localCameraPosition = vec3::ZERO;
	vec3 cameraOffset = vec3::ZERO;
	f32 maxShake = 0.0f;
	f32 bobDelta = 0.0f;
	f32 bobPower = 0.0f;

	Player() : Component{this} {}

	void OnInit() override;
	void OnAwake() override;
	void OnUpdate() override;
	void OnLayerChange() override;
	void OnCollisionEnter(ColliderComponent*) override;

	void Respawn();
	void Respawn(vec3 pos, s32 layer);
	void SetToOrientation(const quat&);
	void ShakeCamera(f32);
};

#endif