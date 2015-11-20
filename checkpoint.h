#ifndef CHECKPOINT_H
#define CHECKPOINT_H

#include "component.h"

struct Checkpoint : Component {
	Checkpoint() : Component(this) {}

	void OnTriggerEnter(ColliderComponent*) override;
	void OnDestroy() override;

	vec3 playerSpawnPosition;
	quat playerSpawnOrientation;
};

#endif