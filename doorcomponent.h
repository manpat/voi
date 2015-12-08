#ifndef DOORCOMPONENT_H
#define DOORCOMPONENT_H

#include "component.h"

struct ColliderComponent;
struct SynthComponent;
struct MoverComponent;
struct ShakeComponent;

// TODO: A way to trigger events when combination is (in)correct

struct DoorComponent : Component {
	SynthComponent* synth = nullptr;
	MoverComponent* mover = nullptr;
	ShakeComponent* shake = nullptr;
	vec3 closedPosition = vec3::ZERO;
	vec3 openPosition = vec3::ZERO;
	f32 openTime = 2.f;
	u32 requiredMask = 1;
	u32 switchStates = 0;
	bool isOpen = false;
	bool ordered = false;

	DoorComponent(u32 rc = 1, bool o = false, f32 ot = 2.f) :
		Component{this}, openTime(ot), requiredMask((1u<<rc) - 1), ordered(o) {}

	void OnAwake() override;
	void OnMessage(const std::string&, const OpaqueType&) override;

private:
	void UpdateState();

};

#endif