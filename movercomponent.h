#ifndef MOVERCOMPONENT_H
#define MOVERCOMPONENT_H

#include "component.h"
#include "common.h"

struct ColliderComponent;

struct MoverComponent : Component {
	ColliderComponent* collider = nullptr;
	vec3 fromPosition = vec3::ZERO;
	vec3 positionDiff = vec3::ZERO;
	f32 a = 0.f;
	f32 animationLength = 0.f;
	bool moving = false;

	MoverComponent() : Component{this} {}

	void OnAwake() override;
	void OnUpdate() override;

	void MoveTo(const vec3& npos, f32 inTime);
};

#endif