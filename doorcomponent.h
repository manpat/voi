#ifndef DOORCOMPONENT_H
#define DOORCOMPONENT_H

#include "component.h"

struct ColliderComponent;
struct MoverComponent;

// TODO: Logic for 'and' and 'sequence' locks
// TODO: blender panel for door configuration

struct DoorComponent : Component {
	MoverComponent* mover = nullptr;
	ColliderComponent* collider = nullptr;

	vec3 closedPosition = vec3::ZERO;
	vec3 openPosition = vec3::ZERO;
	bool isOpen = false;

	DoorComponent() : Component{this} {}

	void OnAwake() override;
	void OnMessage(const std::string&, const OpaqueType&) override;
};

#endif