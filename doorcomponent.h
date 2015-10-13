#ifndef DOORCOMPONENT_H
#define DOORCOMPONENT_H

#include "component.h"

struct ColliderComponent;
struct MoverComponent;

// TODO: A way to trigger events when combination is (in)correct

struct DoorComponent : Component {
	MoverComponent* mover = nullptr;
	vec3 closedPosition = vec3::ZERO;
	vec3 openPosition = vec3::ZERO;
	u32 requiredMask = 1;
	u32 switchStates = 0;
	bool isOpen = false;
	bool ordered = false;

	DoorComponent(u32 rc = 1, bool o = false) : Component{this}, requiredMask((1u<<rc) - 1), ordered(o) {}

	void OnAwake() override;
	void OnMessage(const std::string&, const OpaqueType&) override;

private:
	void UpdateState();

};

#endif