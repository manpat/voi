#ifndef CHECKPOINT_H
#define CHECKPOINT_H

#include "component.h"

struct Checkpoint : Component {
	Checkpoint() : Component(this) {}

	void OnInit() override;
	void OnTriggerEnter(ColliderComponent*) override;
	void OnDestroy() override;
};

#endif